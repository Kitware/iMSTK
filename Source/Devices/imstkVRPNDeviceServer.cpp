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
#ifdef VRPN_USE_PHANTOM_SERVER
#include <vrpn_Phantom.h>
#endif

namespace imstk
{
void
VRPNDeviceServer::addDevice(const std::string& deviceName, DeviceType deviceType, int id)
{
    m_deviceInfoMap[deviceName] = std::make_pair(deviceType, id);

    if (deviceType == DeviceType::PhantomOmni)
    {
        LOG(WARNING) << "VRPNDeviceServer::addDevice warning: OpenHaptics support on VRPN "
                     << "currently unstable for the Phantom Omni (no force feedback implemented).\n"
                     << "Use HDAPIDeviceClient instead of VRPNDeviceServer/Client for ";
    }
}

void
VRPNDeviceServer::addSerialDevice(const std::string& deviceName, DeviceType deviceType, const std::string& port, int baudRate, int id)
{
    SerialInfo serialSettings;
    serialSettings.baudRate     = baudRate;
    serialSettings.port         = port;
    m_deviceInfoMap[deviceName] = std::make_pair(deviceType, id);
    m_SerialInfoMap[deviceName] = serialSettings;
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
        DeviceType  type = device.second.first;
        auto        id   = device.second.second;

        switch (type)
        {
        case DeviceType::SpaceExplorer3DConnexion:
        {
            m_deviceConnections->add(new vrpn_3DConnexion_SpaceExplorer(name.c_str(), m_serverConnection));
        } break;
        case DeviceType::Navigator3DConnexion:
        {
            m_deviceConnections->add(new vrpn_3DConnexion_Navigator(name.c_str(), m_serverConnection));
        } break;
        case DeviceType::NovintFalcon:
        {
#ifdef VRPN_USE_LIBNIFALCON
            m_deviceConnections->add(new vrpn_Tracker_NovintFalcon(name.c_str(), m_serverConnection,
                                                                   id, "4-button", "stamper"));
#else
            LOG(WARNING) << "VRPNDeviceServer::initModule error: no support for Novint Falcon in VRPN. "
                         << "Build VRPN with VRPN_USE_LIBNIFALCON.";
#endif
        } break;
        case DeviceType::PhantomOmni:
        {
#ifdef VRPN_USE_PHANTOM_SERVER
            char* deviceName = const_cast<char*>(name.c_str());
            m_deviceConnections->add(new vrpn_Phantom(deviceName, m_serverConnection, 90.0f, deviceName));
#else
            LOG(WARNING) << "VRPNDeviceServer::initModule error: no support for Phantom Omni in VRPN. "
                         << "Install OpenHaptics SDK, the omni driver, and build VRPN with VRPN_USE_PHANTOM_SERVER.";
#endif
        } break;
        //case DeviceType::OSVR_HDK:
        //{
        //    m_deviceConnections->add(new vrpn_Tracker_OSVRHackerDevKit(name.c_str(), m_serverConnection));
        //} break;
        case DeviceType::Arduino:
        {
            SerialInfo connectionSettings = m_SerialInfoMap[name];
            //open with 6 channels (max needed for IMU, can use less)
            m_deviceConnections->add(new vrpn_Streaming_Arduino(name.c_str(), m_serverConnection, connectionSettings.port, 6, connectionSettings.baudRate));
        } break;
        default:
        {
            LOG(WARNING) << "VRPNDeviceServer::initModule error: can not connect to "
                         << name << ", device type unknown.";
            return false;
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
