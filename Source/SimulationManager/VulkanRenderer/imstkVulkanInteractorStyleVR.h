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

#ifdef iMSTK_ENABLE_VR
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <thread>

#include "glm/gtx/matrix_decompose.hpp"

#include "openvr.h"

#include "imstkTimer.h"
#include "imstkInteractorStyle.h"
#include "imstkVulkanRenderer.h"
#include "imstkSurfaceMesh.h"
#include "imstkVulkanInteractorStyle.h"

namespace imstk
{
struct VulkanVRDevice
{
    vr::TrackedDevicePose_t m_pose;
    vr::TrackedDeviceClass m_type;
    std::string m_ID; ///< combination of serial number and model number

    bool m_rendered = false;
    vr::RenderModel_t * m_renderModel;
    std::string m_renderModelName;
    vr::TextureID_t m_textureID;
    std::shared_ptr<VisualModel> m_visualModel;
};

///
/// \class VulkanInteractorStyleVR
///
/// \brief Interactor for VR
///
class VulkanInteractorStyleVR : public VulkanInteractorStyle
{
public:
    ///
    /// \brief Default constructor
    ///
    VulkanInteractorStyleVR();

    ///
    /// \brief Initialize interactor
    ///
    void initialize(std::shared_ptr<VulkanRenderer> renderer);

    ///
    /// \brief Update VR devices
    ///
    void updateVRDevices();

    ///
    /// \brief Device interactor
    ///
    void addVisualVRObject(std::unique_ptr<VulkanVRDevice>& device);

    ///
    /// \brief Get device ID
    ///
    const std::string getDeviceID(const uint32_t index);

    ///
    /// \brief Get string property from device
    /// \param index index of device
    /// \param stringProperty property that should be returned
    ///
    ///
    const std::string getDeviceStringProperty(
        const uint32_t index,
        vr::ETrackedDeviceProperty stringProperty);

    ///
    /// \brief Update function for VR
    /// Computes/extracts matrices for devices (HMD, controllers, etc.)
    ///
    virtual void OnTimer();

protected:
    friend VulkanViewer;
    std::shared_ptr<VulkanRenderer> m_renderer;

    vr::TrackedDevicePose_t m_devicePoses[16];

    std::map<std::string, std::unique_ptr<VulkanVRDevice>> m_devices; ///< Key is unique ID

    std::map<std::string, vr::RenderModel_t*> m_VRModels;

    std::map<vr::TextureID_t, vr::RenderModel_TextureMap_t*> m_VRTextures;
};
}

#endif
