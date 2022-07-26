/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc.

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
