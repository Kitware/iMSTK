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
#include "imstkLogger.h"
#include <limits>

namespace imstk
{
DeviceClient::DeviceClient(const std::string& name, const std::string& ip) :
    m_deviceName(name),
    m_ip(ip),
    m_position(Vec3d::Zero()),
    m_velocity(Vec3d::Zero()),
    m_orientation(Quatd::Identity()),
    m_force(Vec3d::Zero())
{
}

Vec3d
DeviceClient::getPosition()
{
    Vec3d pos;
    m_transformLock.lock();
    pos = m_position;
    m_transformLock.unlock();
    return pos;
}

Vec3d
DeviceClient::getVelocity()
{
    Vec3d vel;
    m_transformLock.lock();
    vel = m_velocity;
    m_transformLock.unlock();
    return vel;
}

Vec3d
DeviceClient::getAngularVelocity()
{
    Vec3d angVel;
    m_transformLock.lock();
    angVel = m_angularVelocity;
    m_transformLock.unlock();
    return angVel;
}

Quatd
DeviceClient::getOrientation()
{
    Quatd orientation;
    m_transformLock.lock();
    orientation = m_orientation;
    m_transformLock.unlock();
    return orientation;
}

Vec3d
DeviceClient::getForce()
{
    Vec3d force;
    m_transformLock.lock();
    force = m_force;
    m_transformLock.unlock();
    return force;
}

void
DeviceClient::setForce(Vec3d force)
{
    m_forceLock.lock();
    m_force = force;
    m_forceLock.unlock();
}

const std::unordered_map<int, int>&
DeviceClient::getButtons() const
{
    return m_buttons;
}

int
DeviceClient::getButton(const int buttonId)
{
    int result = 0;
    m_dataLock.lock();
    if (m_buttons.find(buttonId) != m_buttons.end())
    {
        result = m_buttons.at(buttonId);
    }
    m_dataLock.unlock();
    return result;
}

std::vector<double>
DeviceClient::getAnalog() const
{
    std::vector<double> result;
    m_dataLock.lock();
    result = m_analogChannels;
    m_dataLock.unlock();
    return result;
}

double
DeviceClient::getAnalog(int i) const
{
    if (i > m_analogChannels.size())
    {
        LOG(WARNING) << "Requested unknown channel, returning NAN";
        return std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
        double result;
        m_dataLock.lock();
        result = m_analogChannels[i];
        m_dataLock.unlock();
        return result;
    }
}
} // namespace imstk