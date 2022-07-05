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
void
MouseControl::setDevice(std::shared_ptr<MouseDeviceClient> device)
{
    // Remove old observer if it exists
    if (m_deviceClient != nullptr)
    {
        disconnect(m_deviceClient, this, &MouseDeviceClient::mouseButtonPress);
        disconnect(m_deviceClient, this, &MouseDeviceClient::mouseButtonRelease);
        disconnect(m_deviceClient, this, &MouseDeviceClient::mouseScroll);
        disconnect(m_deviceClient, this, &MouseDeviceClient::mouseMove);
    }

    // Set the new device
    DeviceControl::setDevice(device);

    // Subscribe to the device clients events
    connect(device, &MouseDeviceClient::mouseButtonPress, this, &MouseControl::mouseButtonPressEvent);
    connect(device, &MouseDeviceClient::mouseButtonRelease, this, &MouseControl::mouseButtonReleaseEvent);
    connect(device, &MouseDeviceClient::mouseScroll, this, &MouseControl::mouseScrollEvent);
    connect(device, &MouseDeviceClient::mouseMove, this, &MouseControl::mouseMoveEvent);
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
    OnMouseMove(m_deviceClient->getPos());
}
} // namespace imstk