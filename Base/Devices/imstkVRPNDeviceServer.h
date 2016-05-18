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

#ifndef imstkVRPNDeviceServer_h
#define imstkVRPNDeviceServer_h

#include <vector>
#include <map>
#include <typeinfo>

#include <vrpn_Connection.h>
#include <vrpn_MainloopContainer.h>

#include "imstkModule.h"
#include "imstkVRPNDeviceClient.h"

namespace imstk {

enum class DeviceType
{
    SPACE_EXPLORER_3DCONNEXION,
    NAVIGATOR_3DCONNEXION,
    NOVINT_FALCON
};

///
/// \class VRPNDeviceServer
/// \brief Devices server using VRPN
///
class VRPNDeviceServer : public Module
{
public:

    VRPNDeviceServer(std::string machine = "localhost", int port = vrpn_DEFAULT_LISTEN_PORT_NO):
        m_machine(machine),
        m_port(port),
        Module(machine + ":" + std::to_string(port))
    {}

    virtual ~VRPNDeviceServer() {}

    void addDevice(std::string deviceName, DeviceType deviceType, size_t id = 0);

protected:

    void initModule() override;
    void runModule() override;
    void cleanUpModule() override;

private:

    const std::string m_machine; //!< machine name or IP
    const int m_port;            //!< connection port

    std::map<std::string, std::pair<DeviceType, size_t>> m_deviceInfoMap; //!< list of iMSTK client info
    vrpn_Connection * m_serverConnection;                                //!< VRPN server connection
    vrpn_MainloopContainer* m_deviceConnections;                         //!< VRPN device connections

};
}

#endif // ifndef imstkVRPNDeviceServer_h
