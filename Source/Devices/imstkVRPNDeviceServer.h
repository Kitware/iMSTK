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

#include <map>
#include <string>

#include <vrpn_Connection.h>
#include <vrpn_MainloopContainer.h>

//VRPN
#include "imstkDeviceClient.h"
#include "quat.h"

namespace imstk
{
class VRPNDeviceClient;

using VRPNDeviceType = int;
const VRPNDeviceType VRPNAnalog  = 0x1;
const VRPNDeviceType VRPNButton  = 0x2;
const VRPNDeviceType VRPNTracker = 0x4;
const VRPNDeviceType VRPNForce   = 0x8;

///
/// \class VRPNDeviceServer
/// \brief Devices server using VRPN
///
class VRPNDeviceServer : public Module
{
public:

    ///
    /// \brief Constructor
    ///
    VRPNDeviceServer(const std::string& machine = "localhost", int port = vrpn_DEFAULT_LISTEN_PORT_NO);

    ///
    /// \brief Destructor
    ///
    virtual ~VRPNDeviceServer() override = default;

    ///
    /// \brief Add device
    ///
    void addDevice(const std::string& deviceName, DeviceType deviceType, int id = 0);

    ///
    /// \brief Add serial device
    ///
    void addSerialDevice(const std::string& deviceName, DeviceType deviceType, const std::string& port = "COM6", int baudRate = 57600, int id = 0);

    ///
    /// \bried Add device client
    ///
    void addDeviceClient(std::shared_ptr<VRPNDeviceClient> client);

    std::shared_ptr<DeviceClient> getClient(const std::string& deviceName, VRPNDeviceType deviceType);

protected:
    ///
    /// \brief Initialize the server module
    ///
    bool initModule() override;

    ///
    /// \brief Run the server module
    ///
    void updateModule() override;

    ///
    /// \brief Clean the server module
    ///
    void uninitModule() override;

private:

    ///
    /// \struct
    /// \brief TODO
    ///
    struct SerialInfo
    {
        int baudRate;
        std::string port;
    };

    const std::string m_machine;                                       ///< machine name or IP
    const int m_port;                                                  ///< connection port

    struct Client
    {
        std::shared_ptr<VRPNDeviceClient> client;
        VRPNDeviceType type;
        int id;
    };

    std::map<std::string, Client>           m_deviceInfoMap;     ///< list of iMSTK client info
    std::map<std::string, SerialInfo>       m_SerialInfoMap;
    std::unique_ptr<vrpn_MainloopContainer> m_deviceConnections; ///< VRPN device connections
};
} // imstk
