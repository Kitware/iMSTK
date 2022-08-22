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
class OpenHapticDeviceClient;

///
/// \class OpenHapticDeviceManager
///
/// \brief Devices manager using HDAPI
/// \todo add the frame rate option for the servo loop
///
class OpenHapticDeviceManager : public DeviceManager
{
public:
    OpenHapticDeviceManager()
    {
        // Default a 1ms sleep to avoid over consumption of the CPU
        m_sleepDelay = 1.0;
    }

    ~OpenHapticDeviceManager() override = default;

    IMSTK_TYPE_NAME(OpenHapticDeviceManager)

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param Device name or use empty string for default device
    ///
    std::shared_ptr<DeviceClient> makeDeviceClient(std::string name = "") override;

protected:
    ///
    /// \brief
    ///
    bool initModule() override;

    void updateModule() override;

    ///
    /// \brief
    ///
    void uninitModule() override;

private:
    friend OpenHapticDeviceClient;

    std::vector<std::shared_ptr<OpenHapticDeviceClient>> m_deviceClients; ///< list of all the device clients
};
} // namespace imstk