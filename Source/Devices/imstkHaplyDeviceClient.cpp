/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkHaplyDeviceClient.h"
#include "imstkLogger.h"

#include <SerialStream.h>

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
    return os;
}

void
HaplyDeviceClient::initialize()
{
    m_deviceStream = std::make_shared<IO::SerialStream>(m_deviceName.c_str(), true);
    m_device      = std::make_shared<Devices::Inverse3>(m_deviceStream.get());

    if (m_handleName != "")
    {
        m_handleEnabled = true;
        m_handleDeviceStream = std::make_shared<IO::SerialStream>(m_handleName.c_str(), true);
        m_handleDevice = std::make_shared<HaplyHandle>(m_handleDeviceStream.get());
    }

    m_device->SendDeviceWakeup();

    m_device->ReceiveDeviceInfo(&m_deviceInfo.deviceId,
        &m_deviceInfo.modelNumber,
        &m_deviceInfo.hardwareVersion, &m_deviceInfo.firmwareVersion,
        &m_deviceInfo.quat);

    // Wake up the handle. If the handle is awake, the first response may not be correct response type, so we wait until
    if (m_handleEnabled)
    {
        m_handleDevice->SendDeviceWakeup();
        while (m_lastReturnType != 0xD0)
        {
            m_handleDevice->Receive(&m_lastReturnType);
        }
        //Haply::HardwareAPI::Devices::Handle::HandleInfoResponse handleInfo = m_handleDevice->GetInfoResponse();
        // Calibrate the handle. Make sure to point the handle toward the screen
        //handle->Calibrate();
    }

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

    //m_deviceResponse = m_device->EndEffectorForce({ m_deviceForce[0], m_deviceForce[1], m_deviceForce[2] });

    // Get handle update
    if (m_handleEnabled)
    {
        m_handleDevice->Receive(&m_lastReturnType);
    }

    // Swap the axes a bit (Haply uses a RHS z-up)
    m_transformLock.lock();
    m_position = Vec3d(
        static_cast<double>(m_devicePos[1]),
        static_cast<double>(m_devicePos[2]),
        static_cast<double>(m_devicePos[0]));
    if (m_handleEnabled)
    {
        m_orientation = Quatd(
            static_cast<double>(m_handleDevice->m_statusResponse.quaternion[0]),
            static_cast<double>(m_handleDevice->m_statusResponse.quaternion[1]),
            static_cast<double>(m_handleDevice->m_statusResponse.quaternion[2]),
            static_cast<double>(m_handleDevice->m_statusResponse.quaternion[3]));
    }
    m_transformLock.unlock();
}

void
HaplyDeviceClient::disable()
{
    LOG(INFO) << "Closing Haply device streams";
    m_deviceStream->CloseDevice();
    if (m_handleEnabled)
    {
        m_handleDeviceStream->CloseDevice();
    }
}
} // namespace imstk