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

#include "imstkVulkanInteractorStyle.h"

#include "imstkVulkanViewer.h"
#include "imstkSimulationManager.h"

namespace imstk
{
VulkanInteractorStyle::VulkanInteractorStyle()
{
}

void
VulkanInteractorStyle::OnTimer()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onTimerFunction &&
       m_onTimerFunction(this))
    {
        return;
    }
}

void
VulkanInteractorStyle::OnChar(int keyID, int type)
{
    char key = std::tolower((char)keyID);

    if (type != GLFW_PRESS)
    {
        return;
    }

    // Call custom function if exists, and return
    // if it returned `override=true`
    if(m_onCharFunctionMap.count(key) &&
       m_onCharFunctionMap.at(key) &&
       m_onCharFunctionMap.at(key)(this))
    {
        return;
    }

    SimulationStatus status = m_simManager->getStatus();

    if (key == ' ')
    {
        // pause simulation
        if (status == SimulationStatus::RUNNING)
        {
            m_simManager->pauseSimulation();
        }
        // play simulation
        else if (status == SimulationStatus::PAUSED)
        {
            m_simManager->runSimulation();
        }
        // Launch simulation if inactive
        if (status == SimulationStatus::INACTIVE)
        {
            m_simManager->startSimulation(SimulationStatus::RUNNING);
        }
    }
    else if (status != SimulationStatus::INACTIVE &&
             (key == 'q' || key == 'Q' || key == 'e' || key == 'E')) // end Simulation
    {
        m_simManager->endSimulation();
    }
    else if (key == 'd' || key == 'D') // switch rendering mode
    {
        if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::SIMULATION)
        {
            m_simManager->getViewer()->setRenderingMode(Renderer::Mode::SIMULATION);
        }
        else
        {
            m_simManager->getViewer()->setRenderingMode(Renderer::Mode::DEBUG);
        }
    }
    else if (keyID == GLFW_KEY_ESCAPE) // quit viewer
    {
        m_simManager->getViewer()->endRenderingLoop();
    }
    else if (key == 'p' || key == 'P') // switch framerate display
    {
    }
    else if (key == 'r' || key == 'R')
    {
        m_simManager->resetSimulation();
    }
}

void
VulkanInteractorStyle::OnMouseMove(double x, double y)
{
    if (m_onMouseMoveFunction && m_onMouseMoveFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnMouseMove(x, y);
}

void
VulkanInteractorStyle::OnLeftButtonDown()
{
    if (m_onLeftButtonDownFunction && m_onLeftButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnLeftButtonDown();
}

void
VulkanInteractorStyle::OnLeftButtonUp()
{
    if (m_onLeftButtonUpFunction && m_onLeftButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnLeftButtonUp();
}

void
VulkanInteractorStyle::OnMiddleButtonDown()
{
    if (m_onMiddleButtonDownFunction && m_onMiddleButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnMiddleButtonDown();
}

void VulkanInteractorStyle::OnMiddleButtonUp()
{
    if (m_onMiddleButtonUpFunction && m_onMiddleButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnMiddleButtonUp();
}

void
VulkanInteractorStyle::OnRightButtonDown()
{
    if (m_onRightButtonDownFunction && m_onRightButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnRightButtonDown();
}

void
VulkanInteractorStyle::OnRightButtonUp()
{
    if (m_onRightButtonUpFunction && m_onRightButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnRightButtonUp();
}

void
VulkanInteractorStyle::OnMouseWheelForward(double y)
{
    if (m_onMouseWheelForwardFunction && m_onMouseWheelForwardFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnMouseWheelForward(y);
}

void
VulkanInteractorStyle::OnMouseWheelBackward(double y)
{
    if (m_onMouseWheelBackwardFunction && m_onMouseWheelBackwardFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }

    VulkanBaseInteractorStyle::OnMouseWheelBackward(y);
}
}