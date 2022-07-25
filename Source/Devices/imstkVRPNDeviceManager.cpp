/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVRPNDeviceManager.h"

#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Tracker.h"

#include "imstkDeviceClient.h"
#include "imstkLogger.h"
#include "imstkVRPNDeviceClient.h"

namespace imstk
{
VRPNDeviceManager::VRPNDeviceManager(const std::string& machine /*= "localhost"*/, int port /*= vrpn_DEFAULT_LISTEN_PORT_NO*/) : Module(),
    m_machine(machine),
    m_port(port),
    m_deviceConnections(new vrpn_MainloopContainer())
{
    setSleepDelay(1000 / 60);
}

void
VRPNDeviceManager::addDeviceClient(std::shared_ptr<VRPNDeviceClient> client)
{
    std::string name   = client->getDeviceName();
    void*       handle = client.get();

    std::string address  = name + "@" + m_machine;
    const char* _address = address.c_str();

    int type = client->getType();

    if ( (type & VRPNAnalog) != 0)
    {
        LOG(INFO) << "Adding Analog Device: " << name;
        vrpn_Analog_Remote* vrpnAnalog = new vrpn_Analog_Remote(_address);
        m_deviceConnections->add(vrpnAnalog);
        vrpnAnalog->register_change_handler(handle, VRPNDeviceClient::analogChangeHandler);
        m_deviceMap[VRPNAnalog].push_back(client);
    }
    if ( (type & VRPNTracker) != 0)
    {
        LOG(INFO) << "Adding Tracker Device: " << name;
        vrpn_Tracker_Remote* vrpnTracker = new vrpn_Tracker_Remote(_address);
        m_deviceConnections->add(vrpnTracker);
        vrpnTracker->register_change_handler(handle, VRPNDeviceClient::trackerPositionChangeHandler);
        vrpnTracker->register_change_handler(handle, VRPNDeviceClient::trackerVelocityChangeHandler);
        m_deviceMap[VRPNTracker].push_back(client);
    }
    if ( (type & VRPNButton) != 0)
    {
        LOG(INFO) << "Adding Button Device: " << name;
        vrpn_Button_Remote* vrpnButton = new vrpn_Button_Remote(_address);
        m_deviceConnections->add(vrpnButton);
        vrpnButton->register_change_handler(handle, VRPNDeviceClient::buttonChangeHandler);
        m_deviceMap[VRPNButton].push_back(client);
    }
}

std::shared_ptr<imstk::DeviceClient>
VRPNDeviceManager::makeDeviceClient(const std::string& deviceName, VRPNDeviceType deviceType)
{
    auto client = std::make_shared<VRPNDeviceClient>(deviceName, deviceType, m_machine);
    addDeviceClient(client);
    return client;
}

bool
VRPNDeviceManager::initModule()
{
    return true;
}

void
VRPNDeviceManager::updateModule()
{
    m_deviceConnections->mainloop();
}

void
VRPNDeviceManager::uninitModule()
{
    m_deviceConnections->clear();
}
} // imstk
