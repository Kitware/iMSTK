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

#ifndef imstkVulkanInteractorStyleVR_h
#define imstkVulkanInteractorStyleVR_h

#ifdef iMSTK_ENABLE_VR

#include "openvr.h"

#include "imstkTimer.h"
#include "imstkInteractorStyle.h"
#include "imstkVulkanRenderer.h"
#include "imstkVulkanInteractorStyle.h"

namespace imstk
{

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

    virtual void OnTimer();

protected:
    friend VulkanViewer;
    vr::TrackedDevicePose_t m_devicePoses[vr::k_unMaxTrackedDeviceCount];
    vr::TrackedDeviceClass m_deviceClasses[vr::k_unMaxTrackedDeviceCount];
    std::shared_ptr<VulkanRenderer> m_renderer;
};

}

#endif

#endif