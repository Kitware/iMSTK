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

#ifndef SM_HAPTIC_INTERFACE_H
#define SM_HAPTIC_INTERFACE_H

#include "Devices/DeviceInterface.h"

#include "Core/Matrix.h"

#define SM_MAX_BUTTONS 4

/// \brief holds haptic device data
struct hapticDeviceData_t
{
    int deviceID;
    std::string deviceName;
    core::Vec3d position;
    core::Vec3d velocity;
    core::Vec3d angles;
    Matrix44d transform;
    bool buttonState[SM_MAX_BUTTONS];
};

/// \brief Absract base class for haptic device
class HapticInterface: public DeviceInterface
{

public:

    /// \brief constructor
    HapticInterface();

    /// \brief destructor
    virtual ~HapticInterface() {};

    /// \brief open haptic device
    virtual Message openDevice()
    {
        return Message::Unknown;
    }

    /// \brief close haptic device
    virtual Message closeDevice()
    {
        return Message::Unknown;
    }

    /// \brief start the haptic device
    virtual Message startDevice()
    {
        return Message::Unknown;
    }

    /// \brief get the position of the end effector the haptic device
    virtual Message  getPosition(core::Vec3d & d_pos)
    {
        return Message::Unknown;
    }

    /// \brief get the orientation of the end effector the haptic device
    virtual Message getOreintation(Matrix33d  *d_rot)
    {
        return Message::Unknown;
    }

    /// \brief get the transform (position + orientation) of the end effector the haptic device
    virtual Message getDeviceTransform(Matrix44d  *d_transform)
    {
        return Message::Unknown;
    }

    /// \brief set force to the haptic device
    virtual Message setForce(core::Vec3d & force)
    {
        return Message::Unknown;
    }

    /// \brief set torque to the haptic device
    virtual Message setForceandTorque(core::Vec3d & force, core::Vec3d  & torque)
    {
        return Message::Unknown;
    }

};

#endif
