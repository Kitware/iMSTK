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
#include "imstkDummyClient.h"
#include "imstkSceneObjectController.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the object
/// using a simulated tracker: dummy client
///
int
main()
{
    // simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("ObjectControllerDummyClient");

    // Device Client
    auto client = std::make_shared<DummyClient>("DummyClient");

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

    // Supply translation to dummy client frame
    auto translateFunc =
        [&client](Module* module)
        {
            Vec3d p = client->getPosition() + Vec3d(1.0e-4, 0, 0);
            if (p.x() > 50.)
            {
                p = Vec3d(0, 0, 0);
            }
            client->setPosition(p);
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(translateFunc);

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
    simManager->setActiveScene(scene);
    simManager->startSimulation(SimulationStatus::RUNNING);

    return 0;
}
