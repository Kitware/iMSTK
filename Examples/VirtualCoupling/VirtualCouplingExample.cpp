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
#include "imstkCDObjectFactory.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkCollisionGraph.h"
#include "imstkCollisionPair.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVirtualCouplingCH.h"
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

    // Scene
    imstkNew<Scene> scene("VirtualCoupling");

    // Create a plane in the scene
    auto            planeGeom = std::make_shared<Plane>();
    imstkNew<Plane> plane(Vec3d(0.0, -50.0, 0.0));
    planeGeom->setWidth(400.0);
    imstkNew<CollidingObject> planeObj("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Create the virtual coupling object controller

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    const std::string                   deviceName = "";
    std::shared_ptr<HapticDeviceClient> client     = server->makeDeviceClient(deviceName);

    // Create a virtual coupling object
    imstkNew<Sphere>          visualGeom(Vec3d(0.0, 0.0, 0.0), 20.0);
    imstkNew<Sphere>          collidingGeom(Vec3d(0.0, 0.0, 0.0), 20.0);
    imstkNew<CollidingObject> obj("VirtualCouplingObject");
    obj->setCollidingGeometry(collidingGeom);

    imstkNew<RenderMaterial> material;
    imstkNew<VisualModel>    visualModel(visualGeom.get());
    visualModel->setRenderMaterial(material);
    obj->addVisualModel(visualModel);

    // Add virtual coupling object (with visual, colliding, and physics geometry) in the scene.
    scene->addSceneObject(obj);

    // Create and add virtual coupling object controller in the scene
    imstkNew<SceneObjectController> controller(obj, client);
    scene->addController(controller);

    {
        // Setup CD, and collision data
        imstkNew<CollisionData> colData;

        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(CollisionDetection::Type::UnidirectionalPlaneToSphere,
            planeObj->getCollidingGeometry(), obj->getCollidingGeometry(), colData);

        // Setup the handler
        imstkNew<VirtualCouplingCH> colHandler(CollisionHandling::Side::B, colData, obj);
        colHandler->setStiffness(5e-01);
        colHandler->setDamping(0.005);

        imstkNew<CollisionPair> pair(planeObj, obj, colDetect, nullptr, colHandler);
        scene->getCollisionGraph()->addInteraction(pair);
    }

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(200, 200, 200));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    //Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(server);
        driver->addModule(viewer);
        driver->addModule(sceneManager);

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
