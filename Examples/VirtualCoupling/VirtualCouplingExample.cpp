/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkCamera.h"
#include "imstkCollisionGraph.h"
#include "imstkDirectionalLight.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates the concept of virtual coupling
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup haptics manager
    imstkNew<HapticDeviceManager>       hapticsManager;
    std::shared_ptr<HapticDeviceClient> client = hapticsManager->makeDeviceClient();

    // Scene
    imstkNew<Scene> scene("VirtualCoupling");

    std::shared_ptr<CollidingObject> obstacleObjs[] =
    {
        std::make_shared<CollidingObject>("Plane"),
        std::make_shared<CollidingObject>("Cube")
    };

    // Create a plane and cube for collision with scissors
    imstkNew<Plane> plane(Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(7.0);
    obstacleObjs[0]->setVisualGeometry(plane);
    obstacleObjs[0]->setCollidingGeometry(plane);

    imstkNew<OrientedBox> cube(Vec3d(0.0, -1.0, 0.0), Vec3d(0.5, 0.5, 0.5), Quatd(Rotd(1.0, Vec3d(0.0, 1.0, 0.0))));
    obstacleObjs[1]->setVisualGeometry(cube);
    obstacleObjs[1]->setCollidingGeometry(cube);

    for (int i = 0; i < 2; i++)
    {
        scene->addSceneObject(obstacleObjs[i]);
    }

    // A line mesh is used for collision geometry
    imstkNew<LineMesh> lineMesh;
    auto               verticesPtr = std::make_shared<VecDataArray<double, 3>>(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 2.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.0, -2.4); // tip
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    lineMesh->initialize(verticesPtr, indicesPtr);

    // The visual geometry is the scissor mesh read in from file
    auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scissors/Metzenbaum Scissors/Metz_Scissors.stl");

    imstkNew<RigidObject2>    rbdObj("rbdObj");
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_dt      = 0.001;
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdObj->setDynamicalModel(rbdModel);
    rbdObj->getRigidBody()->m_mass = 0.5;
    rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 1000000.0;
    rbdObj->setCollidingGeometry(lineMesh);
    rbdObj->setVisualGeometry(surfMesh);
    rbdObj->setPhysicsGeometry(lineMesh);
    rbdObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(lineMesh, surfMesh));
    std::shared_ptr<RenderMaterial> mat = rbdObj->getVisualModel(0)->getRenderMaterial();
    mat->setShadingModel(RenderMaterial::ShadingModel::PBR);
    mat->setRoughness(0.5);
    mat->setMetalness(1.0);
    scene->addSceneObject(rbdObj);

    // Create a virtual coupling controller
    imstkNew<RigidObjectController> controller(rbdObj, client);
    controller->setLinearKs(10000.0);
    controller->setLinearKd(100.0);
    controller->setAngularKs(1000000000.0);
    controller->setAngularKd(300000000.0);
    controller->setTranslationScaling(0.02);
    controller->setForceScaling(0.001);
    //controller->setInversionFlags(RigidObjectController::InvertFlag::rotY);
    scene->addController(controller);

    // Add interaction between the rigid object sphere and static plane
    scene->getCollisionGraph()->addInteraction(
        std::make_shared<RigidObjectCollision>(rbdObj, obstacleObjs[0], "PointSetToPlaneCD"));
    scene->getCollisionGraph()->addInteraction(
        std::make_shared<RigidObjectCollision>(rbdObj, obstacleObjs[1], "PointSetToOrientedBoxCD"));

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 5.0, 10.0));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, -1.0, 0.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(hapticsManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
        {
            // Run the rbd model in real time
            rbdModel->getConfig()->m_dt = driver->getDt();
            });

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}
