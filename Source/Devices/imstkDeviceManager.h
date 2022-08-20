/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkModule.h"
#include "imstkMacros.h"

#include <vector>

namespace imstk
{
class DeviceClient;

///
/// \class DeviceManager
///
/// \brief Abstract class for DeviceManagers which must return a DeviceClient
/// for which it manages
///
class DeviceManager : public Module
{
protected:
    DeviceManager()
    {
        // Throwing events at high rates is a bad idea for this thread
        m_muteUpdateEvents = true;
        // Run in another thread
        m_executionType = ExecutionType::PARALLEL;
    }

public:
    ~DeviceManager() override = default;

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param Port name
    ///
    virtual std::shared_ptr<DeviceClient> makeDeviceClient(std::string portName = "") = 0;
};
} // namespace imstk