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

#include "imstkSimulationManager.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkMeshIO.h"

// Devices and controllers
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkCameraController.h"

const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates controlling the camera
/// using external device. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    #ifndef iMSTK_USE_OPENHAPTICS
    std::cout << "Audio not enabled at build time" << std::endl;
    return 1;
    #endif

    // Create simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("CameraController");

#ifdef iMSTK_USE_OPENHAPTICS

    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    simManager->addModule(server);
#else
    LOG(WARNING) << "Phantom device option not enabled during build!";
#endif

    // Load Mesh
    auto mesh       = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto meshObject = std::make_shared<VisualObject>("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 10));

#ifdef iMSTK_USE_OPENHAPTICS

    auto camControllerInput = std::make_shared<CameraController>(*cam, client);

    // Set camera controller
    auto camController = cam->setController(camControllerInput);
    //camController->setTranslationScaling(100);
    //LOG(INFO) << camController->getTranslationOffset(); // should be the same than initial cam position
    camController->setInversionFlags(CameraController::InvertFlag::rotY |
        CameraController::InvertFlag::rotZ);
#endif

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::paused);

    return 0;
}
