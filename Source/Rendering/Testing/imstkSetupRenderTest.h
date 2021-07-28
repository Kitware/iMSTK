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

#pragma once

#include "gtest/gtest.h"

#include <memory>

#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

namespace
{
void
run_for(SimulationManager* driver, int i)
{
    std::thread t(&SimulationManager::start, driver);

    std::this_thread::sleep_for(std::chrono::seconds(i));

    driver->requestStatus(ModuleDriverStopped);
    t.join();
}
}

class RenderTest : public testing::Test
{
public:

    void SetUp() override
    {
        scene  = std::make_shared<Scene>("Render Test Scene");
        viewer = std::make_shared<VTKViewer>("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        sceneManager = std::make_shared<SceneManager>("Scene Manager");
        sceneManager->setActiveScene(scene);

        driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        mouseControl = std::make_shared<MouseSceneControl>(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        viewer->addControl(mouseControl);

        keyControl = std::make_shared<KeyboardSceneControl>(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        viewer->addControl(keyControl);

        driver->requestStatus(ModuleDriverRunning);
    }

    std::shared_ptr<Scene>                scene;
    std::shared_ptr<VTKViewer>            viewer;
    std::shared_ptr<SceneManager>         sceneManager;
    std::shared_ptr<SimulationManager>    driver;
    std::shared_ptr<MouseSceneControl>    mouseControl;
    std::shared_ptr<KeyboardSceneControl> keyControl;
};