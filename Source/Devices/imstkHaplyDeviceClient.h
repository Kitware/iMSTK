/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <Handle.h>
#include <Inverse3.h>
#include <UUID.h>

namespace Haply
{
namespace HardwareAPI
{
namespace Devices
{
class Handle;
class Inverse3;
}
namespace IO { class SerialStream; }
} // namespace HardwareAPI
} // namespace Haply

namespace imstk
{
///
/// \class HaplyDeviceClient
///
/// \brief Subclass of DeviceClient for haply, currently implemented only for the Inverse3.
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
    Haply::HardwareAPI::UUID quat;
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
    HaplyDeviceClient(const std::string& name = "", const std::string& handleName = "") :
        DeviceClient(name, "localhost"), m_handleName(handleName) { }

    ///
    /// \brief Initialize the device
    ///
    void initialize();

    ///
    /// \brief Disables the device
    ///
    void disable();

private:
    ///
    /// \brief Haply requires a subclass to get orientation data from a handle via an overridden function.
    /// 
    class HaplyHandle : public Haply::HardwareAPI::Devices::Handle
    {
    public:
        HaplyHandle(std::iostream* stream) : Handle(stream) { }

    protected:
        void OnReceiveHandleInfo(HandleInfoResponse& response) override { m_infoResponse = response; }
        void OnReceiveHandleStatusMessage(HandleStatusResponse& response) override { m_statusResponse = response; }
        void OnReceiveHandleErrorResponse(HandleErrorResponse& response) override { m_errorResponse = response; }

    public:
        Haply::HardwareAPI::Devices::Handle::HandleInfoResponse m_infoResponse;
        Haply::HardwareAPI::Devices::Handle::HandleStatusResponse m_statusResponse;
        //Haply::HardwareAPI::Devices::Handle::HandleStatusResponse statusResponseCalibrated;
        Haply::HardwareAPI::Devices::Handle::HandleErrorResponse m_errorResponse;
    };

private:
    std::shared_ptr<Haply::HardwareAPI::Devices::Inverse3> m_device;
    std::shared_ptr<Haply::HardwareAPI::IO::SerialStream>  m_deviceStream;

    std::string m_handleName = "";
    bool m_handleEnabled = false;
    unsigned char m_lastReturnType;
    std::shared_ptr<HaplyHandle> m_handleDevice;
    std::shared_ptr<Haply::HardwareAPI::IO::SerialStream> m_handleDeviceStream;
    Haply::HardwareAPI::Devices::Inverse3::EndEffectorStateResponse m_deviceResponse;

    Vec3f      m_devicePos      = Vec3f::Zero();
    Vec3f      m_deviceVelocity = Vec3f::Zero();
    Vec3f      m_deviceForce    = Vec3f::Zero();
    DeviceInfo m_deviceInfo;
};
} // namespace imstk
