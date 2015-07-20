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

#ifndef SMLIGHTMOTIONEVENT_H
#define SMLIGHTMOTIONEVENT_H

// SimMedTK includes
#include "Core/Event.h"
#include <Core/Vector.h>

namespace mstk {
namespace Event {

class smLightMotionEvent : public Event
{
public:
    static EventType EventName;

public:
    smLightMotionEvent(const int &lightIndex);

    void setPosition(const core::Vec3d &lightPosition);

    const core::Vec3d &getPosition();

    void setDirection(const core::Vec3d &lightDirection);

    const core::Vec3d &getDirection();

    void setLightIndex(const int &lightIndex);

    const int &getLightIndex();

private:
    int index; // light index
    core::Vec3d position; // light position
    core::Vec3d direction; // direction
};

} // Event namespace
} // mstk namespace

#endif // SMLIGHTMOTIONEVENT_H
