/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
        this->postEvent(KeyEvent(KeyboardDeviceClient::keyPress(), key, KEY_PRESS));
    }
}

void
KeyboardDeviceClient::emitKeyUp(char key)
{
    const int prevKeyState = m_buttons[key];
    m_buttons[key] = KEY_RELEASE;
    if (prevKeyState != KEY_RELEASE)
    {
        this->postEvent(KeyEvent(KeyboardDeviceClient::keyRelease(), key, KEY_RELEASE));
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
} // namespace imstk