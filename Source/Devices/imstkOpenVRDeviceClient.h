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

#include <unordered_map>

class vtkInteractorStyleVR;

namespace imstk
{
///
/// \class OpenVRDeviceClient
///
/// \brief This class provides quantities for the specified VR device
/// The devices creation is tied to the viewer. It is only acquirable
/// from a VR viewer and exists on the viewers thread.
///
class OpenVRDeviceClient : public DeviceClient
{
protected:
    ///
    /// This object is only creatable through its New method
    ///
    OpenVRDeviceClient(DeviceType deviceType) : DeviceClient("OpenVRDevice", ""),
        m_deviceType(deviceType), m_trackpadPosition(Vec2d::Zero()) { }

    ///
    /// This object is only creatable through this method
    ///
    static std::shared_ptr<OpenVRDeviceClient> New(DeviceType deviceType);

public:
    virtual ~OpenVRDeviceClient() override = default;

    // Only the viewer is allowed to provide these objects
    friend class ::vtkInteractorStyleVR;

public:
    DeviceType getDeviceType() const { return m_deviceType; }

    const Vec2d& getTrackpadPosition() { return m_trackpadPosition; }
    void setTrackpadPosition(const Vec2d& pos) { m_trackpadPosition = pos; }

    ///
    /// \brief Set the current position and orientation
    ///
    void setPose(const Vec3d& pos, const Quatd& orientation)
    {
        m_trackingEnabled = true;
        m_position    = pos;
        m_orientation = orientation;
    }

protected:
    ///
    /// \brief Emit various button events
    ///
    void emitButtonTouched(const int buttonId);
    void emitButtonUntouched(const int buttonId);
    void emitButtonPress(const int buttonId);
    void emitButtonRelease(const int buttonId);

private:
    DeviceType m_deviceType;
    Vec2d      m_trackpadPosition;
};
}