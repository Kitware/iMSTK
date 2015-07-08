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
#include "smCore/smEvent.h"
#include "smCore/smVector.h"
#include "smCore/smQuaternion.h"

namespace smtk {
namespace Event {

class smCameraEvent : public smEvent
{
public:
    static EventType EventName;

public:
    smCameraEvent();

    void setPosition(const smVec3d &cameraPosition);

    const smVec3d &getPosition();

    void setDirection(const smVec3d &cameraDirection);

    const smVec3d &getDirection();

    void setUpDirection(const smVec3d &cameraUpDirection);

    const smVec3d &getUpDirection();

    void applyRotation(const smQuaterniond &quat)
    {
        direction = quat*direction;
        upDirection = quat*upDirection;
    }

private:
    smVec3d position; //  camera position
    smVec3d direction; // direction
    smVec3d upDirection; //  upward direction
};

} // Event namespace
} // smtk namespace

#endif // SMCAMERAEVENT_H
