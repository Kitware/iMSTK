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

#include "imstkDeviceClient.h"

namespace imstk
{
using DeviceType = int;
#define OPENVR_LEFT_CONTROLLER 0
#define OPENVR_RIGHT_CONTROLLER 1
#define OPENVR_HMD 2

///
/// \class OpenVRDeviceClient
///
/// \brief This class provides quantities for the specified VR device
///
class OpenVRDeviceClient : public DeviceClient
{
public:
    OpenVRDeviceClient(DeviceType deviceType) : DeviceClient("OpenVRDevice", ""), m_deviceType(deviceType) { }
    virtual ~OpenVRDeviceClient() override = default;

    DeviceType getDeviceType() const { return m_deviceType; }

    void setPose(Vec3d pos, Quatd orientation)
    {
        m_position    = pos;
        m_orientation = orientation;
    }

private:
    friend class OpenVRDeviceServer;

    DeviceType m_deviceType;
};
}