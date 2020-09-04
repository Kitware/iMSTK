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
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

namespace imstk
{
KeyboardSceneControl::KeyboardSceneControl() :
    m_showFps(false), m_sceneManager(nullptr), m_viewer(nullptr)
{
}

KeyboardSceneControl::KeyboardSceneControl(std::shared_ptr<KeyboardDeviceClient> keyDevice) :
    KeyboardControl(keyDevice),
    m_showFps(false), m_sceneManager(nullptr), m_viewer(nullptr)
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
    if (key == ' ')
    {
        if (m_sceneManager != nullptr)
        {
            ThreadStatus status = m_sceneManager->getStatus();
            // Pause the active scene
            if (status == ThreadStatus::Running)
            {
                m_sceneManager->pause(true);
            }
            // Resume the active scene
            else if (status == ThreadStatus::Paused)
            {
                m_sceneManager->resume();
            }
            // Launch the active scene if it has yet to start
            if (status == ThreadStatus::Inactive)
            {
                //m_textStatusManager->setStatusVisibility(VTKTextStatusManager::FPS, m_displayFps);
                m_sceneManager->resume();
            }
        }
    }
    else if (key == 'q' || key == 'Q' || key == 'e' || key == 'E') // end Simulation
    {
        if (m_sceneManager != nullptr && m_viewer != nullptr)
        {
            if (m_viewer->getStatus() != ThreadStatus::Inactive)
            {
                m_viewer->stop(false);
            }
        }
        else if (m_sceneManager != nullptr && m_viewer == nullptr)
        {
            if (m_sceneManager->getStatus() != ThreadStatus::Inactive)
            {
                m_sceneManager->stop(false);
            }
        }
        else if (m_sceneManager == nullptr && m_viewer != nullptr)
        {
            if (m_viewer->getStatus() != ThreadStatus::Inactive)
            {
                m_viewer->stop(false);
            }
        }
    }
    else if (key == 'd' || key == 'D') // switch rendering mode of the modules
    {
        if (m_sceneManager != nullptr && m_viewer != nullptr)
        {
            if (m_viewer->getRenderingMode() != Renderer::Mode::Simulation)
            {
                m_sceneManager->setMode(SceneManager::Mode::Simulation);
                m_viewer->setRenderingMode(Renderer::Mode::Simulation);
            }
            else
            {
                m_sceneManager->setMode(SceneManager::Mode::Debug);
                m_viewer->setRenderingMode(Renderer::Mode::Debug);
            }
        }
    }
    else if (key == 'p' || key == 'P')  // switch framerate display
    {
        if (m_sceneManager != nullptr && m_viewer != nullptr)
        {
            // If we're dealing with a VTK viewer, flip the text visibility for FPS
            std::shared_ptr<VTKViewer> vtkViewer = std::dynamic_pointer_cast<VTKViewer>(m_viewer);
            if (vtkViewer != nullptr)
            {
                m_showFps = !m_showFps;
                std::shared_ptr<VTKTextStatusManager> textManager = vtkViewer->getTextStatusManager();
                textManager->setStatusVisibility(VTKTextStatusManager::StatusType::FPS, m_showFps);

                std::shared_ptr<Scene> activeScene = m_sceneManager->getActiveScene();
                activeScene->setEnableTaskTiming(m_showFps);
                const auto vtkRen = std::dynamic_pointer_cast<VTKRenderer>(m_viewer->getActiveRenderer());
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