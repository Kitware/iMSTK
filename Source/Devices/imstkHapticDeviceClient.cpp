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

#include "imstkHapticDeviceClient.h"
#include "imstkLogger.h"

#include <HDU/hduError.h>
#include <HDU/hduVector.h>

namespace imstk
{
void
HapticDeviceClient::initialize()
{
    m_buttons[0] = 0;
    m_buttons[1] = 0;
    m_buttons[2] = 0;
    m_buttons[3] = 0;

    //flush error stack
    HDErrorInfo errorFlush;
    while (HD_DEVICE_ERROR(errorFlush = hdGetError())) {}

    // Initialize the device
    if (m_deviceName == "")
    {
        m_handle = hdInitDevice(HD_DEFAULT_DEVICE);
    }
    else
    {
        m_handle = hdInitDevice(getDeviceName().c_str());
    }

    // If failed
    HDErrorInfo error;
    LOG_IF(FATAL, HD_DEVICE_ERROR(error = hdGetError())) << "Failed to initialize Phantom Omni \"" << this->getDeviceName() + "\"";

    // If initialized as default, set the name
    if (m_deviceName == "")
    {
        // Worth noting that in this case the name will not match the actual device name and is
        // now only useful for scene level identification, OpenHaptics provides no mechanisms
        // for querying device names
        hdMakeCurrentDevice(m_handle);
        HDstring str = hdGetString(HD_DEVICE_SERIAL_NUMBER);
        m_deviceName = "Device_" + std::string(str);
    }

    // Enable forces
    hdEnable(HD_FORCE_OUTPUT);
    hdEnable(HD_FORCE_RAMPING);

    // Success
    LOG(INFO) << "\"" << this->getDeviceName() << "\" successfully initialized.";
}

void
HapticDeviceClient::update()
{
    hdScheduleSynchronous(hapticCallback, this, HD_MAX_SCHEDULER_PRIORITY);
}

void
HapticDeviceClient::disable()
{
    hdDisableDevice(m_handle);
}

HDCallbackCode HDCALLBACK
HapticDeviceClient::hapticCallback(void* pData)
{
    auto    client = static_cast<HapticDeviceClient*>(pData);
    HHD     handle = client->m_handle;
    HDstate state  = client->m_state;

    if (handle == HD_BAD_HANDLE || handle == HD_INVALID_HANDLE)
    {
        return HD_CALLBACK_DONE;
    }

    hdBeginFrame(handle);

    hdMakeCurrentDevice(handle);
    hdSetDoublev(HD_CURRENT_FORCE, client->m_force.data());
    hdGetDoublev(HD_CURRENT_POSITION, state.pos);
    hdGetDoublev(HD_CURRENT_VELOCITY, state.vel);
    hdGetDoublev(HD_CURRENT_TRANSFORM, state.trans);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &state.buttons);

    hdEndFrame(handle);

    client->m_position << state.pos[0], state.pos[1], state.pos[2];
    client->m_velocity << state.vel[0], state.vel[1], state.vel[2];
    client->m_orientation = (Eigen::Affine3d(Eigen::Matrix4d(state.trans))).rotation();
    
    for (int i = 0; i < 4; i++)
    {
        // If button down and not previously down
        if ((state.buttons & (1 << i)) && !client->m_buttons[i])
        {
            client->m_buttons[i] = true;
            client->postEvent(ButtonEvent(i, BUTTON_PRESSED));
        }
        // If button not down, and previously down
        else if (!(state.buttons & (1 << i)) && client->m_buttons[i])
        {
            client->m_buttons[i] = false;
            client->postEvent(ButtonEvent(i, BUTTON_RELEASED));
        }
    }

    client->m_trackingEnabled = true;

    return HD_CALLBACK_DONE;
}
} // imstk
