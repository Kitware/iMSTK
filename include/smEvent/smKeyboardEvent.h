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

#ifndef SMKEYBOARDEVENT_H
#define SMKEYBOARDEVENT_H

// SimMedTK includes
#include "smEvent/smEvent.h"
#include "smEvent/smKey.h"

namespace smtk {
namespace Event {

class smKeyboardEvent : public smEvent
{
public:
    static constexpr EventType EventName = EventType::Keyboard;

public:
    smKeyboardEvent(const smKey &button);

    const smKey &getKeyPressed();

    void setPressed(const bool &press);

    const bool &getPressed();

    const bool &togglePressed();

    void setModifierKey(const smModKey &modKey);

    const smModKey &getModifierKey();

private:
    bool pressed; ///< If the key was pressed or released in this event
    smKey key; ///< Key that was pressed
    smModKey modKey; ///< Modifier keys.  See smModKey for values
};

} // Event namespace
} // smtk namespace

#endif // SMKEYBOARDEVENT_H
