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
VRPNDeviceServer::VRPNDeviceServer(const std::string& machine /*= "localhost"*/, int port /*= vrpn_DEFAULT_LISTEN_PORT_NO*/) : Module(),
    m_machine(machine),
    m_port(port),
    m_deviceConnections(new vrpn_MainloopContainer())
{
    setSleepDelay(1000 / 60);
}

void
VRPNDeviceServer::addDeviceClient(std::shared_ptr<VRPNDeviceClient> client)
{
    std::string name   = client->getDeviceName();
    void*       handle = client.get();

    std::string address  = name + "@" + m_machine;
    const char* _address = address.c_str();

    int type = client->getType();

    if ( (type & VRPNAnalog) != 0)
    {
        LOG(INFO) << "Analog: " << name;
        SerialInfo          connectionSettings = m_SerialInfoMap[name];
        vrpn_Analog_Remote* vrpnAnalog = new vrpn_Analog_Remote(_address);
        m_deviceConnections->add(vrpnAnalog);

        vrpnAnalog->register_change_handler(handle, VRPNDeviceClient::analogChangeHandler);
    }
    if ( (type & VRPNTracker) != 0)
    {
        LOG(INFO) << "Tracker: " << name;
        vrpn_Tracker_Remote* vrpnTracker = new vrpn_Tracker_Remote(_address);
        m_deviceConnections->add(vrpnTracker);

        vrpnTracker->register_change_handler(handle, VRPNDeviceClient::trackerChangeHandler);
    }
    if ( (type & VRPNButton) != 0)
    {
        LOG(INFO) << "Button: " << name;
        vrpn_Button_Remote* vrpnButton = new vrpn_Button_Remote(_address);
        m_deviceConnections->add(vrpnButton);

        vrpnButton->register_change_handler(handle, VRPNDeviceClient::buttonChangeHandler);
    }
}

std::shared_ptr<imstk::DeviceClient>
VRPNDeviceServer::getClient(const std::string& deviceName, VRPNDeviceType deviceType)
{
    auto client = std::make_shared<VRPNDeviceClient>(deviceName, deviceType, m_machine);
    addDeviceClient(client);
    return client;
}

bool
VRPNDeviceServer::initModule()
{
    return true;
}

void
VRPNDeviceServer::updateModule()
{
    m_deviceConnections->mainloop();
}

void
VRPNDeviceServer::uninitModule()
{
    m_deviceConnections->clear();
}
} // imstk
