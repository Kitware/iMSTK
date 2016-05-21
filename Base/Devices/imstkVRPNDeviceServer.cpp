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

#include "imstkVRPNDeviceServer.h"

#include "vrpn_3DConnexion.h"
#define VRPN_USE_LIBNIFALCON
#define VERBOSE
#include "vrpn_Tracker_NovintFalcon.h"
#include "vrpn_Tracker_OSVRHackerDevKit.h"

#include "g3log/g3log.hpp"

namespace imstk {
void
VRPNDeviceServer::addDevice(std::string deviceName, DeviceType deviceType, size_t id)
{
    m_deviceInfoMap[deviceName] = std::make_pair(deviceType,id);
}

void
VRPNDeviceServer::initModule()
{
    std::string ip = m_machine + ":" + std::to_string(m_port);
    m_serverConnection = vrpn_create_server_connection(ip.c_str());

    m_deviceConnections = new vrpn_MainloopContainer();

    for (const auto& device : m_deviceInfoMap)
    {
        std::string name = device.first;
        DeviceType type = device.second.first;
        size_t id = device.second.second;

        switch (type)
        {
        case DeviceType::SPACE_EXPLORER_3DCONNEXION:
        {
            m_deviceConnections->add(new vrpn_3DConnexion_SpaceExplorer(name.c_str(), m_serverConnection));
        } break;
        case DeviceType::NAVIGATOR_3DCONNEXION:
        {
            m_deviceConnections->add(new vrpn_3DConnexion_Navigator(name.c_str(), m_serverConnection));
        } break;
        case DeviceType::NOVINT_FALCON:
        {
            m_deviceConnections->add(new vrpn_Tracker_NovintFalcon(name.c_str(), m_serverConnection,
                                                                   id, "4-button", "stamper"));
        } break;
        case DeviceType::OSVR_HDK:
        {
            m_deviceConnections->add(new vrpn_Tracker_OSVRHackerDevKit(name.c_str(), m_serverConnection));
        } break;
        default:
        {
            LOG(WARNING) << "VRPNDeviceServer::initModule error: can not connect to "
                         << name<< ", device type unknown.";
        } break;
        }
    }
}

void
VRPNDeviceServer::runModule()
{
    m_serverConnection->mainloop();
    m_deviceConnections->mainloop();
}

void
VRPNDeviceServer::cleanUpModule()
{
    m_deviceConnections->clear();
    delete(m_deviceConnections);

    m_serverConnection->removeReference();
    //delete(m_connection);
}
}
