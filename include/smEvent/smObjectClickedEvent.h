/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#ifndef SMOBJECTCLICKEDEVENT_H
#define SMOBJECTCLICKEDEVENT_H

// SimMedTK includes
#include "smEvent/smEvent.h"
#include <smUtilities/smVector.h>

namespace smtk {
namespace Event {

class smObjectClickedEvent : public smEvent
{
public:
    static constexpr EventType EventName = EventType::ObjectClicked;

public:
    smObjectClickedEvent(const size_t &objectId);

    void setWindowCoord(const smVec3d &coordinates);

    const smVec3d &getWindowCoord();

private:
    size_t id; // object id
    smVec3d coord; // position
};

} // Event namespace
} // smtk namespace

#endif // SMOBJECTCLICKEDEVENT_H
