/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkHaplyDeviceClient.h"
#include "imstkLogger.h"

#include <SerialStream.h>
#include <Inverse3.h>

using namespace Haply::HardwareAPI;

namespace imstk
{
std::ostream&
operator<<(std::ostream& os, const HaplyDeviceClient::DeviceInfo& info)
{
    os << "Device id: " << info.deviceId << std::endl;
    os << "Device Model #: " << static_cast<int>(info.modelNumber) << std::endl;
    os << "Hardware Version: " << static_cast<int>(info.hardwareVersion) << std::endl;
    os << "Firmware Version: " << static_cast<int>(info.firmwareVersion) << std::endl;
    os << "Quat: " << info.quat[0] << ", " << info.quat[1] << ", " << info.quat[2] << ", " << info.quat[3] << std::endl;
    return os;
}

void
HaplyDeviceClient::initialize()
{
    m_inputStream = std::make_shared<IO::SerialStream>(m_deviceName.c_str(), true);
    m_device      = std::make_shared<Devices::Inverse3>(m_inputStream.get());

    m_device->SendDeviceWakeup();

    m_device->ReceiveDeviceInfo(&m_deviceInfo.deviceId,
        &m_deviceInfo.modelNumber,
        &m_deviceInfo.hardwareVersion, &m_deviceInfo.firmwareVersion,
        m_deviceInfo.quat);

    LOG(INFO) << m_deviceInfo;
}

void
HaplyDeviceClient::update()
{
    m_forceLock.lock();
    m_deviceForce = m_force.cast<float>();
    m_deviceForce = Vec3f(
        static_cast<float>(m_force[2]),
        static_cast<float>(m_force[0]),
        static_cast<float>(m_force[1]));
    m_forceLock.unlock();

    m_device->SendEndEffectorForce(m_deviceForce.data());
    m_device->ReceiveEndEffectorState(m_devicePos.data(), m_deviceVelocity.data());

    // Swap the axes a bit (Haply uses a RHS z-up)
    m_transformLock.lock();
    m_position = Vec3d(
        static_cast<double>(m_devicePos[1]),
        static_cast<double>(m_devicePos[2]),
        static_cast<double>(m_devicePos[0]));
    m_transformLock.unlock();
}

void
HaplyDeviceClient::disable()
{
    LOG(INFO) << "Closing Haply device stream";
    m_inputStream->CloseDevice();
}
} // namespace imstk