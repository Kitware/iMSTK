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

#include "imstkVulkanInteractorStyleTrackballCamera.h"

#include "imstkVulkanViewer.h"
#include "imstkSimulationManager.h"

namespace imstk
{
VulkanInteractorStyleTrackballCamera::VulkanInteractorStyleTrackballCamera()
{
}

void
VulkanInteractorStyleTrackballCamera::setWindow(GLFWwindow * window, VulkanViewer * viewer)
{
    m_window = window;
    m_viewer = viewer;

    glfwSetWindowUserPointer(window, (void *)this);

    glfwSetKeyCallback(m_window, VulkanInteractorStyleTrackballCamera::OnCharInterface);
    glfwSetMouseButtonCallback(m_window, VulkanInteractorStyleTrackballCamera::OnMouseButtonInterface);
    glfwSetCursorPosCallback(m_window, VulkanInteractorStyleTrackballCamera::OnMouseMoveInterface);
    glfwSetScrollCallback(m_window, VulkanInteractorStyleTrackballCamera::OnMouseWheelInterface);
    glfwSetWindowSizeCallback(m_window, VulkanInteractorStyleTrackballCamera::OnWindowResizeInterface);
    glfwSetFramebufferSizeCallback(m_window, VulkanInteractorStyleTrackballCamera::OnFramebuffersResizeInterface);
}

void
VulkanInteractorStyleTrackballCamera::OnTimer()
{
}

void
VulkanInteractorStyleTrackballCamera::OnCharInterface(GLFWwindow * window, int keyID, int code, int type, int extra)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);
    style->OnChar(keyID, type);
}

void
VulkanInteractorStyleTrackballCamera::OnMouseButtonInterface(GLFWwindow * window, int buttonID, int type, int extra)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);

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
VulkanInteractorStyleTrackballCamera::OnMouseMoveInterface(GLFWwindow * window, double x, double y)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);
    style->OnMouseMove(x, y);
}

void
VulkanInteractorStyleTrackballCamera::OnMouseWheelInterface(GLFWwindow * window, double x, double y)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);
    if (y < 0)
    {
        style->OnMouseWheelBackward(y);
    }
    else
    {
        style->OnMouseWheelForward(y);
    }
}

void
VulkanInteractorStyleTrackballCamera::OnChar(int keyID, int type)
{
}

void
VulkanInteractorStyleTrackballCamera::OnMouseMove(double x, double y)
{
    m_mouseX = (x - m_viewer->m_width / 2) / m_viewer->m_width;
    m_mouseY = (y - m_viewer->m_height / 2) / m_viewer->m_height;
    auto camera = m_simManager->getActiveScene()->getCamera();
    auto offset = camera->getPosition() - camera->getFocalPoint();
    auto dx = m_mouseX - m_lastMouseX;
    auto dy = m_mouseY - m_lastMouseY;

    if (m_state & VulkanInteractorStyleTrackballCamera::LEFT_MOUSE_DOWN)
    {
        double strength = distance(m_mouseX, m_mouseY);

        auto convertedOffset = glm::vec3(offset.x(), offset.y(), offset.z());
        glm::vec3 rotationAxis(0,1,0);
        glm::mat4 rotation;
        rotation = glm::rotate(rotation, -(float)dx, rotationAxis);
        auto new_position = rotation * glm::vec4(offset.x(), offset.y(), offset.z(), 1);
        imstk::Vec3d position(new_position[0], new_position[1], new_position[2]);
        camera->setPosition(camera->getFocalPoint() + position);
    }
    else if (m_state & VulkanInteractorStyleTrackballCamera::MIDDLE_MOUSE_DOWN)
    {
        auto camera = m_simManager->getActiveScene()->getCamera();
        auto eye = glm::tvec3<float>(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z());
        auto center = glm::tvec3<float>(camera->getFocalPoint().x(), camera->getFocalPoint().y(), camera->getFocalPoint().z());
        auto up = glm::tvec3<float>(camera->getViewUp().x(), camera->getViewUp().y(), camera->getViewUp().z());

        glm::mat4 cameraTranslationMatrix(1);
        cameraTranslationMatrix = glm::translate(cameraTranslationMatrix, glm::tvec3<float>(-dx * 10, dy * 10, 0));
        auto cameraMatrix = glm::inverse(glm::lookAt(eye, center, up));

        auto new_position = cameraMatrix * cameraTranslationMatrix;
        auto new_focal_point_offset = glm::mat3(cameraMatrix) * glm::vec3(cameraTranslationMatrix[3]);
        imstk::Vec3d position(new_position[3][0], new_position[3][1], new_position[3][2]);
        camera->setPosition(position);
        auto new_focal_point = center + new_focal_point_offset;
        camera->setFocalPoint(Vec3d(new_focal_point[0], new_focal_point[1], new_focal_point[2]));
    }

    m_lastMouseX = m_mouseX;
    m_lastMouseY = m_mouseY;
}

