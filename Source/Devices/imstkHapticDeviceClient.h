/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"

#include <HD/hd.h>

#include <array>

namespace imstk
{
struct HDstate
{
    // \todo pos are redundant?
    HDdouble pos[3];
    HDdouble vel[3];
    HDdouble angularVel[3];
    HDdouble transform[16];
    HDint buttons;
};

///
/// \class HapticDeviceClient
///
/// \brief Subclass of DeviceClient for phantom omni
/// Holds and updates the data sync or on its own thread
/// Holder of data
///
class HapticDeviceClient : public DeviceClient
{
friend class HapticDeviceManager;

public:
    ~HapticDeviceClient() override = default;

    ///
    /// \brief Use callback to get tracking data from phantom omni
    ///
    void update() override;

protected:
    ///
    /// \brief Constructor/Destructor, only the DeviceManager can construct
    /// \param Device name or use empty string for default device
    ///
    HapticDeviceClient(const std::string& name = "") : DeviceClient(name, "localhost") { }

    ///
    /// \brief Initialize the phantom omni device
    ///
    void initialize();

    ///
    /// \brief Disables the phantom omni device
    ///
    void disable();

private:
    typedef unsigned int HDCallbackCode;

    ///
    /// \brief Phantom omni device api callback
    ///
    static HDCallbackCode HDCALLBACK hapticCallback(void* pData);

    HHD     m_handle = HD_INVALID_HANDLE; ///< device handle
    HDstate m_state;                      ///< device reading state
    HDSchedulerHandle m_schedulerHandle = 0;
    std::vector<std::pair<int, int>> m_events;
};
} // namespace imstk
