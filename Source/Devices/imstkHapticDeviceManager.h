/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

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

#include <vector>

// imstk
#include "imstkThreadObject.h"

namespace imstk
{
class HapticDeviceClient;
///
/// \class HDAPIDeviceServer
/// \brief Devices manager using HDAPI
/// \todo add the frame rate option for the servo loop
///
class HapticDeviceManager : public ThreadObject
{
friend HapticDeviceClient;

public:

    HapticDeviceManager() : ThreadObject("HapticDeviceManager") {}

    ///
    /// \brief Destructor
    /// Stop the scheduler
    ///
    virtual ~HapticDeviceManager() override = default;

    ///
    /// \brief Create a haptic device client and add it to the internal list
    ///
    std::shared_ptr<HapticDeviceClient> makeDeviceClient(const std::string& name);

    ///
    /// \brief Initialize the client devices and start the scheduler
    ///
    void initialize();

protected:
    ///
    /// \brief
    ///
    void startThread() override;

    ///
    /// \brief
    ///
    void stopThread() override;

private:

    std::vector<std::shared_ptr<HapticDeviceClient>> m_deviceClients; ///< list of all the device clients
};
} // imstk
