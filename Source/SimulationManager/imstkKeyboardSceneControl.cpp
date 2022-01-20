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

#include "imstkKeyboardSceneControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkModuleDriver.h"
#include "imstkRenderer.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"

namespace imstk
{
KeyboardSceneControl::KeyboardSceneControl(std::shared_ptr<KeyboardDeviceClient> keyDevice) :
    KeyboardControl(keyDevice)
{
}

void
KeyboardSceneControl::printControls()
{
    LOG(INFO) << "Keyboard Scene Controls:";
    LOG(INFO) << "----------------------------------------------------------------------";
    LOG(INFO) << " | Space - pause/resume the scene";
    LOG(INFO) << " | q/Q   - stop both the scene and viewer";
    LOG(INFO) << " | d/D   - toggle debug/simulation mode";
    LOG(INFO) << " | p/P   - toggle visual/physics updates/second and task times display";
    LOG(INFO) << " | r/R   - reset the simulation";
    LOG(INFO) << " | n/N   - print camera pose of the active scene's camera";
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
KeyboardSceneControl::OnKeyPress(const char key)
{
    auto sceneManager = m_sceneManager.lock();

    if (sceneManager == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No scene manager provided";
        return;
    }
    auto driver = m_driver.lock();
    if (driver == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No driver provided";
        return;
    }

    // Pause/Resume the simulation
    if (key == ' ')
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool paused = sceneManager->getPaused();

        // Resume or pause all modules, expect viewers
        for (auto module : driver->getModules())
        {
            if (std::dynamic_pointer_cast<Viewer>(module) == nullptr)
            {
                module->setPaused(!paused);
            }
        }

        // In case the SceneManager is not apart of the driver
        paused ? sceneManager->resume() : sceneManager->pause();
    }
    // Stop the program
    else if (key == 'q' || key == 'Q' || key == 'e' || key == 'E') // end Simulation
    {
        driver->requestStatus(ModuleDriverStopped);
    }
    // Switch to debug mode
    else if (key == 'd' || key == 'D') // switch rendering mode of the modules
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool simModeOn = sceneManager->getMode() == SceneManager::Mode::Simulation ? true : false;

        for (auto module : driver->getModules())
        {
            if (auto subManager = std::dynamic_pointer_cast<SceneManager>(module))
            {
                if (simModeOn)
                {
                    subManager->setMode(SceneManager::Mode::Debug);
                }
                else
                {
                    subManager->setMode(SceneManager::Mode::Simulation);
                }
            }
            if (auto viewer = std::dynamic_pointer_cast<Viewer>(module))
            {
                if (simModeOn)
                {
                    viewer->setRenderingMode(Renderer::Mode::Debug);
                }
                else
                {
                    viewer->setRenderingMode(Renderer::Mode::Simulation);
                }
            }
        }

        simModeOn ? sceneManager->setMode(SceneManager::Mode::Debug) : sceneManager->setMode(SceneManager::Mode::Simulation);
    }
    // Toggle through info levels
    else if (key == 'p' || key == 'P')
    {
        for (auto module : driver->getModules())
        {
            std::shared_ptr<Viewer> viewer = std::dynamic_pointer_cast<Viewer>(module);
            if (viewer != nullptr)
            {
                viewer->setInfoLevel((viewer->getInfoLevel() + 1) % viewer->getInfoLevelCount());
            }
        }
    }
    // Reset simulation
    else if (key == 'r' || key == 'R')
    {
        sceneManager->getActiveScene()->reset();
    }
    else if (key == 'n' || key == 'N')
    {
        sceneManager->getActiveScene()->getActiveCamera()->print();
    }
}

void
KeyboardSceneControl::OnKeyRelease(const char imstkNotUsed(key))
{
}
} // namespace imstk