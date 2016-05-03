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

#include "g3log/g3log.hpp"

namespace imstk {
bool
VRPNDeviceServer::addDeviceClient(const std::shared_ptr<VRPNDeviceClient> deviceClient)
{
    // Check that url is localhost
    if (deviceClient->getUrl() != "localhost")
    {
        LOG(WARNING) << "VRPNDeviceServer::addDeviceClient error: can not connect to "
                     << deviceClient->getUrl() << "\n"
                     << "Can only communicate with devices locally (url = localhost).";
        return EXIT_FAILURE;
    }

    m_clientsList.push_back(deviceClient);
}

void
VRPNDeviceServer::initModule()
{
    m_connection = vrpn_create_server_connection();

    for (const auto& client : m_clientsList)
    {
        std::string name = client->getName();

        switch (client->getType())
        {
        case DeviceType::SPACE_EXPLORER_3DCONNEXION:
        {
            m_devices->add(new vrpn_3DConnexion_SpaceExplorer(name.c_str(), m_connection));
        } break;
        case DeviceType::NAVIGATOR_3DCONNEXION:
        {
            m_devices->add(new vrpn_3DConnexion_Navigator(name.c_str(), m_connection));
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
    m_connection->mainloop();
    m_devices->mainloop();
}

void
VRPNDeviceServer::cleanUpModule()
{
    m_devices->clear();
    delete(m_devices);

    m_connection->removeReference();
    delete(m_connection);
}
}
