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

#include "imstkVulkanInteractorStyleFreeCamera.h"

#include "imstkSimulationManager.h"
#include "imstkVulkanViewer.h"

namespace imstk
{
VulkanInteractorStyleFreeCamera::VulkanInteractorStyleFreeCamera()
{
}

void
VulkanInteractorStyleFreeCamera::OnTimer()
{
    float speed = 0.0005f;

    auto camera = m_simManager->getActiveScene()->getCamera();

    // Initialize certain variables
    if (!m_started)
    {
        m_lastFrameMode = m_viewer->getRenderingMode();
        m_lastTime = m_stopWatch.getTimeElapsed();
        m_mousePosLastNormalized[0] = m_mousePosNormalized[0];
        m_mousePosLastNormalized[1] = m_mousePosNormalized[1];
        m_started = true;
    }

    auto currentMode = m_viewer->getRenderingMode();

    // Detect change to/from simulation and debug mode
    if (m_lastFrameMode == Renderer::Mode::SIMULATION)
    {
        if (currentMode == Renderer::Mode::DEBUG)
        {
            m_simCameraPosition = camera->getPosition();
            m_simCameraFocalPoint = camera->getFocalPoint();
            camera->setPosition(m_simCameraPosition[0], m_simCameraFocalPoint[1], m_simCameraPosition[2]);
            m_cameraAngle = 0;
        }
    }
    else if (m_lastFrameMode = Renderer::Mode::DEBUG)
    {
        if (currentMode == Renderer::Mode::SIMULATION)
        {
            camera->setPosition(m_simCameraPosition);
            camera->setFocalPoint(m_simCameraFocalPoint);
        }
    }

    m_lastFrameMode = m_viewer->getRenderingMode();

    // Extract variables/calculate derivatives
    auto pos = camera->getPosition();
    auto fp = camera->getFocalPoint();
    auto dx = m_mousePosNormalized[0] - m_mousePosLastNormalized[0];
    auto dy = m_mousePosNormalized[1] - m_mousePosLastNormalized[1];
    auto direction = Vec3d(pos - fp);
    direction.normalize();
    auto yaw = atan2(pos[0] - fp[0], pos[2] - fp[2]);
    auto xDirection = Vec3d(cos(yaw), 0, -sin(yaw));

    auto currentTime = m_stopWatch.getTimeElapsed();
    float dt = currentTime - m_lastTime; // For variable time-step

    // Update for next frame
    m_lastTime = currentTime;
    m_mousePosLastNormalized[0] = m_mousePosNormalized[0];
    m_mousePosLastNormalized[1] = m_mousePosNormalized[1];

    // The following is only for movement mode
    if (!(m_state & VulkanInteractorStyleFreeCamera::RIGHT_MOUSE_DOWN))
    {
        return;
    }

    auto angleTempOffset = Vec3d(fp - pos);
    auto angleOffset = glm::vec4(angleTempOffset[0], angleTempOffset[1], angleTempOffset[2], 1);

    // Prevents flipping camera at up and down view
    float dCameraAngle = -4 * dy;
    if (m_cameraAngle + dCameraAngle <= -PI_2 + 0.01)
    {
        dCameraAngle = -PI_2 - m_cameraAngle + 0.01;
    }
    else if (m_cameraAngle + dCameraAngle >= PI_2 - 0.01)
    {
        dCameraAngle = PI_2 - m_cameraAngle - 0.01;
    }
    m_cameraAngle += dCameraAngle;

    // Rotational offset
    auto xRotation = glm::rotate<float>(-4 * dx, glm::tvec3<float>(0.0f, 1.0f, 0.0f));
    auto yRotation = glm::rotate<float>(dCameraAngle, glm::tvec3<float>(xDirection[0], 0, xDirection[2]));
    angleOffset = xRotation * yRotation * angleOffset;
    angleTempOffset[0] = angleOffset.x;
    angleTempOffset[1] = angleOffset.y;
    angleTempOffset[2] = angleOffset.z;

    auto offset = Vec3d(0, 0, 0); // Position offset

    // Controls
    if (glfwGetKey(m_viewer->m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed *= 5;
    }
    if (glfwGetKey(m_viewer->m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        offset += xDirection * -speed * dt;
    }
    if (glfwGetKey(m_viewer->m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        offset += xDirection * speed * dt;
    }
    if (glfwGetKey(m_viewer->m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        offset += direction * -speed * dt;
    }
    if (glfwGetKey(m_viewer->m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        offset += direction * speed * dt;
    }

    camera->setPosition(pos + offset);
    fp = angleTempOffset + camera->getPosition();
    camera->setFocalPoint(fp);
}
}