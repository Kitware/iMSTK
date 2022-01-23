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

#include "imstkMouseControl.h"
#include "imstkMouseDeviceClient.h"

namespace imstk
{
MouseControl::MouseControl(std::shared_ptr<MouseDeviceClient> mouseDevice) :
    DeviceControl(mouseDevice)
{
    setDevice(mouseDevice);
}

void
MouseControl::setDevice(std::shared_ptr<DeviceClient> device)
{
    // Remove old observer if it exists
    if (m_mouseDeviceClient != nullptr)
    {
        disconnect(m_mouseDeviceClient, this, &MouseDeviceClient::mouseButtonPress);
        disconnect(m_mouseDeviceClient, this, &MouseDeviceClient::mouseButtonRelease);
        disconnect(m_mouseDeviceClient, this, &MouseDeviceClient::mouseScroll);
        disconnect(m_mouseDeviceClient, this, &MouseDeviceClient::mouseMove);
    }

    // Set the new device
    m_mouseDeviceClient = std::dynamic_pointer_cast<MouseDeviceClient>(device);
    DeviceControl::setDevice(device);

    // Subscribe to the device clients events
    connect(m_mouseDeviceClient, &MouseDeviceClient::mouseButtonPress, this, &MouseControl::mouseButtonPressEvent);
    connect(m_mouseDeviceClient, &MouseDeviceClient::mouseButtonRelease, this, &MouseControl::mouseButtonReleaseEvent);
    connect(m_mouseDeviceClient, &MouseDeviceClient::mouseScroll, this, &MouseControl::mouseScrollEvent);
    connect(m_mouseDeviceClient, &MouseDeviceClient::mouseMove, this, &MouseControl::mouseMoveEvent);
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
    OnMouseMove(m_mouseDeviceClient->getPos());
}
} // namespace imstk