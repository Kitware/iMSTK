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

#include "imstkDeviceClient.h"

#include "g3log/g3log.hpp"

namespace imstk {

const std::string&
DeviceClient::getIp()
{
    return m_ip;
}

void
DeviceClient::setIp(const std::string& ip)
{
    m_ip = ip;
}

const bool&
DeviceClient::getTrackingEnabled() const
{
    return m_trackingEnabled;
}

void
DeviceClient::setTrackingEnabled(const bool& status)
{
    if (this->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "DeviceClient::setTrackingEnabled error: can not change listeners for"
                     << this->getName() << " while the device is active.";
        return;
    }
    m_trackingEnabled = status;
}

const bool&
DeviceClient::getAnalogicEnabled() const
{
    return m_analogicEnabled;
}

void
DeviceClient::setAnalogicEnabled(const bool& status)
{
    if (this->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "DeviceClient::setAnalogicEnabled error: can not change listeners for"
                     << this->getName() << " while the device is active.";
        return;
    }
    m_analogicEnabled = status;
}

const bool&
DeviceClient::getButtonsEnabled() const
{
    return m_buttonsEnabled;
}

void
DeviceClient::setButtonsEnabled(const bool& status)
{
    if (this->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "DeviceClient::setButtonsEnabled error: can not change listeners for"
                     << this->getName() << " while the device is active.";
        return;
    }
    m_buttonsEnabled = status;
}

const bool&
DeviceClient::getForceEnabled() const
{
    return m_forceEnabled;
}

void
DeviceClient::setForceEnabled(const bool& status)
{
    if (this->getStatus() != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "DeviceClient::setForceEnabled error: can not change listeners for"
                     << this->getName() << " while the device is active.";
        return;
    }
    m_forceEnabled = status;
}

const Vec3d&
DeviceClient::getPosition() const
{
    return m_position;
}

const Vec3d&
DeviceClient::getVelocity() const
{
    return m_velocity;
}

const Quatd&
DeviceClient::getOrientation() const
{
    return m_orientation;
}

const std::map<size_t, bool>&
DeviceClient::getButtons() const
{
    return m_buttons;
}

bool
DeviceClient::getButton(size_t buttonId) const
{
    if (m_buttons.find(buttonId) == m_buttons.end())
    {
        LOG(WARNING) << "DeviceClient::getButton warning: button "
                     << buttonId << " was not found in the buttons list.";
        return false;
    }
    return m_buttons.at(buttonId);
}

const Vec3d&
DeviceClient::getForce() const
{
    return m_force;
}

const Vec3d&
DeviceClient::getTorque() const
{
    return m_torque;
}

}
