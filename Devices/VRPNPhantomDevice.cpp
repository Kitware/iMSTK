#include "VRPNPhantomDevice.h"

#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Tracker.h>

#include <chrono>
#include <thread>
#include <memory>
#include <algorithm>

VRPNPhantomDevice::VRPNPhantomDevice()
    : deviceURL("Phantom0@localhost"),
      delay(std::chrono::milliseconds(100))
{

}

VRPNPhantomDevice::~VRPNPhantomDevice()
{

}

DeviceInterface::Message VRPNPhantomDevice::openDevice()
{
    vrpnButton = std::make_shared<vrpn_Button_Remote>(this->deviceURL.c_str());
    vrpnForce = std::make_shared<vrpn_ForceDevice_Remote>(this->deviceURL.c_str());
    vrpnTracker = std::make_shared<vrpn_Tracker_Remote>(this->deviceURL.c_str());
    //need try/catch incase the memory couldn't be allocated, then return
    // Message::Failure;

    vrpnButton->register_change_handler(this, buttonChangeHandler);
    vrpnForce->register_force_change_handler(this, forceChangeHandler);
    vrpnTracker->register_change_handler(this, trackerChangeHandler);

    return DeviceInterface::Message::Success;
}

DeviceInterface::Message VRPNPhantomDevice::closeDevice()
{
    this->terminate();
    return DeviceInterface::Message::Success;
}

void VRPNPhantomDevice::init()
{
    buttonTimers[0].start();
    buttonTimers[1].start();
    forceTimer.start();
    posTimer.start();
    quatTimer.start();
}

void VRPNPhantomDevice::exec()
{
    while(!terminateExecution)
    {
        processChanges();
        std::this_thread::sleep_for(delay);
    }
}

void VRPNPhantomDevice::beginFrame()
{
}

void VRPNPhantomDevice::endFrame()
{
}

void VRPNPhantomDevice::setDeviceURL(const std::string s)
{
    deviceURL = s;
}

std::string VRPNPhantomDevice::getDeviceURL()
{
    return deviceURL;
}

void VRPNPhantomDevice::setPollDelay(const std::chrono::milliseconds d)
{
    delay = d;
}

std::chrono::milliseconds VRPNPhantomDevice::getPollDelay()
{
    return delay;
}

void VRPNPhantomDevice::processChanges()
{
    vrpnButton->mainloop();
    vrpnForce->mainloop();
    vrpnTracker->mainloop();
}

core::Vec3d VRPNPhantomDevice::getForce()
{
    return force;
}

core::Vec3d VRPNPhantomDevice::getPosition()
{
    return pos;
}

core::Quaterniond VRPNPhantomDevice::getOrientation()
{
    return quat;
}

bool VRPNPhantomDevice::getButton(size_t i)
{
    if (i < 2)
        return buttons[i];
    else
        return false;
}

long double VRPNPhantomDevice::getForceETime()
{
    return forceTimer.elapsed();
}

long double VRPNPhantomDevice::getPositionETime()
{
    return posTimer.elapsed();
}

long double VRPNPhantomDevice::getOrientationETime()
{
    return quatTimer.elapsed();
}

long double VRPNPhantomDevice::getButtonETime(size_t i)
{
    if (i < 2)
        return buttonTimers[i].elapsed();
    else
        return -1;
}


void VRPN_CALLBACK
VRPNPhantomDevice::buttonChangeHandler(void *userData, const vrpn_BUTTONCB b)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    if (b.button < handler->buttons.size())
    {
        handler->buttons[b.button] = (1 == b.state);
        handler->buttonTimers[b.button].start();
    }//else the button isn't accounted for, as far as we know, it didn't exist
}

void VRPN_CALLBACK
VRPNPhantomDevice::forceChangeHandler(void *userData, const vrpn_FORCECB f)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    handler->force = Eigen::Map<const core::Vec3d>(f.force);
    handler->forceTimer.start();
}

void VRPN_CALLBACK
VRPNPhantomDevice::trackerChangeHandler(void *userData, const vrpn_TRACKERCB t)
{
    VRPNPhantomDevice *handler = reinterpret_cast<VRPNPhantomDevice*>(userData);
    handler->pos = Eigen::Map<const core::Vec3d>(t.pos);
    handler->posTimer.start();
    handler->quat = Eigen::Map<const core::Quaterniond>(t.quat);
    handler->quatTimer.start();
}
