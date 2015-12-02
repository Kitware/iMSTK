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

#ifndef SM_DEVICE_H
#define SM_DEVICE_H

#include <chrono>

// SimMedTK includes
#include "Core/Module.h"
#include "Core/Quaternion.h"
#include "Core/Vector.h"
#include "Core/Timer.h"

///
/// \brief Base class for all devices. Device specific implementation should be done
///     by instantiating this class
///
class DeviceInterface: public Module
{

public:
    enum class Message
    {
        Unknown = -2,
        Failure = -1,
        Success = 0
    };

public:
    ///
    /// \brief Constructor
    ///
    DeviceInterface();

    ///
    /// \brief Cestructor
    ///
    virtual ~DeviceInterface();

    ///
    /// \brief Get the polling delay
    /// \return The current polling delay between cycles
    ///
    const std::chrono::milliseconds &getPollDelay() const;

    ///
    /// \brief Set the polling delay for the phantom device
    /// \param d The polling delay to set
    ///
    void setPollDelay(const std::chrono::milliseconds &d);

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
    /// \brief Get the force of the phantom omni
    /// \return The absolute force emitting from the device
    ///
    const core::Vec3d &getForce() const;

    ///
    /// \brief Get the force of the phantom omni
    /// \return The absolute force emitting from the device
    ///
    void setForce(const core::Vec3d &f);

    ///
    /// \brief Get the torque of the phantom omni
    /// \return The absolute force emitting from the device
    ///
    const core::Vec3d &getTorque() const;

    ///
    /// \brief Get the position of the phantom omni
    /// \return The absolute position of the device
    ///
    const core::Vec3d &getPosition() const;

    ///
    /// \brief Get the velocity of the phantom omni
    /// \return The absolute position of the device
    ///
    const core::Vec3d &getVelocity() const;

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
    /// \brief Get the time elapsed since the last button reading
    /// \param i The button to read(0 = 1st button, 1 = 2nd button)
    /// \return The time elapsed since the last button reading
    ///
    long double getButtonETime(size_t i);

    ///
    /// \brief Initialize module
    ///
    virtual bool init() override;

    ///
    /// \brief Begin frame will be called before the cycle.
    ///
    virtual void beginFrame() override;

    ///
    /// \brief End frame will be called after the cycle.
    ///
    virtual void endFrame() override;

    ///
    /// \brief Execution function. Main module execution.
    ///
    virtual void exec() override;

    ///
    /// \brief open the device
    ///
    virtual Message openDevice();

    ///
    /// \brief close the device
    ///
    virtual Message closeDevice();

    ///
    /// \brief write data (for ADU interface device)
    ///
    virtual Message write(void */*Interfacehandle*/, int /*port*/, void */*data*/);

    ///
    /// \brief read data (for ADU interface device)
    ///
    virtual Message read(void */*Interfacehandle*/, int /*port*/, void */*data*/);

protected:
    bool driverInstalled;   //!< true if device driver is installed
    core::Timer forceTimer; //!< Timer to track last update for force
    core::Timer posTimer;   //!< Timer to track last update for position
    core::Timer quatTimer;  //!< Timer to track last update for orientation
    core::Timer velTimer;   //!< Timer to track last update for velocity
    core::Vec3d force;      //!< Force vector
    core::Vec3d torque;     //!< Torque vector
    core::Vec3d position;   //!< Position of end effector
    core::Vec3d velocity;   //!< Linear velocity of end effector
    core::Quaterniond orientation; //!< Orientation of the end effector

    std::vector<bool> buttons; //!< Buttons: true = pressed/false = not pressed
    std::vector<core::Timer> buttonTimers; //!< Timer to track last update for buttons
    std::chrono::milliseconds pollDelay; //!< Poll delay
};

#endif
