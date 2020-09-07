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
using DeviceType = int;
#define OPENVR_LEFT_CONTROLLER 0
#define OPENVR_RIGHT_CONTROLLER 1
#define OPENVR_HMD 2

using VRButtonStateType = int;
#define BUTTON_RELEASED 0
#define BUTTON_TOUCHED 1
#define BUTTON_UNTOUCHED 2
#define BUTTON_PRESSED 3

class VRButtonEvent : public Event
{
public:
    VRButtonEvent(const int button, const VRButtonStateType keyPressType) : Event(EventType::VRButtonPress, 1),
        m_buttonState(keyPressType),
        m_button(button)
    {
    }

    virtual ~VRButtonEvent() override = default;

public:
    VRButtonStateType m_buttonState;
    const int m_button = -1;
};

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
        m_deviceType(deviceType) { }

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
};
}