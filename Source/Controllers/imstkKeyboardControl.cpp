/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkKeyboardControl.h"
#include "imstkKeyboardDeviceClient.h"

namespace imstk
{
void
KeyboardControl::setDevice(std::shared_ptr<DeviceClient> device)
{
    // Remove old observer if it exists
    if (m_deviceClient != nullptr)
    {
        disconnect(m_deviceClient, this, &KeyboardDeviceClient::keyPress);
        disconnect(m_deviceClient, this, &KeyboardDeviceClient::keyRelease);
    }

    // Set the new device
    DeviceControl::setDevice(device);

    // Subscribe to the device clients events
    connect(device, &KeyboardDeviceClient::keyPress, this, &KeyboardControl::keyPressEvent);
    connect(device, &KeyboardDeviceClient::keyRelease, this, &KeyboardControl::keyReleaseEvent);
}

void
KeyboardControl::keyPressEvent(KeyEvent* keyPressEvent)
{
    OnKeyPress(keyPressEvent->m_key);
}

void
KeyboardControl::keyReleaseEvent(KeyEvent* keyPressEvent)
{
    OnKeyRelease(keyPressEvent->m_key);
}
} // namespace imstk