// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef EVENT_CAMERAEVENT_H
#define EVENT_CAMERAEVENT_H

// iMSTK includes
#include "Core/Event.h"
#include "Core/Vector.h"
#include "Core/Quaternion.h"

namespace imstk {

class CameraEvent : public Event
{
public:
    static EventType EventName;

public:
    CameraEvent();

    void setPosition(const Vec3d &cameraPosition);

    const Vec3d &getPosition();

    void setDirection(const Vec3d &cameraDirection);

    const Vec3d &getDirection();

    void setUpDirection(const Vec3d &cameraUpDirection);

    const Vec3d &getUpDirection();

    void applyRotation(const Quaterniond &quat)
    {
        direction = quat*direction;
        upDirection = quat*upDirection;
    }

private:
    Vec3d position; //  camera position
    Vec3d direction; // direction
    Vec3d upDirection; //  upward direction
};

} // event namespace

#endif // EVENT_CAMERAEVENT_H
