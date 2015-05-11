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

#include "smCore/smEventData.h"

smKeyboardEventData::smKeyboardEventData() :
    keyBoardKey{smKey::Unknown},
    pressed{false},
    modKeys{smModKey::none}
{
}

smKeyboardEventData::smKeyboardEventData(smKey key, smBool pressed, smModKey mods) :
    keyBoardKey{key},
    pressed{pressed},
    modKeys{mods}
{
}

smKeyboardEventData::smKeyboardEventData(smKey key, smBool pressed, smBool shift,
                                         smBool control, smBool alt, smBool super) :
    smKeyboardEventData(key, pressed, smModKey::none)
{
    if (shift)
        this->modKeys |= smModKey::shift;
    if (control)
        this->modKeys |= smModKey::control;
    if (alt)
        this->modKeys |= smModKey::alt;
    if (super)
        this->modKeys |= smModKey::super;
}

smMouseButtonEventData::smMouseButtonEventData() :
    mouseButton{smMouseButton::Unknown},
    pressed{false},
    windowX{-1},
    windowY{-1}
{
}

smMouseButtonEventData::smMouseButtonEventData(smMouseButton button,
                       smBool pressed, smDouble x, smDouble y) :
    mouseButton{button},
    pressed{pressed},
    windowX{x},
    windowY{y}
{
}

smMouseMoveEventData::smMouseMoveEventData() :
    windowX{-1}, windowY{-1}
{
}

smMouseMoveEventData::smMouseMoveEventData(smDouble x, smDouble y) :
    windowX{x}, windowY{y}
{
}
