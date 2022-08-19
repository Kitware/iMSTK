/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkModule.h"

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
class HaplyDeviceManager : public Module
{
public:
    HaplyDeviceManager() : Module()
    {
        m_muteUpdateEvents = true;
        m_executionType    = ExecutionType::PARALLEL;
    }

    ~HaplyDeviceManager() override = default;

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param COM port name
    ///
    std::shared_ptr<HaplyDeviceClient> makeDeviceClient(std::string portName = "");

    ///
    /// \brief Autodetect and get all Inverse3 device port names
    ///
    std::vector<std::string> getPortNames() const;

protected:
    bool initModule() override;

    void updateModule() override;

    void uninitModule() override;

private:
    friend HaplyDeviceClient;

    std::vector<std::shared_ptr<HaplyDeviceClient>> m_deviceClients; ///< list of all the device clients
};
} // namespace imstk