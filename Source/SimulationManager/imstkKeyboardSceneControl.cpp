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
#include "imstkLogger.h"
#include "imstkModuleDriver.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

namespace imstk
{
KeyboardSceneControl::KeyboardSceneControl() :
    m_showFps(false), m_driver(nullptr)
{
}

KeyboardSceneControl::KeyboardSceneControl(std::shared_ptr<KeyboardDeviceClient> keyDevice) :
    KeyboardControl(keyDevice),
    m_showFps(false), m_driver(nullptr)
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
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
KeyboardSceneControl::OnKeyPress(const char key)
{
    if (m_sceneManager == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No scene manager provided";
        return;
    }
    if (m_driver == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No driver provided";
        return;
    }

    if (key == ' ')
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool paused = m_sceneManager->getPaused();

        // Resume or pause all modules
        for (auto module : m_driver->getModules())
        {
            if (paused)
            {
                module->resume();
            }
            else
            {
                module->pause();
            }
        }
    }
    else if (key == 'q' || key == 'Q' || key == 'e' || key == 'E') // end Simulation
    {
        m_driver->requestStatus(ModuleDriverStopped);
    }
    else if (key == 'd' || key == 'D') // switch rendering mode of the modules
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool simModeOn = m_sceneManager->getMode() == SceneManager::Mode::Simulation ? true : false;

        for (auto module : m_driver->getModules())
        {
            std::shared_ptr<SceneManager> sceneManager = std::dynamic_pointer_cast<SceneManager>(module);
            if (sceneManager != nullptr)
            {
                if (simModeOn)
                {
                    sceneManager->setMode(SceneManager::Mode::Debug);
                }
                else
                {
                    sceneManager->setMode(SceneManager::Mode::Simulation);
                }
            }
            std::shared_ptr<VTKViewer> viewer = std::dynamic_pointer_cast<VTKViewer>(module);
            if (viewer != nullptr)
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
    }
    else if (key == 'p' || key == 'P')  // switch framerate display
    {
        // The designated m_sceneManager framerate is displayed in all views
        for (auto module : m_driver->getModules())
        {
            std::shared_ptr<VTKViewer> viewer = std::dynamic_pointer_cast<VTKViewer>(module);
            if (viewer != nullptr)
            {
                m_showFps = !m_showFps;
                std::shared_ptr<VTKTextStatusManager> textManager = viewer->getTextStatusManager();
                textManager->setStatusVisibility(VTKTextStatusManager::StatusType::FPS, m_showFps);

                std::shared_ptr<Scene> activeScene = m_sceneManager->getActiveScene();
                activeScene->setEnableTaskTiming(m_showFps);
                std::shared_ptr<VTKRenderer> vtkRen = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer());
                vtkRen->setTimeTableVisibility(m_showFps);
            }
        }
    }
    else if (key == 'r' || key == 'R')
    {
        if (m_sceneManager != nullptr)
        {
            m_sceneManager->getActiveScene()->reset();
        }
    }
}

void
KeyboardSceneControl::OnKeyRelease(const char imstkNotUsed(key))
{
}
}