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
#include "imstkSpinLock.h"

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
    ButtonEvent(std::string type, const int button, const ButtonStateType keyPressType) : Event(type),
        m_buttonState(keyPressType),
        m_button(button)
    {
    }

    virtual ~ButtonEvent() override = default;

    ButtonStateType m_buttonState;
    const int       m_button = -1;
};

///
/// \class DeviceClient
///
/// \brief The device client's represents the device and provides
/// an interface to acquire data from a device.
/// It posts events the device may have as well as provides the state
/// \todo Abstract base class for device client
///
class DeviceClient : public EventObject
{
public:
    ///
    /// \brief Destructor
    ///
    virtual ~DeviceClient() = default;

    // *INDENT-OFF*
    SIGNAL(DeviceClient,buttonStateChanged);
    // *INDENT-ON*

    ///
    /// \brief Get/Set the device IP
    ///
    const std::string& getIp() { return m_ip; }
    void setIp(const std::string& ip) { m_ip = ip; }

    ///
    /// \brief Get/Set the device name
    ///
    const std::string& getDeviceName() { return m_deviceName; }
    void setDeviceName(const std::string& deviceName) { m_deviceName = deviceName; }

    ///
    /// \brief Get/Set what listeners to enable on the device: tracking, analogic, force, buttons.
    ///
    const bool getTrackingEnabled() const { return m_trackingEnabled; }
    void setTrackingEnabled(const bool status) { m_trackingEnabled = status; }
    const bool getAnalogicEnabled() const { return m_analogicEnabled; }
    void setAnalogicEnabled(const bool status) { m_analogicEnabled = status; }
    const bool getButtonsEnabled() const { return m_buttonsEnabled; }
    void setButtonsEnabled(const bool status) { m_buttonsEnabled = status; }
    const bool getForceEnabled() const { return m_forceEnabled; }
    void setForceEnabled(const bool status) { m_forceEnabled = status; }

    ///
    /// \brief Get the device position
    ///
    const Vec3d getPosition();

    ///
    /// \brief Get the device velocity
    ///
    const Vec3d getVelocity();

    ///
    /// \brief Get the device angular velocity
    ///
    const Vec3d getAngularVelocity();

    ///
    /// \brief Get the device orientation
    ///
    const Quatd getOrientation();

    ///
    /// \brief Get offset from position for device end effector
    ///
    const Vec3d& getOffset() const { return m_endEffectorOffset; }

    ///
    /// \brief Get/Set the device force
    ///
    const Vec3d getForce();
    void setForce(Vec3d force);

    ///
    /// \brief Get button map
    ///
    const std::unordered_map<int, int>& getButtons() const;

    ///
    /// \brief Get the state of a button
    /// returns 0 if can't find button
    ///
    const int getButton(const int buttonId)
    {
        int result = 0;
        m_dataLock.lock();
        if (m_buttons.find(buttonId) != m_buttons.end())
        {
            result = m_buttons.at(buttonId);
        }
        m_dataLock.unlock();
        return result;
    }

    const std::vector<double> getAnalog() const;

    ///
    /// \brief Do runtime logic
    ///
    virtual void update() {}

protected:
    DeviceClient(const std::string& name, const std::string& ip);

    std::string m_deviceName;                         ///< Device Name
    std::string m_ip;                                 ///< Connection device IP

    bool m_trackingEnabled = true;                    ///< Tracking enabled if true
    bool m_analogicEnabled = true;                    ///< Analogic enabled if true
    bool m_buttonsEnabled  = true;                    ///< Buttons enabled if true
    bool m_forceEnabled    = false;                   ///< Force enabled if true

    Vec3d m_position;                                 ///< Position of end effector
    Vec3d m_velocity;                                 ///< Linear velocity of end effector
    Vec3d m_angularVelocity;                          ///< Angular velocity of the end effector
    Quatd m_orientation;                              ///< Orientation of the end effector
    Vec3d m_force;                                    ///< Force vector
    Vec3d m_endEffectorOffset = Vec3d(0.0, 0.0, 0.0); ///< Offset from origin

    std::unordered_map<int, int> m_buttons;
    std::vector<double> m_analogChannels;

    ParallelUtils::SpinLock m_transformLock;    /// > Used for devices filling data from other threads
    ParallelUtils::SpinLock m_forceLock;        /// > Used for devices filling data from other threads
    mutable ParallelUtils::SpinLock m_dataLock; /// > Used for button and analog data
private:
};
}
