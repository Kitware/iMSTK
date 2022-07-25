/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOpenVRDeviceClient.h"

namespace imstk
{
std::shared_ptr<OpenVRDeviceClient>
OpenVRDeviceClient::New(DeviceType deviceType)
{
    return std::shared_ptr<OpenVRDeviceClient>(new OpenVRDeviceClient(deviceType));
}

void
OpenVRDeviceClient::emitButtonTouched(const int buttonId)
{
    const int prevButtonState = m_buttons[buttonId];
    m_buttons[buttonId] = BUTTON_TOUCHED;
    if (prevButtonState != BUTTON_TOUCHED)
    {
        this->postEvent(ButtonEvent(OpenVRDeviceClient::buttonStateChanged(), buttonId, BUTTON_TOUCHED));
    }
}

void
OpenVRDeviceClient::emitButtonUntouched(const int buttonId)
{
    const int prevButtonState = m_buttons[buttonId];
    m_buttons[buttonId] = BUTTON_UNTOUCHED;
    if (prevButtonState != BUTTON_UNTOUCHED)
    {
        this->postEvent(ButtonEvent(OpenVRDeviceClient::buttonStateChanged(), buttonId, BUTTON_UNTOUCHED));
    }
}

void
OpenVRDeviceClient::emitButtonPress(const int buttonId)
{
    const int prevButtonState = m_buttons[buttonId];
    m_buttons[buttonId] = BUTTON_PRESSED;
    if (prevButtonState != BUTTON_PRESSED)
    {
        this->postEvent(ButtonEvent(OpenVRDeviceClient::buttonStateChanged(), buttonId, BUTTON_PRESSED));
    }
}

void
OpenVRDeviceClient::emitButtonRelease(const int buttonId)
{
    const int prevButtonState = m_buttons[buttonId];
    m_buttons[buttonId] = BUTTON_RELEASED;
    if (prevButtonState != BUTTON_RELEASED)
    {
        this->postEvent(ButtonEvent(OpenVRDeviceClient::buttonStateChanged(), buttonId, BUTTON_RELEASED));
    }
}
} // namespace imstk