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
    HDErrorInfo error = hdGetError();
    LOG_IF(FATAL, HD_DEVICE_ERROR(error)) <<
        "Failed to initialize Phantom Omni \"" << this->getDeviceName() + "\"" << "\n"
        "Error code: " << error.errorCode << "\n"
        "Internal Error code: " << error.internalErrorCode;

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

    m_schedulerHandle = hdScheduleAsynchronous(hapticCallback, this, HD_MAX_SCHEDULER_PRIORITY); // Call sometime later
    // Success
    LOG(INFO) << "\"" << this->getDeviceName() << "\" successfully initialized.";
}

void
HapticDeviceClient::update()
{
    std::vector<std::pair<int, int>> localEvents;
    m_dataLock.lock();
    std::swap(m_events, localEvents);
    m_dataLock.unlock();

    for (const auto& item : localEvents)
    {
        postEvent(ButtonEvent(HapticDeviceClient::buttonStateChanged(), item.first, item.second));
    }
}

void
HapticDeviceClient::disable()
{
    // HS 2021-oct-07 There is no documentation on whether hdUnschedule is synchronous
    // or asynchronous, but as all the examples set the sequence to shutdown an HD device
    // with hdStopScheduler, hdUnschedule, hdDisableDevice i'm assuming an hdWaitForCompletion
    // is unnecessary here
    hdUnschedule(m_schedulerHandle);
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

    client->m_forceLock.lock();
    const Vec3d force = client->m_force;
    client->m_forceLock.unlock();

    hdBeginFrame(handle);

    hdMakeCurrentDevice(handle);
    hdSetDoublev(HD_CURRENT_FORCE, force.data());
    hdGetDoublev(HD_CURRENT_POSITION, state.pos);
    hdGetDoublev(HD_CURRENT_VELOCITY, state.vel);
    hdGetDoublev(HD_CURRENT_ANGULAR_VELOCITY, state.angularVel);
    hdGetDoublev(HD_CURRENT_TRANSFORM, state.transform);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &state.buttons);

    hdEndFrame(handle);

    // Might be worth locking each part separately
    const Quatd orientation = Quatd((Eigen::Affine3d(Eigen::Matrix4d(state.transform))).rotation());
    client->m_transformLock.lock();
    client->m_position << state.pos[0], state.pos[1], state.pos[2];
    client->m_velocity << state.vel[0], state.vel[1], state.vel[2];
    client->m_angularVelocity << state.angularVel[0], state.angularVel[1], state.angularVel[2];
    client->m_orientation = orientation;
    client->m_transformLock.unlock();

    client->m_dataLock.lock();
    for (int i = 0; i < 4; i++)
    {
        // If button down and not previously down
        if ((state.buttons & (1 << i)) && !client->m_buttons[i])
        {
            client->m_buttons[i] = true;
            client->m_events.push_back({ i, BUTTON_PRESSED });
        }
        // If button not down, and previously down
        else if (!(state.buttons & (1 << i)) && client->m_buttons[i])
        {
            client->m_buttons[i] = false;
            client->m_events.push_back({ i, BUTTON_RELEASED });
        }
    }
    client->m_dataLock.unlock();

    client->m_trackingEnabled = true;

    return HD_CALLBACK_CONTINUE;
}
}