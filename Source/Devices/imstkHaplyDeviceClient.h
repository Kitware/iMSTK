/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

namespace Haply
{
namespace HardwareAPI
{
namespace Devices { class Inverse3; }
namespace IO { class SerialStream; }
} // namespace HardwareAPI
} // namespace Haply

namespace imstk
{
///
/// \class HaplyDeviceClient
///
/// \brief Subclass of DeviceClient for haply, currently implemented only for the Inverse3
/// Warning: This code is based off an early version of the Haply Hardware API.
///
class HaplyDeviceClient : public DeviceClient
{
struct DeviceInfo
{
    uint16_t deviceId = -1;
    unsigned char modelNumber     = -1;
    unsigned char hardwareVersion = -1;
    unsigned char firmwareVersion = -1;
    float quat[4] = { -1.0, -1.0, -1.0, -1.0 };
};

friend std::ostream& operator<<(std::ostream& os, const DeviceInfo& info);

friend class HaplyDeviceManager;

public:
    ~HaplyDeviceClient() override = default;

    ///
    /// \brief Use callback to get tracking data from phantom omni
    ///
    void update() override;

protected:
    ///
    /// \brief Constructor/Destructor, only the DeviceManager can construct
    /// \param Device name or use empty string for default device
    ///
    HaplyDeviceClient(const std::string& name = "") : DeviceClient(name, "localhost") { }

    ///
    /// \brief Initialize the device
    ///
    void initialize();

    ///
    /// \brief Disables the device
    ///
    void disable();

private:
    std::shared_ptr<Haply::HardwareAPI::Devices::Inverse3> m_device;
    std::shared_ptr<Haply::HardwareAPI::IO::SerialStream>  m_inputStream;

    Vec3f      m_devicePos      = Vec3f::Zero();
    Vec3f      m_deviceVelocity = Vec3f::Zero();
    Vec3f      m_deviceForce    = Vec3f::Zero();
    DeviceInfo m_deviceInfo;
};
} // namespace imstk
