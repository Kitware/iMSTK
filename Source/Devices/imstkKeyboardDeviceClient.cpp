/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkKeyboardDeviceClient.h"

namespace imstk
{
std::shared_ptr<KeyboardDeviceClient>
KeyboardDeviceClient::New()
{
    return std::shared_ptr<KeyboardDeviceClient>(new KeyboardDeviceClient());
}

void
KeyboardDeviceClient::emitKeyDown(char key)
{
    const int prevKeyState = m_buttons[key];
    m_buttons[key] = KEY_PRESS;
    if (prevKeyState != KEY_PRESS)
    {
        emit(KeyPressEvent(key, KEY_PRESS));
    }
}

void
KeyboardDeviceClient::emitKeyUp(char key)
{
    const int prevKeyState = m_buttons[key];
    m_buttons[key] = KEY_RELEASE;
    if (prevKeyState != KEY_RELEASE)
    {
        emit(KeyPressEvent(key, KEY_RELEASE));
    }
}

bool
KeyboardDeviceClient::isKeyDown(const char key) const
{
    if (m_buttons.find(key) != m_buttons.end())
    {
        return (m_buttons.at(key) == KEY_PRESS);
    }
    else
    {
        return false;
    }
}
}