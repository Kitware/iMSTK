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

#include "imstkNew.h"
#include "imstkLogger.h"
#include "imstkSceneManager.h"
#include "imstkVTKViewer.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

#include "imstkCube.h"
#include "imstkCamera.h"
#include "imstkLight.h"
#include "imstkCollidingObject.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkSceneObjectController.h"
#include "imstkCollisionGraph.h"
#include "imstkScene.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates controlling the object
/// using external device. NOTE: Requires GeoMagic Touch device
///
int
main()
{
#ifdef iMSTK_USE_OPENHAPTICS

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("ObjectController");

    // Device Server
    auto server = std::make_shared<HapticDeviceManager>();
    auto client = server->makeDeviceClient(phantomOmni1Name);

    // Object
    auto geom = std::make_shared<Cube>();
    geom->setPosition(0, 1, 0);
    geom->setWidth(2);

    auto object = std::make_shared<CollidingObject>("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    auto controller = std::make_shared<SceneObjectController>(object, client);
    controller->setTranslationScaling(0.1);
    scene->addController(controller);

    // Update Camera position
    auto cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0, 0, 10));
    cam->setFocalPoint(geom->getPosition());

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        viewer->addChildThread(server);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        // Start viewer running, scene as paused
        viewer->start();
    }
#endif
    return 0;
}
