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

#include "Event/KeyboardEvent.h"

namespace imstk {

EventType KeyboardEvent::EventName = EventType::Keyboard;

KeyboardEvent::KeyboardEvent(const Key& button)
    : pressed(false), key(button), modKey(ModKey::none)
{
}

const Key& KeyboardEvent::getKeyPressed()
{
    return key;
}
void KeyboardEvent::setPressed(const bool& press)
{
    this->pressed = press;
}
const bool& KeyboardEvent::getPressed()
{
    return this->pressed;
}
const bool& KeyboardEvent::togglePressed()
{
    return this->pressed = !this->pressed;
}
void KeyboardEvent::setModifierKey(const ModKey& modKey)
{
    this->modKey = modKey;
}
const ModKey& KeyboardEvent::getModifierKey()
{
    return this->modKey;
}

} // event namespace
