/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceManager.h"
#include "imstkMacros.h"

#include <vector>

namespace imstk
{
class HaplyDeviceClient;

///
/// \class HaplyDeviceManager
///
/// \brief Devices manager using Haply, only supports Inverse3 right now
/// Warning: This code is based off an early version of the Haply Hardware API.
///
class HaplyDeviceManager : public DeviceManager
{
public:
    HaplyDeviceManager() = default;
    ~HaplyDeviceManager() override = default;

    IMSTK_TYPE_NAME(HaplyDeviceManager)

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param portName COM port name of device
    ///
    std::shared_ptr<DeviceClient> makeDeviceClient(std::string portName = "") override { return makeDeviceClient(portName, ""); }

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param portName COM port name of device
    /// \param handlePortName Bluetooth port name for the device handle
    ///
    std::shared_ptr<DeviceClient> makeDeviceClient(std::string portName, std::string handlePortName);

    ///
    /// \brief Autodetect and get all Inverse3 device port names
    ///
    static std::vector<std::string> getInverse3PortNames();

    ///
    /// \brief Autodetect and get all the Haply handle port names
    /// 
    static std::vector<std::string> getHandlePortNames();

    static bool isDevicePresent();

protected:
    bool initModule() override;

    void updateModule() override;

    void uninitModule() override;

private:
    friend HaplyDeviceClient;

    std::vector<std::shared_ptr<HaplyDeviceClient>> m_deviceClients; ///< list of all the device clients
};
} // namespace imstk