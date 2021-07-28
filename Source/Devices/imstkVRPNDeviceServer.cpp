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
#include "imstkLogger.h"

#include <vrpn_3DConnexion.h>
#include <vrpn_Streaming_Arduino.h>
#include <vrpn_Tracker_NovintFalcon.h>
#include <vrpn_Tracker_OSVRHackerDevKit.h>

//VRPN
#include "imstkDeviceClient.h"
#include "imstkVRPNDeviceClient.h"
#include <vrpn_Analog.h>
#include <vrpn_Tracker.h>
#include "quat.h"

#ifdef VRPN_USE_PHANTOM_SERVER
#include <vrpn_Phantom.h>
#endif

namespace imstk
{
    void
    VRPNDeviceServer::addDeviceClient(std::shared_ptr<VRPNDeviceClient> client, const std::string& deviceName, VRPNDeviceType deviceType, int id)
    {
        m_deviceClients2.push_back(client);
        //m_deviceClient1 = client;
        m_deviceInfoMap[deviceName] = std::make_pair(deviceType, id);

    }


bool
VRPNDeviceServer::initModule()
{
    std::string ip = m_machine + ":" + std::to_string(m_port);
    m_serverConnection = vrpn_create_server_connection(ip.c_str());

    m_deviceConnections = new vrpn_MainloopContainer();

    for (const auto& device : m_deviceInfoMap)
    {
        std::string name = device.first;
        VRPNDeviceType  type = device.second.first;
        auto        id   = device.second.second;

        std::string address = name + "@" + m_machine;
        const char* _address = address.c_str();

        switch (type)
        {
        case VRPNDeviceType::Analog:
        {
            std::cout << "ANALOG" << std::endl;
            SerialInfo connectionSettings = m_SerialInfoMap[name];
            vrpn_Analog_Remote* vrpnAnalog = new vrpn_Analog_Remote(_address);
            m_deviceConnections->add(vrpnAnalog);

            vrpnAnalog->register_change_handler(this, VRPNDeviceClient::analogChangeHandler);


        } break;
        case VRPNDeviceType::Tracker:
        {
            std::cout << "TRACKER" << std::endl;
            SerialInfo connectionSettings = m_SerialInfoMap[name];
            //vrpn_Analog_Remote* vrpnAnalog = new vrpn_Analog_Remote(_address);
            vrpn_Tracker_Remote* vrpnTracker = new vrpn_Tracker_Remote(_address);
            m_deviceConnections->add(vrpnTracker);

            vrpnTracker->register_change_handler(this, VRPNDeviceClient::trackerChangeHandler);
        } break;
        }
    }
    return true;
}

void
VRPNDeviceServer::updateModule()
{
    m_serverConnection->mainloop();
    m_deviceConnections->mainloop();
}

void
VRPNDeviceServer::uninitModule()
{
    m_deviceConnections->clear();
    delete(m_deviceConnections);

    m_serverConnection->removeReference();
    //delete(m_connection);
}
} // imstk
