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
/// \class VRPNDeviceManager
///
/// \brief connects to the vrpn.exe server and lets iMSTK attach devices to the server
///
/// This module enables communication with VRPN and lets us map VRPN devices to the
/// imstk DeviceClient interface. The VRPN server is external to iMSTK and needs to
/// be run from the command line `vrpn_server`, when using the default vrpn.cfg file needs to be
/// edited represent the projects requirements.
/// For more information on VRPN see the wiki https://github.com/vrpn/vrpn/wiki
///
class VRPNDeviceManager : public Module
{
public:

    ///
    /// \brief Constructor
    /// \param machine The ip address of the machine where the vrpn server is running, "localhost" and "loopback"
    ///                are also options
    /// \param port The port that the vrpn server is listening to (can set on the command line)
    ///
    VRPNDeviceManager(const std::string& machine = "localhost", int port = vrpn_DEFAULT_LISTEN_PORT_NO);

    virtual ~VRPNDeviceManager() override = default;

    void addDeviceClient(std::shared_ptr<VRPNDeviceClient> client);

    ///
    /// \brief Creates a client from the given parameters
    /// \param deviceName The name of the device that you want, it must match (case-sensitive) with the
    ///                   name in the selected `vrpn.cfg` file
    /// \param deviceType A binary combination of the parameters that should be updated. Note that this
    ///                   is a request, if the server doesn't supply the requested updates no changes
    ///                   will be observable
    ///
    std::shared_ptr<DeviceClient> makeDeviceClient(const std::string& deviceName, VRPNDeviceType deviceType);

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

    const std::string m_machine;                                       ///< machine name or IP
    const int m_port;                                                  ///< connection port

    struct Client
    {
        std::shared_ptr<VRPNDeviceClient> client;
        VRPNDeviceType type;
    };

    std::map<std::string, Client> m_deviceInfoMap;               ///< list of iMSTK client info
    std::unique_ptr<vrpn_MainloopContainer> m_deviceConnections; ///< VRPN device connections
};
} // imstk
