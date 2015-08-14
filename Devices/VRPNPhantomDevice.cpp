#include "VRPNPhantomDevice.h"

#include <vrpn_Connection.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Tracker.h>
#include <server_src/vrpn_Phantom.h>

#include <chrono>
#include <thread>
#include <memory>
#include <algorithm>

VRPNPhantomDevice::VRPNPhantomDevice()
    : terminate{false},
      deviceURL("Phantom0@localhost"),
      delay(std::chrono::milliseconds(100))
{

}

VRPNPhantomDevice::~VRPNPhantomDevice()
{

}

Device::Message VRPNPhantomDevice::open()
{
    vrpnButton = std::make_shared<vrpn_Button_Remote>(this->deviceURL);
    vrpnForce = std::make_shared<vrpn_ForceDevice_Remote>(this->deviceURL);
    vrpnTracker = std::make_shared<vrpn_Tracker_Remote>(this->deviceURL);
    //need try/catch incase the memory couldn't be allocated, the return
    // Message::Failure;

    vrpnButton->register_change_handler( this, ButtonChangeHandler );
    vrpnForce->register_force_change_handler( this, ForceChangeHandler );
    vrpnTracker->register_change_handler( this, TrackerChangeHandler );

    return Device::Message::Success;
}

Device::Message VRPNPhantomDevice::close()
{
    terminate = true;
    return Device::Message::Success;
}

void VRPNPhantomDevice::exec()
{
    while(!terminate)
    {
        processChanges();
        std::this_thread::sleep_for(delay);
    }
}

void VRPNPhantomDevice::setDeviceURL(const string s)
{
    deviceURL = s;
}

const std::string VRPNPhantomDevice::getDeviceURL()
{
    return deviceURL;
}

void VRPNPhantomDevice::setPollDelay(const std::chrono::duration d)
{
    delay = d;
}

const std::chrono::duration VRPNPhantomDevice::getPollDelay()
{
    return delay;
}

void VRPNPhantomDevice::processChanges()
{
    vrpnButton->mainloop();
    vrpnForce->mainloop();
    vrpnTracker->mainloop();
}

void VRPN_CALLBACK
VRPNPhantomDevice::buttonChangeHandler(void *userData, const vrpn_BUTTONCB b)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    if (b.button < handler->buttons.size())
    {
        buttons[b.button] = (1 == b.state);
        //Should also handle time stamps
    }//else the button isn't accounted for, as far as we know, it didn't exist
}

void VRPN_CALLBACK
VRPNPhantomDevice::forceChangeHandler(void *userData, const vrpn_FORCECB f)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    std::copy(std::begin(f.force), std::end(f.force), std::begin(handler->force));
    //Should also handle time stamps
}

void VRPN_CALLBACK
VRPNPhantomDevice::trackerChangeHandler(void *userData, const vrpn_TRACKERCB t)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    std::copy(std::begin(t.pos), std::end(t.pos), std::begin(handler->pos));
    std::copy(std::begin(t.quat), std::end(t.quat), std::begin(handler->quat));
    //Should also handle time stamps
}




VRPNPhantomDeviceServer::VRPNPhantomDeviceServer()
    : terminate{false},
      deviceName("Phantom0"),
      delay(std::chrono::milliseconds(100))
{

}

VRPNPhantomDeviceServer::~VRPNPhantomDeviceServer()
{
    delete connection;
}

Device::Message VRPNPhantomDeviceServer::open()
{
    connection = vrpn_create_server_connection();
    phantom = std::make_shared<vrpn_Phantom>(deviceName, connection, 60.0f);
    //need try/catch incase the memory couldn't be allocated, the return
    // Message::Failure;
    return Device::Message::Success;
}

Device::Message VRPNPhantomDeviceServer::close()
{
    terminate = true;
    return Device::Message::Success;
}

void VRPNPhantomDeviceServer::exec()
{
    while(!terminate)
    {
        phantom->mainloop();
        connection->mainloop();
        std::this_thread::sleep_for(delay);
    }
}

void VRPNPhantomDeviceServer::setDeviceName(const string s)
{
    deviceName = s;
}

const std::string VRPNPhantomDeviceServer::getDeviceName()
{
    return deviceName;
}

void VRPNPhantomDeviceServer::setPollDelay(const std::chrono::duration d)
{
    delay = d;
}

const std::chrono::duration VRPNPhantomDeviceServer::getPollDelay()
{
    return delay;
}
