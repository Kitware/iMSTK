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

#include "imstkVulkanInteractorStyleVR.h"

#ifdef iMSTK_ENABLE_VR

#include "imstkSimulationManager.h"

namespace imstk
{

VulkanInteractorStyleVR::VulkanInteractorStyleVR()
{
}

void
VulkanInteractorStyleVR::OnTimer()
{
    vr::VRCompositor()->WaitGetPoses(m_devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    auto scene = m_simManager->getActiveScene();
    auto indexHMD = vr::k_unTrackedDeviceIndex_Hmd;

    // Iterate over maximum number of devices
    for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
    {
        m_deviceClasses[i] = m_renderer->m_VRSystem->GetTrackedDeviceClass(i);
    }

    auto HMDMatrixNative = m_devicePoses[indexHMD].mDeviceToAbsoluteTracking.m;

    glm::mat4 HMDMatrix(
        HMDMatrixNative[0][0], HMDMatrixNative[1][0], HMDMatrixNative[2][0], 0.0f,
        HMDMatrixNative[0][1], HMDMatrixNative[1][1], HMDMatrixNative[2][1], 0.0f,
        HMDMatrixNative[0][2], HMDMatrixNative[1][2], HMDMatrixNative[2][2], 0.0f,
        HMDMatrixNative[0][3], HMDMatrixNative[1][3], HMDMatrixNative[2][3], 1.0f);
    HMDMatrix = glm::inverse(HMDMatrix);

    auto projectionMatrixLeftNative = m_renderer->m_VRSystem->GetProjectionMatrix(vr::Eye_Left, m_renderer->m_nearPlane, m_renderer->m_farPlane).m;
    auto projectionMatrixRightNative = m_renderer->m_VRSystem->GetProjectionMatrix(vr::Eye_Right, m_renderer->m_nearPlane, m_renderer->m_farPlane).m;

    glm::mat4 projectionMatrixLeft(
        projectionMatrixLeftNative[0][0], projectionMatrixLeftNative[1][0], projectionMatrixLeftNative[2][0], projectionMatrixLeftNative[3][0],
        projectionMatrixLeftNative[0][1], projectionMatrixLeftNative[1][1], projectionMatrixLeftNative[2][1], projectionMatrixLeftNative[3][1],
        projectionMatrixLeftNative[0][2], projectionMatrixLeftNative[1][2], projectionMatrixLeftNative[2][2], projectionMatrixLeftNative[3][2],
        projectionMatrixLeftNative[0][3], projectionMatrixLeftNative[1][3], projectionMatrixLeftNative[2][3], projectionMatrixLeftNative[3][3]);

    glm::mat4 projectionMatrixRight(
        projectionMatrixRightNative[0][0], projectionMatrixRightNative[1][0], projectionMatrixRightNative[2][0], projectionMatrixRightNative[3][0],
        projectionMatrixRightNative[0][1], projectionMatrixRightNative[1][1], projectionMatrixRightNative[2][1], projectionMatrixRightNative[3][1],
        projectionMatrixRightNative[0][2], projectionMatrixRightNative[1][2], projectionMatrixRightNative[2][2], projectionMatrixRightNative[3][2],
        projectionMatrixRightNative[0][3], projectionMatrixRightNative[1][3], projectionMatrixRightNative[2][3], projectionMatrixRightNative[3][3]);

    auto eyeMatrixLeftNative = m_renderer->m_VRSystem->GetEyeToHeadTransform(vr::Eye_Left).m;
    auto eyeMatrixRightNative = m_renderer->m_VRSystem->GetEyeToHeadTransform(vr::Eye_Right).m;

    glm::mat4 eyeMatrixLeft(
        eyeMatrixLeftNative[0][0], eyeMatrixLeftNative[1][0], eyeMatrixLeftNative[2][0], 0.0f,
        eyeMatrixLeftNative[0][1], eyeMatrixLeftNative[1][1], eyeMatrixLeftNative[2][1], 0.0f,
        eyeMatrixLeftNative[0][2], eyeMatrixLeftNative[1][2], eyeMatrixLeftNative[2][2], 0.0f,
        eyeMatrixLeftNative[0][3], eyeMatrixLeftNative[1][3], eyeMatrixLeftNative[2][3], 1.0f);

    glm::mat4 eyeMatrixRight(
        eyeMatrixRightNative[0][0], eyeMatrixRightNative[1][0], eyeMatrixRightNative[2][0], 0.0f,
        eyeMatrixRightNative[0][1], eyeMatrixRightNative[1][1], eyeMatrixRightNative[2][1], 0.0f,
        eyeMatrixRightNative[0][2], eyeMatrixRightNative[1][2], eyeMatrixRightNative[2][2], 0.0f,
        eyeMatrixRightNative[0][3], eyeMatrixRightNative[1][3], eyeMatrixRightNative[2][3], 1.0f);

    m_renderer->m_viewMatrices[0] = eyeMatrixLeft * HMDMatrix;
    m_renderer->m_viewMatrices[1] = eyeMatrixRight * HMDMatrix;

    m_renderer->m_projectionMatrices[0] = projectionMatrixLeft;
    m_renderer->m_projectionMatrices[1] = projectionMatrixRight;
}

}

#endif