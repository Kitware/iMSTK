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

#include "imstkVRPNDeviceClient.h"

#include "g3log/g3log.hpp"

namespace imstk {

void
VRPNDeviceClient::initModule()
{
    std::string fullDeviceUrl = this->getName() + "@" + this->getUrl();

    m_vrpnTracker = std::make_shared<vrpn_Tracker_Remote>(fullDeviceUrl.c_str());
    m_vrpnAnalog = std::make_shared<vrpn_Analog_Remote>(fullDeviceUrl.c_str());
    m_vrpnButton = std::make_shared<vrpn_Button_Remote>(fullDeviceUrl.c_str());
    m_vrpnForceDevice = std::make_shared<vrpn_ForceDevice_Remote>(fullDeviceUrl.c_str());

    m_vrpnTracker->register_change_handler(this, trackerChangeHandler);
    m_vrpnTracker->register_change_handler(this, velocityChangeHandler);
    m_vrpnAnalog->register_change_handler(this, analogChangeHandler);
    m_vrpnButton->register_change_handler(this, buttonChangeHandler);
    m_vrpnForceDevice->register_force_change_handler(this, forceChangeHandler);
}

void
VRPNDeviceClient::runModule()
{
    if (this->getTrackingEnabled())
    {
        m_vrpnTracker->mainloop();
    }
    if (this->getAnalogicEnabled())
    {
        m_vrpnAnalog->mainloop();
    }
    if (this->getButtonsEnabled())
    {
        m_vrpnButton->mainloop();
    }
    if (this->getForceEnabled())
    {
        m_vrpnForceDevice->mainloop();
    }
}

void
VRPNDeviceClient::cleanUpModule()
{
    m_vrpnTracker->unregister_change_handler(this, trackerChangeHandler);
    m_vrpnTracker->unregister_change_handler(this, velocityChangeHandler);
    m_vrpnAnalog->unregister_change_handler(this, analogChangeHandler);
    m_vrpnButton->unregister_change_handler(this, buttonChangeHandler);
    m_vrpnForceDevice->unregister_force_change_handler(this, forceChangeHandler);

    m_vrpnTracker.reset();
    m_vrpnAnalog.reset();
    m_vrpnButton.reset();
    m_vrpnForceDevice.reset();
}

void VRPN_CALLBACK
VRPNDeviceClient::trackerChangeHandler(void *userData, const _vrpn_TRACKERCB t)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_position << t.pos[0], t.pos[1], t.pos[2];
    deviceClient->m_orientation.x() = t.quat[0];
    deviceClient->m_orientation.y() = t.quat[1];
    deviceClient->m_orientation.z() = t.quat[2];
    deviceClient->m_orientation.w() = t.quat[3];

    LOG(DEBUG) << "tracker: position = " << deviceClient->m_position;
    LOG(DEBUG) << "tracker: orientation = " << deviceClient->m_orientation.matrix();
}

void VRPN_CALLBACK
VRPNDeviceClient::analogChangeHandler(void *userData, const _vrpn_ANALOGCB a)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);

    if (a.num_channel > 0)
    {
        deviceClient->m_position << a.channel[0], a.channel[1], a.channel[2];
        LOG(DEBUG) << "analog: position = " << deviceClient->m_position;
    }
    if (a.num_channel > 3)
    {
        deviceClient->m_orientation =
                Eigen::AngleAxisd(a.channel[3]*M_PI,Vec3d::UnitX())*
                Eigen::AngleAxisd(a.channel[4]*M_PI,Vec3d::UnitY())*
                Eigen::AngleAxisd(a.channel[5]*M_PI,Vec3d::UnitZ());
        LOG(DEBUG) << "analog: orientation = " << deviceClient->m_orientation.matrix();
    }
}

void VRPN_CALLBACK
VRPNDeviceClient::velocityChangeHandler(void *userData, const _vrpn_TRACKERVELCB v)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_velocity << v.vel[0], v.vel[1], v.vel[2];
    LOG(DEBUG) << "tracker: velocity = " << deviceClient->m_velocity;
}

void VRPN_CALLBACK
VRPNDeviceClient::buttonChangeHandler(void *userData, const _vrpn_BUTTONCB b)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_buttons[b.button] = (b.state == 1);
    LOG(DEBUG) << "buttons: " << b.button << " = " << deviceClient->m_buttons[b.button];
}

void VRPN_CALLBACK
VRPNDeviceClient::forceChangeHandler(void *userData, const _vrpn_FORCECB f)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_force << f.force[0], f.force[1], f.force[2];
    LOG(DEBUG) << "forceDevice: force = " << deviceClient->m_force;
}
}
