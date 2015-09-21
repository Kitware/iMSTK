// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SM_VRPNPHANTOMDEVICE_H
#define SM_VRPNPHANTOMDEVICE_H

#include "Core/Vector.h"
#include "Core/Quaternion.h"
#include "Core/Timer.h"
#include "Devices/DeviceInterface.h"

#include <vrpn_Types.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Tracker.h>

#include <string>
#include <memory>
#include <chrono>
#include <array>

///
/// \brief An interface for the Phantom Omni or Geomagic Touch using VRPN
///
class VRPNPhantomDevice : public DeviceInterface
{
public:
    ///
    /// \brief Constructor/Destructor
    ///
    VRPNPhantomDevice();
    virtual ~VRPNPhantomDevice();

    ///
    /// \brief Open the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    DeviceInterface::Message openDevice() override;

    ///
    /// \brief Close the connection to the VRPN server
    /// \return DeviceInterface::Message::Success on success and
    /// DeviceInterface::Message::Failure on failure
    ///
    DeviceInterface::Message closeDevice() override;

    ///
    /// \brief Initialize module
    ///
    void init() override;

    ///
    /// \brief Execute this module
    ///
    void exec() override;

    ///
    /// \brief Module overrides, executed at begining and end of rendering computing
    ///     operations, respectively.
    ///
    void beginFrame() override;
    void endFrame() override;

    ///
    /// \brief Set the url used to connect to the VRPN server
    /// \detail Example URLs: Phantom0@localhost, PHANToM2@10.0.0.13
    /// \param s The new device URL to use.
    ///
    void setDeviceURL(const std::string s);

    ///
    /// \brief Get the phantom's device URL
    /// \return The current device URL used to connect to the VRPN server
    ///
    const std::string &getDeviceURL() const;

    ///
    /// \brief Set the polling delay for the phantom device
    /// \param d The polling delay to set
    ///
    void setPollDelay(const std::chrono::milliseconds &d);

    ///
    /// \brief Get the polling delay
    /// \return The current polling delay between cycles
    ///
    const std::chrono::milliseconds &getPollDelay() const;

    ///
    /// \brief Get the force of the phantom omni
    /// \return The absolute force emitting from the device
    ///
    const core::Vec3d &getForce() const;

    ///
    /// \brief Get the position of the phantom omni
    /// \return The absolute position of the device
    ///
    const core::Vec3d &getPosition() const;

    ///
    /// \brief Get the orientation of the phantom omni
    /// \return The absolute orientation of the device
    ///
    const core::Quaterniond &getOrientation() const;

    ///
    /// \brief Get the status of a button
    /// \param i The button to read(0 = 1st button, 1 = 2nd button)
    /// \return True if the button is pressed, false if it is not
    ///
    bool getButton(size_t i) const;

    ///
    /// \brief Get the time elapsed since the last force reading
    /// \return The time elapsed since the last force reading
    ///
    long double getForceETime();

    ///
    /// \brief Get the time elapsed since the last position reading
    /// \return The time elapsed since the last position reading
    ///
    long double getPositionETime();

    ///
    /// \brief Get the time elapsed since the last orientation reading
    /// \return The time elapsed since the last orientation reading
    ///
    long double getOrientationETime();

    ///
    /// \brief Get the time elapsed since the last button reading
    /// \param i The button to read(0 = 1st button, 1 = 2nd button)
    /// \return The time elapsed since the last button reading
    ///
    long double getButtonETime(size_t i);

private:
    std::string deviceURL;
    std::chrono::milliseconds delay;

    std::array<bool, 2> buttons; //!< Buttons: true = pressed/false = not pressed
    core::Vec3d force; //!< Force vector for the phantom omni
    core::Vec3d pos; //!< Position of the phantom omni end effector
    core::Quaterniond quat; //!< Orientation of the end effector

    std::array<core::Timer, 2> buttonTimers; //!< Timer to track last update for buttons
    core::Timer forceTimer; //!< Timer to track last update for force
    core::Timer posTimer; //!< Timer to track last update for position
    core::Timer quatTimer; //!< Timer to track last update for orientation

    std::shared_ptr<vrpn_Button_Remote> vrpnButton; //!< VRPN button interface
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce; //!< VRPN force interface
    std::shared_ptr<vrpn_Tracker_Remote> vrpnTracker; //!< VRPN position/orientation interface

    ///
    /// \brief Executes the mainloop of each of the VRPN interfaces to obtain data
    /// \detail If there is new data, the callback functions will be called
    ///
    void processChanges();

    ///
    /// \brief VRPN call back for button changed (pressed or released)
    /// \param userData Pointer to this VRPNPhantomDevice to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new button data
    ///
    static void VRPN_CALLBACK buttonChangeHandler(void *userData, const vrpn_BUTTONCB b);

    ///
    /// \brief VRPN call back for force data
    /// \param userData Pointer to this VRPNPhantomDevice to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new force data
    ///
    static void VRPN_CALLBACK forceChangeHandler(void *userData, const vrpn_FORCECB f);

    ///
    /// \brief VRPN call back for position and orientation data
    /// \param userData Pointer to this VRPNPhantomDevice to allow updating
    /// internal data
    /// \param b VRPN callback structure containing new position and
    /// orientation data
    ///
    static void VRPN_CALLBACK trackerChangeHandler(void *userData, const vrpn_TRACKERCB b);
};

#endif
