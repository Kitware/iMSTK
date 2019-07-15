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

#ifdef iMSTK_USE_OPENHAPTICS

#include "imstkHDAPIDeviceClient.h"

#include <HDU/hduVector.h>
#include <HDU/hduError.h>

#include "g3log/g3log.hpp"

namespace imstk
{
void
HDAPIDeviceClient::init()
{
    m_buttons = std::map<size_t, bool>{ { 0, false }, { 1, false }, { 2, false }, { 3, false } }

    //flush error stack
    HDErrorInfo errorFlush;
    while (HD_DEVICE_ERROR(errorFlush = hdGetError())) {}

    // Open Device
    m_handle = hdInitDevice(this->getDeviceName().c_str());

    // If failed
    HDErrorInfo error;
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        LOG(FATAL) << "Failed to initialize Phantom Omni " << this->getDeviceName();
        m_handle = -1;
        return;
    }

    // Enable forces
    hdEnable(HD_FORCE_OUTPUT);
    hdEnable(HD_FORCE_RAMPING);

    // Success
    LOG(INFO) << this->getDeviceName() << " successfully initialized.";
}

void
HDAPIDeviceClient::run()
{
    hdScheduleSynchronous(hapticCallback, this, HD_MAX_SCHEDULER_PRIORITY);
}

void
HDAPIDeviceClient::cleanUp()
{
    hdDisableDevice(m_handle);
}

HDCallbackCode HDCALLBACK
HDAPIDeviceClient::hapticCallback(void* pData)
{
    auto client = reinterpret_cast<HDAPIDeviceClient*>(pData);
    auto handle = client->m_handle;
    auto state  = client->m_state;

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
    client->m_buttons[0]  = state.buttons & HD_DEVICE_BUTTON_1;
    client->m_buttons[1]  = state.buttons & HD_DEVICE_BUTTON_2;
    client->m_buttons[2]  = state.buttons & HD_DEVICE_BUTTON_3;
    client->m_buttons[3]  = state.buttons & HD_DEVICE_BUTTON_4;

    return HD_CALLBACK_DONE;
}
} // imstk
#endif // ifdef iMSTK_USE_OPENHAPTICS
