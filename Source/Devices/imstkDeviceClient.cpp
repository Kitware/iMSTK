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

const Vec3d
DeviceClient::getPosition()
{
    Vec3d pos;
    m_transformLock.lock();
    pos = m_position;
    m_transformLock.unlock();
    return pos;
}

const Vec3d
DeviceClient::getVelocity()
{
    Vec3d vel;
    m_transformLock.lock();
    vel = m_velocity;
    m_transformLock.unlock();
    return vel;
}

const Vec3d
DeviceClient::getAngularVelocity()
{
    Vec3d angVel;
    m_transformLock.lock();
    angVel = m_angularVelocity;
    m_transformLock.unlock();
    return angVel;
}

const Quatd
DeviceClient::getOrientation()
{
    Quatd orientation;
    m_transformLock.lock();
    orientation = m_orientation;
    m_transformLock.unlock();
    return orientation;
}

const Vec3d
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

const std::vector<double>
DeviceClient::getAnalog() const
{
    std::vector<double> result;
    m_dataLock.lock();
    result = m_analogChannels;
    m_dataLock.unlock();
    return result;
}
} // imstk
