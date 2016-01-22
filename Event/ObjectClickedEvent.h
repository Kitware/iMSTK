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

#ifndef EVENT_OBJECTCLICKEDEVENT_H
#define EVENT_OBJECTCLICKEDEVENT_H

// iMSTK includes
#include "Core/Event.h"
#include "Core/Vector.h"

namespace event {

class ObjectClickedEvent : public core::Event
{
public:
    static core::EventType EventName;

public:
    ObjectClickedEvent(const size_t &objectId);

    void setWindowCoord(const core::Vec3d &coordinates);

    const core::Vec3d &getWindowCoord();

private:
    size_t id; // object id
    core::Vec3d coord; // position
};

} // event namespace

#endif // EVENT_OBJECTCLICKEDEVENT_H
