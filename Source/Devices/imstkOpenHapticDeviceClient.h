/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <array>

namespace imstk
{
///
/// \class OpenHapticDeviceClient
///
/// \brief Subclass of DeviceClient for phantom omni
/// Holds and updates the data sync or on its own thread
/// Holder of data
///
class OpenHapticDeviceClient : public DeviceClient
{
friend class OpenHapticDeviceManager;

public:
    ~OpenHapticDeviceClient() override = default;

    ///
    /// \brief Use callback to get tracking data from phantom omni
    ///
    void update() override;

    ///
    /// \brief Constructor/Destructor, only the DeviceManager can construct
    /// \param Device name or use empty string for default device
    ///
    OpenHapticDeviceClient(const std::string& name = "") : DeviceClient(name, "localhost")
    {
        m_buttons = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 } };
    }

protected:

    ///
    /// \brief Initialize the phantom omni device
    ///
    void initialize();

    ///
    /// \brief Disables the phantom omni device
    ///
    void disable();

private:

    std::vector<std::pair<int, int>> m_events;
};
} // namespace imstk
