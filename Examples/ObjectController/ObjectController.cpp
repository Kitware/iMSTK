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
#include "imstkCube.h"
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkSceneObjectController.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

void objectController()
{
#ifdef iMSTK_USE_OPENHAPTICS
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("ObjectController");

    // Device Client
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Object
    auto geom = std::make_shared<Cube>();
    geom->setPosition(0, 1, 0);
    geom->setWidth(2);

    auto object = std::make_shared<CollidingObject>("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    auto trackCtrl = std::make_shared<DeviceTracker>(client);
    trackCtrl->setTranslationScaling(0.1);
    auto controller = std::make_shared<SceneObjectController>(object, trackCtrl);
    scene->addObjectController(controller);

    // Update Camera position
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 10));
    cam->setFocalPoint(geom->getPosition());

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::RUNNING);
#endif
}

int main()
{
    objectController();
    return 0;
}
