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

#include "imstkKeyboardControl.h"
#include "imstkKeyboardDeviceClient.h"

namespace imstk
{
KeyboardControl::KeyboardControl(std::shared_ptr<KeyboardDeviceClient> keyDevice) :
    DeviceControl(keyDevice)
{
    setDevice(keyDevice);
}

void
KeyboardControl::setDevice(std::shared_ptr<DeviceClient> device)
{
    // Remove old observer if it exists
    if (m_keyboardDeviceClient != nullptr)
    {
        disconnect(m_keyboardDeviceClient, this, &KeyboardDeviceClient::keyPress);
        disconnect(m_keyboardDeviceClient, this, &KeyboardDeviceClient::keyRelease);
    }

    // Set the new device
    m_keyboardDeviceClient = std::dynamic_pointer_cast<KeyboardDeviceClient>(device);
    DeviceControl::setDevice(device);

    // Subscribe to the device clients events
    connect(m_keyboardDeviceClient, &KeyboardDeviceClient::keyPress, this, &KeyboardControl::keyPressEvent);
    connect(m_keyboardDeviceClient, &KeyboardDeviceClient::keyRelease, this, &KeyboardControl::keyReleaseEvent);
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
}