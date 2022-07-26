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

#include "imstkModule.h"

#include <vector>

namespace imstk
{
class HaplyDeviceClient;

///
/// \class HaplyDeviceManager
///
/// \brief Devices manager using Haply, only supports Inverse3 right now
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