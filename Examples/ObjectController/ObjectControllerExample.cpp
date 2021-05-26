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
#include "imstkCapsule.h"
#include "imstkCollidingObject.h"
#include "imstkCylinder.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the object
/// using external device. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("ObjectController");

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient();

    std::shared_ptr<AnalyticalGeometry> geometries[] = {
        std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(1.0, 5.0, 1.0)),
        std::make_shared<Plane>(Vec3d::Zero(), Vec3d(0.0, 1.0, 0.0)),
        std::make_shared<Capsule>(Vec3d::Zero(), 0.5, 1.0),
        std::make_shared<Cylinder>(Vec3d::Zero(), 5.0, 1.0),
        std::make_shared<Sphere>(Vec3d::Zero(), 2.0)
    };

    imstkNew<SceneObject> object("VirtualObject");
    object->setVisualGeometry(geometries[0]);
    scene->addSceneObject(object);

    imstkNew<SceneObjectController> controller(object, client);
    controller->setTranslationScaling(0.1);
    scene->addController(controller);

    // Update Camera position
    std::shared_ptr<Camera> cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0.0, 0.0, 10.0));
    cam->setFocalPoint(geometries[0]->getPosition());

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
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
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(server);
        driver->setDesiredDt(0.01);

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
