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

namespace smtk {
namespace Event {

class smHapticEvent : public smEvent
{
public:
    static EventType EventName;

public:
    smHapticEvent(const size_t &deviceId, const std::string &deviceName);

    void setPosition(const smVec3d &coordinates);

    const smVec3d &getPosition();

    void setVelocity(const smVec3d &deviceVelocity);

    const smVec3d &getVelocity();

    void setAngles(const smVec3d &deviceAngles);

    const smVec3d &getAngles();

    const smMatrix44d &getTransform();

    void setTransform(const smMatrix44d &deviceTransform);

    const smVec3d &getForce();

    void setForce(const smVec3d &deviceForce);

    const smVec3d &getTorque();

    void setTorque(const smVec3d &deviceTorque);

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
    smVec3d position; // position
    smVec3d velocity; // velocity of the tip
    smVec3d angles; // angles
    smVec3d force; // input force
    smVec3d torque; // input torque
    smMatrix44d transform; // transformation matrix
    std::array<bool,4> buttonState;//will be chnage later on
};

} // Event namespace
} // smtk namespace

#endif // SMHAPTICEVENT_H
