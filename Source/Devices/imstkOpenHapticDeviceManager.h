/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceManager.h"
#include "imstkMacros.h"

#include <vector>
#include <memory>

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
        OpenHapticDeviceManager();

        ~OpenHapticDeviceManager() override = default;

        IMSTK_TYPE_NAME(OpenHapticDeviceManager)

            ///
            /// \brief Create a haptic device client and add it to the internal list
            /// \param Device name or use empty string for default device
            ///
            std::shared_ptr<DeviceClient> makeDeviceClient(std::string name = "") override;

    protected:

        bool initModule() override;

        void updateModule() override;

        void uninitModule() override;

    private:
        class OpenHapticDeviceManagerImpl;
        std::unique_ptr<OpenHapticDeviceManagerImpl> m_impl;
    };
} // namespace imstk