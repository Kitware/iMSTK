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
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
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
    const std::string                   deviceName = "";
    std::shared_ptr<HapticDeviceClient> client     = hapticsManager->makeDeviceClient(deviceName);

    // Scene
    imstkNew<Scene> scene("VirtualCoupling");

    // Create a plane in the scene to touch
    imstkNew<Plane> plane(Vec3d(0.0, -50.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(400.0);
    imstkNew<CollidingObject> planeObj("Plane");
    planeObj->setVisualGeometry(plane);
    planeObj->setCollidingGeometry(plane);
    scene->addSceneObject(planeObj);

    // Create a virtual coupling object
    imstkNew<Sphere>          sphere(Vec3d(0.0, 0.0, 0.0), 20.0);
    imstkNew<RigidObject2>    sphereObj("VirtualCouplingObject");
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_dt      = 0.001;
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    sphereObj->setDynamicalModel(rbdModel);
    sphereObj->getRigidBody()->m_mass = 1.0;
    sphereObj->setCollidingGeometry(sphere);
    sphereObj->setVisualGeometry(sphere);
    sphereObj->setPhysicsGeometry(sphere);
    scene->addSceneObject(sphereObj);

    // Create a virtual coupling controller
    imstkNew<RigidObjectController> controller(sphereObj, client);
    controller->setLinearKs(10000.0);
    controller->setLinearKd(100.0);
    controller->setAngularKs(0.0);
    controller->setAngularKd(0.0);
    controller->setForceScaling(0.00001);
    scene->addController(controller);

    // Add interaction between the rigid object sphere and static plane
    scene->getCollisionGraph()->addInteraction(
        std::make_shared<RigidObjectCollision>(sphereObj, planeObj, "UnidirectionalPlaneToSphereCD"));

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 269.0, 295.0));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, -20.0, 5.7));
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
