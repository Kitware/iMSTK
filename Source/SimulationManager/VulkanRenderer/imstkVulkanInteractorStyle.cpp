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
VulkanInteractorStyle::setWindow(GLFWwindow* window, VulkanViewer* viewer)
{
    m_window = window;
    m_viewer = viewer;

    m_stopWatch.start();
    glfwSetWindowUserPointer(window, (void*)this);

    glfwSetKeyCallback(m_window, VulkanInteractorStyle::OnCharInterface);
    glfwSetMouseButtonCallback(m_window, VulkanInteractorStyle::OnMouseButtonInterface);
    glfwSetCursorPosCallback(m_window, VulkanInteractorStyle::OnMouseMoveInterface);
    glfwSetScrollCallback(m_window, VulkanInteractorStyle::OnMouseWheelInterface);
    glfwSetWindowSizeCallback(m_window, VulkanInteractorStyle::OnWindowResizeInterface);
    glfwSetFramebufferSizeCallback(m_window, VulkanInteractorStyle::OnFramebuffersResizeInterface);
}

void
VulkanInteractorStyle::OnCharInterface(GLFWwindow* window, int keyID, int code, int type, int extra)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);
    style->OnChar(keyID, type);
}

void
VulkanInteractorStyle::OnMouseButtonInterface(GLFWwindow* window, int buttonID, int type, int extra)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);

    switch (buttonID)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        if (type == GLFW_PRESS)
        {
            style->OnLeftButtonDown();
        }
        else if (type == GLFW_RELEASE)
        {
            style->OnLeftButtonUp();
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        if (type == GLFW_PRESS)
        {
            style->OnRightButtonDown();
        }
        else if (type == GLFW_RELEASE)
        {
            style->OnRightButtonUp();
        }
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        if (type == GLFW_PRESS)
        {
            style->OnMiddleButtonDown();
        }
        else if (type == GLFW_RELEASE)
        {
            style->OnMiddleButtonUp();
        }
        break;
    }
}

void
VulkanInteractorStyle::OnTimer()
{
    // Call custom function if exists, and return
    // if it returned `override=true`
    if (m_onTimerFunction
        && m_onTimerFunction(this))
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
    if (m_onCharFunctionMap.count(key)
        && m_onCharFunctionMap.at(key)
        && m_onCharFunctionMap.at(key)(this))
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
    else if (status != SimulationStatus::INACTIVE
             && (key == 'q' || key == 'Q')) // || key == 'e' || key == 'E')) // end Simulation
    {
        m_simManager->endSimulation();
    }
    else if (key == 'f' || key == 'F') //(key == 'd' || key == 'D') // switch rendering mode
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
    else if (keyID == GLFW_KEY_ESCAPE)  // quit viewer
    {
        m_simManager->getViewer()->endRenderingLoop();
    }
    else if (key == 'p' || key == 'P')  // switch framerate display
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
    m_mousePos[0] = x;
    m_mousePos[1] = y;
    m_mousePosNormalized[0] = x;
    m_mousePosNormalized[1] = y;
    this->normalizeCoordinate(m_mousePosNormalized[0], m_mousePosNormalized[1]);

    if (m_onMouseMoveFunction && m_onMouseMoveFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnLeftButtonDown()
{
    m_state |= (unsigned int)VulkanInteractorStyle::MouseState::LEFT_MOUSE_DOWN;

    if (m_onLeftButtonDownFunction && m_onLeftButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnLeftButtonUp()
{
    m_state &= ~(unsigned int)VulkanInteractorStyle::MouseState::LEFT_MOUSE_DOWN;

    if (m_onLeftButtonUpFunction && m_onLeftButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnMiddleButtonDown()
{
    m_state |= (unsigned int)VulkanInteractorStyle::MouseState::MIDDLE_MOUSE_DOWN;

    if (m_onMiddleButtonDownFunction && m_onMiddleButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnMiddleButtonUp()
{
    m_state &= ~(unsigned int)VulkanInteractorStyle::MouseState::MIDDLE_MOUSE_DOWN;

    if (m_onMiddleButtonUpFunction && m_onMiddleButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnRightButtonDown()
{
    m_state |= (unsigned int)VulkanInteractorStyle::MouseState::RIGHT_MOUSE_DOWN;

    if (m_onRightButtonDownFunction && m_onRightButtonDownFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
}

void
VulkanInteractorStyle::OnRightButtonUp()
{
    m_state &= ~(unsigned int)VulkanInteractorStyle::MouseState::RIGHT_MOUSE_DOWN;

    if (m_onRightButtonUpFunction && m_onRightButtonUpFunction(this))
    {
        return;
    }

    if (m_simManager->getViewer()->getRenderingMode() != Renderer::Mode::DEBUG)
    {
        return;
    }
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
}

void
VulkanInteractorStyle::OnWindowResizeInterface(GLFWwindow* window, int width, int height)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);
}

void
VulkanInteractorStyle::OnFramebuffersResizeInterface(GLFWwindow* window, int width, int height)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);
    style->OnWindowResize(width, height);
}

void
VulkanInteractorStyle::normalizeCoordinate(double& x, double& y)
{
    x = (x - m_viewer->m_width / 2) / m_viewer->m_width;
    y = (y - m_viewer->m_height / 2) / m_viewer->m_height;
}

void
VulkanInteractorStyle::OnWindowResize(int width, int height)
{
    m_viewer->resizeWindow(width, height);
}

void
VulkanInteractorStyle::OnMouseMoveInterface(GLFWwindow* window, double x, double y)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);
    style->OnMouseMove(x, y);
}

void
VulkanInteractorStyle::OnMouseWheelInterface(GLFWwindow* window, double x, double y)
{
    auto style = (VulkanInteractorStyle*)glfwGetWindowUserPointer(window);
    if (y < 0)
    {
        style->OnMouseWheelBackward(y);
    }
    else
    {
        style->OnMouseWheelForward(y);
    }
}
}