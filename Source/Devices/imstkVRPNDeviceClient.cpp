/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVRPNDeviceClient.h"

#include "quat.h"

#include "imstkLogger.h"

namespace imstk
{
VRPNDeviceClient::VRPNDeviceClient(const std::string& deviceName, VRPNDeviceType type, const std::string& ip /*= "localhost"*/) :
    DeviceClient(deviceName, ip), m_type(type)
{
    m_trackingEnabled = (type & VRPNTracker) != 0;
    m_buttonsEnabled  = (type & VRPNButton) != 0;
    m_analogicEnabled = (type & VRPNAnalog) != 0;
    m_forceEnabled    = (type & VRPNForce) != 0;
}

void VRPN_CALLBACK
VRPNDeviceClient::trackerPositionChangeHandler(void* userData, const _vrpn_TRACKERCB t)
{
    auto deviceClient = static_cast<VRPNDeviceClient*>(userData);

    Quatd quat;
    quat.x() = t.quat[1];
    quat.y() = t.quat[2];
    quat.z() = t.quat[3];
    quat.w() = t.quat[0];

    deviceClient->m_transformLock.lock();
    deviceClient->m_position << t.pos[0], t.pos[1], t.pos[2];
    deviceClient->m_orientation = quat;
    deviceClient->m_transformLock.unlock();
}

void VRPN_CALLBACK
VRPNDeviceClient::analogChangeHandler(void* userData, const _vrpn_ANALOGCB a)
{
    auto deviceClient = static_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_dataLock.lock();
    deviceClient->m_analogChannels.resize(a.num_channel);
    for (int i = 0; i < a.num_channel; i++)
    {
        deviceClient->m_analogChannels[i] = a.channel[i];
    }
    deviceClient->m_dataLock.unlock();
}

void VRPN_CALLBACK
VRPNDeviceClient::trackerVelocityChangeHandler(void* userData, const _vrpn_TRACKERVELCB v)
{
    auto  deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    Quatd quat(v.vel_quat[1], v.vel_quat[2], v.vel_quat[3], v.vel_quat[0]);

    deviceClient->m_transformLock.lock();
    deviceClient->m_velocity << v.vel[0], v.vel[1], v.vel[2];
    // \todo translate velocity quaternion to imstk
    // deviceClient->m_angularVelocity = quat;
    //
    deviceClient->m_transformLock.unlock();
}

void VRPN_CALLBACK
VRPNDeviceClient::buttonChangeHandler(void* userData, const _vrpn_BUTTONCB b)
{
    auto deviceClient = reinterpret_cast<VRPNDeviceClient*>(userData);
    deviceClient->m_buttons[b.button] = (b.state == 1);
}

imstk::VRPNDeviceType
VRPNDeviceClient::getType() const
{
    return m_type;
}
} // imstk
