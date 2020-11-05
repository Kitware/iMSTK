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

#include "imstkMath.h"
#include "imstkEventObject.h"

#include <unordered_map>

namespace imstk
{
using DeviceType = int;
#define UNKNOWN_DEVICE 0
#define OPENHAPTICS_DEVICE 1
#define OPENVR_LEFT_CONTROLLER 3
#define OPENVR_RIGHT_CONTROLLER 4
#define OPENVR_HMD 5

using ButtonStateType = int;
#define BUTTON_RELEASED 0
#define BUTTON_TOUCHED 1
#define BUTTON_UNTOUCHED 2
#define BUTTON_PRESSED 3

class ButtonEvent : public Event
{
public:
    ButtonEvent(const int button, const ButtonStateType keyPressType) : Event(EventType::DeviceButtonPress, 1),
        m_buttonState(keyPressType),
        m_button(button)
    {
    }

    virtual ~ButtonEvent() override = default;

public:
    ButtonStateType m_buttonState;
    const int       m_button = -1;
};

///
/// \class DeviceClient
///
/// \brief The device client's represents the device and provides
/// an interface to acquire data from a device.
/// It posts events the device may have as well as provides the state
/// \todo HAVE AN ABSTRACK BASE CLASS FOR THE DEVICE CLIENT
///
class DeviceClient : public EventObject
{
public:
    ///
    /// \brief Destructor
    ///
    virtual ~DeviceClient() = default;

public:
    ///
    /// \brief Get/Set the device IP
    ///
    const std::string& getIp();
    void setIp(const std::string& ip);

    ///
    /// \brief Get/Set the device name
    ///
    const std::string& getDeviceName();
    void setDeviceName(const std::string& deviceName);

    ///
    /// \brief Get/Set what listeners to enable on the device: tracking, analogic, force, buttons.
    ///
    const bool& getTrackingEnabled() const;
    void setTrackingEnabled(const bool& status);
    const bool& getAnalogicEnabled() const;
    void setAnalogicEnabled(const bool& status);
    const bool& getButtonsEnabled() const;
    void setButtonsEnabled(const bool& status);
    const bool& getForceEnabled() const;
    void setForceEnabled(const bool& status);

    ///
    /// \brief Get the device position
    ///
    const Vec3d& getPosition() const;

    ///
    /// \brief Get the device velocity
    ///
    const Vec3d& getVelocity() const;

    ///
    /// \brief Get the device orientation
    ///
    const Quatd& getOrientation() const;

    ///
    /// \brief Get/Set the device force
    ///
    const Vec3d& getForce() const;
    void setForce(Vec3d force);

    const std::unordered_map<int, int>& getButtons() const { return m_buttons; }

    ///
    /// \brief Get the state of a button
    /// returns 0 if can't find button
    ///
    const int getButton(const int buttonId) const
    {
        if (m_buttons.find(buttonId) != m_buttons.end())
        {
            return m_buttons.at(buttonId);
        }
        else
        {
            return 0;
        }
    }

    virtual void update() {}

protected:
    DeviceClient(const std::string& name, const std::string& ip);

    std::string m_deviceName;                ///< Device Name
    std::string m_ip;                        ///< Connection device IP

    bool m_trackingEnabled = true;           ///< Tracking enabled if true
    bool m_analogicEnabled = true;           ///< Analogic enabled if true
    bool m_buttonsEnabled  = true;           ///< Buttons enabled if true
    bool m_forceEnabled    = false;          ///< Force enabled if true

    Vec3d m_position;                        ///< Position of end effector
    Vec3d m_velocity;                        ///< Linear velocity of end effector
    Quatd m_orientation;                     ///< Orientation of the end effector
    Vec3d m_force;                           ///< Force vector

    std::unordered_map<int, int> m_buttons;
};
}
