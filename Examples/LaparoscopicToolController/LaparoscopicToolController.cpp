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
#include "imstkAPIUtilities.h"
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkLaparoscopicToolController.h"

const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

void lapToolController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("LapToolController");

    // Device clients
    auto client0 = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client0);
    sdk->addModule(server);

    // Plane
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 100., Vec3d(0., -20., 0.));

    // laparoscopic tool
    auto pivot = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/pivot.obj", "pivot");
    auto upperJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/upper.obj", "upperJaw");
    auto lowerJaw = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/laptool/lower.obj", "lowerJaw");

    auto trackingCtrl = std::make_shared<DeviceTracker>(client0);
    trackingCtrl->setTranslationScaling(0.5);
    auto lapToolController = std::make_shared<LaparoscopicToolController>(pivot, upperJaw, lowerJaw, trackingCtrl);
    lapToolController->setJawRotationAxis(Vec3d(1.0, 0, 0));
    lapToolController->setJawAngleChange(0.1);
    scene->addObjectController(lapToolController);

    // Set Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 30, 60));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

#endif
}

int main()
{
    lapToolController();

    return 0;
}
