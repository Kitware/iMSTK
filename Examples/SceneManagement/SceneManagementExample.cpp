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

using namespace imstk;

///
/// \brief This example demonstrates scene management features in imstk
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();

    // Scenes
    LOG(INFO) << "-- Test add scenes";
    auto scene1 = std::make_shared<Scene>("scene1");
    simManager->addScene(scene1);

    simManager->createNewScene("scene2");
    auto scene2 = simManager->getScene("scene2");

    auto scene3 = simManager->createNewScene();
    simManager->removeScene("Scene_3");

    // switch
    LOG(INFO) << "-- Test scene switch";
    int delay = 5;
    simManager->setActiveScene(scene1);
    simManager->start();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->setActiveScene(scene2, false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->setActiveScene(scene1, true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->end();

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    simManager->setActiveScene(scene2);
    simManager->start();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->pause();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->run();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->pause();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    simManager->end();

    // Quit
    while (simManager->getStatus() != SimulationStatus::INACTIVE) {}

    return 0;
}
