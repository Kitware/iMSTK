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

#ifndef SMCAMERAEVENT_H
#define SMCAMERAEVENT_H

// SimMedTK includes
#include "Core/Event.h"
#include "Core/Vector.h"
#include "Core/Quaternion.h"

namespace mstk {
namespace Event {

class smCameraEvent : public Event
{
public:
    static EventType EventName;

public:
    smCameraEvent();

    void setPosition(const core::Vec3d &cameraPosition);

    const core::Vec3d &getPosition();

    void setDirection(const core::Vec3d &cameraDirection);

    const core::Vec3d &getDirection();

    void setUpDirection(const core::Vec3d &cameraUpDirection);

    const core::Vec3d &getUpDirection();

    void applyRotation(const Quaterniond &quat)
    {
        direction = quat*direction;
        upDirection = quat*upDirection;
    }

private:
    core::Vec3d position; //  camera position
    core::Vec3d direction; // direction
    core::Vec3d upDirection; //  upward direction
};

} // Event namespace
} // mstk namespace

#endif // SMCAMERAEVENT_H
