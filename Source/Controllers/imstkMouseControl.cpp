/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkMouseControl.h"
#include "imstkMouseDeviceClient.h"

namespace imstk
{
void
MouseControl::setDevice(std::shared_ptr<DeviceClient> device)
{
    // Remove old observer if it exists
    if (m_deviceClient != nullptr)
    {
        disconnect(m_deviceClient, shared_from_this(), &MouseDeviceClient::mouseButtonPress);
        disconnect(m_deviceClient, shared_from_this(), &MouseDeviceClient::mouseButtonRelease);
        disconnect(m_deviceClient, shared_from_this(), &MouseDeviceClient::mouseScroll);
        disconnect(m_deviceClient, shared_from_this(), &MouseDeviceClient::mouseMove);
    }

    // Set the new device
    m_mouseDevice = std::dynamic_pointer_cast<MouseDeviceClient>(device);
    DeviceControl::setDevice(device);

    // Subscribe to the device clients events
    connect(device, &MouseDeviceClient::mouseButtonPress, shared_from_this(), &MouseControl::mouseButtonPressEvent);
    connect(device, &MouseDeviceClient::mouseButtonRelease, shared_from_this(), &MouseControl::mouseButtonReleaseEvent);
    connect(device, &MouseDeviceClient::mouseScroll, shared_from_this(), &MouseControl::mouseScrollEvent);
    connect(device, &MouseDeviceClient::mouseMove, shared_from_this(), &MouseControl::mouseMoveEvent);
}

void
MouseControl::mouseButtonPressEvent(MouseEvent* e)
{
    OnButtonPress(e->m_buttonId);
}

void
MouseControl::mouseButtonReleaseEvent(MouseEvent* e)
{
    OnButtonRelease(e->m_buttonId);
}

void
MouseControl::mouseScrollEvent(MouseEvent* e)
{
    OnScroll(e->m_scrollDx);
}

void
MouseControl::mouseMoveEvent(MouseEvent* imstkNotUsed(e))
{
    OnMouseMove(m_mouseDevice->getPos());
}
} // namespace imstk