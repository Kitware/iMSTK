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

#ifndef SMHAPTICEVENT_H
#define SMHAPTICEVENT_H

// STL includes
#include <string>
#include <array>

// SimMedTK includes
#include "Core/Event.h"
#include <Core/Vector.h>
#include <Core/Matrix.h>

namespace mstk {
namespace Event {

class smHapticEvent : public Event
{
public:
    static EventType EventName;

public:
    smHapticEvent(const size_t &deviceId, const std::string &deviceName);

    void setPosition(const core::Vec3d &coordinates);

    const core::Vec3d &getPosition();

    void setVelocity(const core::Vec3d &deviceVelocity);

    const core::Vec3d &getVelocity();

    void setAngles(const core::Vec3d &deviceAngles);

    const core::Vec3d &getAngles();

    const Matrix44d &getTransform();

    void setTransform(const Matrix44d &deviceTransform);

    const core::Vec3d &getForce();

    void setForce(const core::Vec3d &deviceForce);

    const core::Vec3d &getTorque();

    void setTorque(const core::Vec3d &deviceTorque);

    size_t getDeviceId()
    {
        return id;
    }

    const bool &getButtonState(size_t i)
    {
        return buttonState.at(i);
    }

    void setButtonState(size_t i, bool state)
    {
        buttonState.at(i) = state;
    }

private:
    size_t id; // device id
    std::string name; // device name
    core::Vec3d position; // position
    core::Vec3d velocity; // velocity of the tip
    core::Vec3d angles; // angles
    core::Vec3d force; // input force
    core::Vec3d torque; // input torque
    Matrix44d transform; // transformation matrix
    std::array<bool,4> buttonState;//will be chnage later on
};

} // Event namespace
} // mstk namespace

#endif // SMHAPTICEVENT_H
