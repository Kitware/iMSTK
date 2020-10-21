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
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"

using namespace imstk;

///
/// \brief This example demonstrates scene management features in imstk
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<SceneManager> sceneManager;

    // Scenes
    LOG(INFO) << "-- Test add scenes";
    imstkNew<Scene> scene1("scene1");
    scene1->initialize();
    sceneManager->addScene(scene1);
    imstkNew<Scene> scene2("scene2");
    scene2->initialize();
    sceneManager->addScene(scene2);

    imstkNew<Scene> scene3("scene3");
    scene3->initialize();
    sceneManager->addScene(scene3);
    sceneManager->removeScene("Scene3");

    // switch
    LOG(INFO) << "-- Test scene switch";
    int delay = 1;
    sceneManager->setActiveScene(scene1);
    sceneManager->start(false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->setActiveScene(scene2);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->setActiveScene(scene1);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->stop(true);

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    sceneManager->setActiveScene(scene2);
    sceneManager->start(false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->pause(true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->resume(true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->pause(true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sceneManager->stop(true);

    return 0;
}
