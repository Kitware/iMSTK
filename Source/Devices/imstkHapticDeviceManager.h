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

#include "imstkModule.h"

#include <vector>

namespace imstk
{
class HapticDeviceClient;

///
/// \class HDAPIDeviceServer
/// \brief Devices manager using HDAPI
/// \todo add the frame rate option for the servo loop
///
class HapticDeviceManager : public Module
{
public:
    HapticDeviceManager() : Module()
    {
        muteUpdateEvents = true;
    }

    ///
    /// \brief Destructor
    ///
    virtual ~HapticDeviceManager() override = default;

    ///
    /// \brief Create a haptic device client and add it to the internal list
    /// \param Device name or use empty string for default device
    ///
    std::shared_ptr<HapticDeviceClient> makeDeviceClient(std::string name = "");

protected:
    ///
    /// \brief
    ///
    bool initModule() override;

    void updateModule() override { }

    ///
    /// \brief
    ///
    void uninitModule() override;

private:
    friend HapticDeviceClient;

    std::vector<std::shared_ptr<HapticDeviceClient>> m_deviceClients; ///< list of all the device clients
};
} // imstk