void
VulkanInteractorStyleTrackballCamera::OnLeftButtonDown()
{
    m_state |= VulkanInteractorStyleTrackballCamera::LEFT_MOUSE_DOWN;
}

void
VulkanInteractorStyleTrackballCamera::OnLeftButtonUp()
{
    m_state &= ~VulkanInteractorStyleTrackballCamera::LEFT_MOUSE_DOWN;
}

void
VulkanInteractorStyleTrackballCamera::OnMiddleButtonDown()
{
    m_state |= VulkanInteractorStyleTrackballCamera::MIDDLE_MOUSE_DOWN;
}

void VulkanInteractorStyleTrackballCamera::OnMiddleButtonUp()
{
    m_state &= ~VulkanInteractorStyleTrackballCamera::MIDDLE_MOUSE_DOWN;
}

void
VulkanInteractorStyleTrackballCamera::OnRightButtonDown()
{
    m_state |= VulkanInteractorStyleTrackballCamera::RIGHT_MOUSE_DOWN;
}

void
VulkanInteractorStyleTrackballCamera::OnRightButtonUp()
{
    m_state &= ~VulkanInteractorStyleTrackballCamera::RIGHT_MOUSE_DOWN;
}

void
VulkanInteractorStyleTrackballCamera::OnMouseWheelForward(double y)
{
    auto camera = m_simManager->getActiveScene()->getCamera();

    auto offset = camera->getPosition() - camera->getFocalPoint();

    auto offsetx = -0.01 * offset.x() * y + camera->getPosition().x();
    auto offsety = -0.01 * offset.y() * y + camera->getPosition().y();
    auto offsetz = -0.01 * offset.z() * y + camera->getPosition().z();

    camera->setPosition(Vec3d(offsetx, offsety, offsetz));
}

void
VulkanInteractorStyleTrackballCamera::OnMouseWheelBackward(double y)
{
    auto camera = m_simManager->getActiveScene()->getCamera();

    auto offset = camera->getPosition() - camera->getFocalPoint();

    auto offsetx = -0.01 * offset.x() * y + camera->getPosition().x();
    auto offsety = -0.01 * offset.y() * y + camera->getPosition().y();
    auto offsetz = -0.01 * offset.z() * y + camera->getPosition().z();

    camera->setPosition(Vec3d(offsetx, offsety, offsetz));
}

inline double
VulkanInteractorStyleTrackballCamera::distance(double x, double y)
{
    return sqrt(x * x + y * y);
}

void
VulkanInteractorStyleTrackballCamera::OnWindowResizeInterface(GLFWwindow * window, int width, int height)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);
}

void
VulkanInteractorStyleTrackballCamera::OnFramebuffersResizeInterface(GLFWwindow * window, int width, int height)
{
    VulkanInteractorStyleTrackballCamera * style = (VulkanInteractorStyleTrackballCamera *)glfwGetWindowUserPointer(window);
    style->OnWindowResize(width, height);
}

void
VulkanInteractorStyleTrackballCamera::OnWindowResize(int width, int height)
{
    m_viewer->resizeWindow(width, height);
}
}