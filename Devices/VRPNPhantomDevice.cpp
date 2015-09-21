// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
VRPNPhantomDevice::~VRPNPhantomDevice()
{

}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNPhantomDevice::openDevice()
{
    this->vrpnButton = std::make_shared<vrpn_Button_Remote>(this->deviceURL.c_str());
    this->vrpnForce = std::make_shared<vrpn_ForceDevice_Remote>(this->deviceURL.c_str());
    this->vrpnTracker = std::make_shared<vrpn_Tracker_Remote>(this->deviceURL.c_str());
    //TODO: need try/catch incase the memory couldn't be allocated, then return
    // Message::Failure;

    this->vrpnButton->register_change_handler(this, buttonChangeHandler);
    this->vrpnForce->register_force_change_handler(this, forceChangeHandler);
    this->vrpnTracker->register_change_handler(this, trackerChangeHandler);

    return DeviceInterface::Message::Success;
}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNPhantomDevice::closeDevice()
{
    this->vrpnButton->unregister_change_handler(this, buttonChangeHandler);
    this->vrpnForce->unregister_force_change_handler(this, forceChangeHandler);
    this->vrpnTracker->unregister_change_handler(this, trackerChangeHandler);

    this->vrpnButton.reset();
    this->vrpnForce.reset();
    this->vrpnTracker.reset();
    return DeviceInterface::Message::Success;
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::init()
{
    this->buttonTimers[0].start();
    this->buttonTimers[1].start();
    this->forceTimer.start();
    this->posTimer.start();
    this->quatTimer.start();
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::exec()
{
    while(!this->terminateExecution)
    {
        this->processChanges();
        std::this_thread::sleep_for(this->delay);
    }
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::beginFrame()
{
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::endFrame()
{
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::setDeviceURL(const std::string s)
{
    this->deviceURL = s;
}

//---------------------------------------------------------------------------
const std::string &VRPNPhantomDevice::getDeviceURL() const
{
    return this->deviceURL;
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::setPollDelay(const std::chrono::milliseconds &d)
{
    this->delay = d;
}

//---------------------------------------------------------------------------
const std::chrono::milliseconds &VRPNPhantomDevice::getPollDelay() const
{
    return this->delay;
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::processChanges()
{
    this->vrpnButton->mainloop();
    this->vrpnForce->mainloop();
    this->vrpnTracker->mainloop();
}

//---------------------------------------------------------------------------
const core::Vec3d &VRPNPhantomDevice::getForce() const
{
    return this->force;
}

//---------------------------------------------------------------------------
const core::Vec3d &VRPNPhantomDevice::getPosition() const
{
    return this->pos;
}

//---------------------------------------------------------------------------
const core::Quaterniond &VRPNPhantomDevice::getOrientation() const
{
    return this->quat;
}

//---------------------------------------------------------------------------
bool VRPNPhantomDevice::getButton(size_t i) const
{
    if (i < 2)
        return this->buttons[i];
    else
        return false;
}

//---------------------------------------------------------------------------
long double VRPNPhantomDevice::getForceETime()
{
    return this->forceTimer.elapsed();
}

//---------------------------------------------------------------------------
long double VRPNPhantomDevice::getPositionETime()
{
    return this->posTimer.elapsed();
}

//---------------------------------------------------------------------------
long double VRPNPhantomDevice::getOrientationETime()
{
    return this->quatTimer.elapsed();
}

//---------------------------------------------------------------------------
long double VRPNPhantomDevice::getButtonETime(size_t i)
{
    if (i < 2)
        return this->buttonTimers[i].elapsed();
    else
        return -1;
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNPhantomDevice::buttonChangeHandler(void *userData, const vrpn_BUTTONCB b)
{
    auto handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    if (b.button < vrpn_int32(handler->buttons.size()))
    {
        handler->buttons[b.button] = (1 == b.state);
        handler->buttonTimers[b.button].start();
    }
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNPhantomDevice::forceChangeHandler(void *userData, const vrpn_FORCECB f)
{
    auto handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    handler->force[0] = f.force[0];
    handler->force[1] = f.force[1];
    handler->force[2] = f.force[2];
    handler->forceTimer.start();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNPhantomDevice::trackerChangeHandler(void *userData, const vrpn_TRACKERCB t)
{
    auto handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    handler->pos[0] = t.pos[0];
    handler->pos[1] = t.pos[1];
    handler->pos[2] = t.pos[2];
    handler->posTimer.start();
    handler->quat.w() = t.quat[0];
    handler->quat.x() = t.quat[1];
    handler->quat.y() = t.quat[2];
    handler->quat.z() = t.quat[3];
    handler->quatTimer.start();
}
