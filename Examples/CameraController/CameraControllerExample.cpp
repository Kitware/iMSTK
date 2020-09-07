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

#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkMeshIO.h"
#include "imstkScene.h"
#include "imstkNew.h"
#include "imstkLogger.h"
#include "imstkSceneManager.h"
#include "imstkVTKViewer.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

#include "imstkSurfaceMesh.h"

// Devices and controllers
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkCameraController.h"
#include "imstkCollisionGraph.h"

#include "imstkScene.h"

const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates controlling the camera
/// using external device. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    #ifndef iMSTK_USE_OPENHAPTICS
    std::cout << "Audio not enabled at build time" << std::endl;
    return 1;
    #endif

    // Create Scene
    imstkNew<Scene> scene("CameraController");

#ifdef iMSTK_USE_OPENHAPTICS

    // Device Server
    auto server = std::make_shared<HapticDeviceManager>();
    auto client = server->makeDeviceClient(phantomOmni1Name);

#else
    LOG(WARNING) << "Phantom device option not enabled during build!";
#endif

    // Load Mesh
    auto mesh       = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto meshObject = std::make_shared<VisualObject>("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    // Update Camera position
    auto camera = scene->getActiveCamera();
    camera->setPosition(Vec3d(0, 0, 10));

#ifdef iMSTK_USE_OPENHAPTICS

    auto camController = std::make_shared<CameraController>(camera, client);

    //camController->setTranslationScaling(100);
    //LOG(INFO) << camController->getTranslationOffset(); // should be the same than initial cam position
    camController->setInversionFlags(CameraController::InvertFlag::rotY |
                                     CameraController::InvertFlag::rotZ);

    scene->addController(camController);
#endif

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

        // attach the camera controller to the viewer
        viewer->addControl(camController);

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

    return 0;
}
