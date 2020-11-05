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
#include "imstkCameraController.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkVTKViewer.h"

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

    // Create Scene
    imstkNew<Scene> scene("CameraController");

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    const std::string deviceName = "";
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient(deviceName);

    // Load Mesh
    auto                   mesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    imstkNew<VisualObject> meshObject("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    imstkNew<VisualObject> planeObject("Plane");
    imstkNew<Plane>        plane(Vec3d(0.0, -2.0, 0.0));
    plane->setWidth(1000.0);
    planeObject->setVisualGeometry(plane);
    scene->addSceneObject(planeObject);

    // Update Camera position
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 10.0));

    imstkNew<CameraController> camController(scene->getActiveCamera(), client);
    //camController->setTranslationScaling(100);
    //LOG(INFO) << camController->getTranslationOffset(); // should be the same than initial cam position
    scene->addController(camController);

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
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

        // Start viewer and its children
        viewer->start();
    }

    return 0;
}
