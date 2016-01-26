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

#ifndef EVENT_KEYBOARDEVENT_H
#define EVENT_KEYBOARDEVENT_H

// iMSTK includes
#include "Core/Event.h"
#include "Event/Key.h"

namespace imstk {

class KeyboardEvent : public Event
{
public:
    static EventType EventName;

public:
    KeyboardEvent(const Key &button);

    const Key &getKeyPressed();

    void setPressed(const bool &press);

    const bool &getPressed();

    const bool &togglePressed();

    void setModifierKey(const ModKey &modKey);

    const ModKey &getModifierKey();

private:
    bool pressed; ///< If the key was pressed or released in this event
    Key key; ///< Key that was pressed
    ModKey modKey; ///< Modifier keys.  See ModKey for values
};

} // event namespace

#endif // EVENT_KEYBOARDEVENT_H
