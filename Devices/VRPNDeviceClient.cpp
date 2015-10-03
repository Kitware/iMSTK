// This file is part of the SimMedTK project.
// Copyright (c) Kitware, Inc.
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

#include "Devices/VRPNDeviceClient.h"

// VRPN includes
#include <vrpn_Button.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>

VRPNDeviceClient::VRPNDeviceClient()
    : deviceURL("Device0@localhost")
    {
        this->name = "VRPNDeviceClient";
    }

//---------------------------------------------------------------------------
VRPNDeviceClient::~VRPNDeviceClient() {}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNDeviceClient::openDevice()
{
    this->vrpnButton = std::make_shared<vrpn_Button_Remote>(this->deviceURL.c_str());
    this->vrpnTracker = std::make_shared<vrpn_Tracker_Remote>(this->deviceURL.c_str());
    this->vrpnAnalog = std::make_shared<vrpn_Analog_Remote>(this->deviceURL.c_str());

    this->vrpnButton->register_change_handler(this, buttonChangeHandler);
    this->vrpnTracker->register_change_handler(this, trackerChangeHandler);
    this->vrpnTracker->register_change_handler(this, velocityChangeHandler);
    this->vrpnAnalog->register_change_handler(this, analogChangeHandler);

    return DeviceInterface::Message::Success;
}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNDeviceClient::closeDevice()
{
    this->vrpnButton->unregister_change_handler(this, buttonChangeHandler);
    this->vrpnTracker->unregister_change_handler(this, trackerChangeHandler);
    this->vrpnTracker->unregister_change_handler(this, velocityChangeHandler);
    this->vrpnAnalog->unregister_change_handler(this, analogChangeHandler);

    this->vrpnButton.reset();
    this->vrpnTracker.reset();
    this->vrpnAnalog.reset();
    return DeviceInterface::Message::Success;
}

//---------------------------------------------------------------------------
void VRPNDeviceClient::exec()
{
    while(!this->terminateExecution)
    {
        this->processChanges();
        std::this_thread::sleep_for(this->pollDelay);
    }
    this->terminationCompleted = true;
}

//---------------------------------------------------------------------------
void VRPNDeviceClient::setDeviceURL(const std::string s)
{
    this->deviceURL = s;
}

//---------------------------------------------------------------------------
const std::string &VRPNDeviceClient::getDeviceURL() const
{
    return this->deviceURL;
}

//---------------------------------------------------------------------------
void VRPNDeviceClient::processChanges()
{
    this->vrpnButton->mainloop();
    this->vrpnTracker->mainloop();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNDeviceClient::buttonChangeHandler(void *userData, const vrpn_BUTTONCB b)
{
    auto handler = reinterpret_cast<VRPNDeviceClient*>(userData);

    if (b.button < vrpn_int32(handler->buttons.size()))
    {
        handler->buttons[b.button] = (1 == b.state);
        handler->buttonTimers[b.button].start();
    }
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNDeviceClient::velocityChangeHandler(void *userData, const vrpn_TRACKERVELCB v)
{
    auto handler = reinterpret_cast<VRPNDeviceClient*>(userData);

    handler->velocity << v.vel[0], v.vel[1], v.vel[2];
    handler->velTimer.start();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNDeviceClient::trackerChangeHandler(void *userData, const vrpn_TRACKERCB t)
{
    auto handler = reinterpret_cast<VRPNDeviceClient*>(userData);

    handler->position << t.pos[0], -t.pos[1], -t.pos[2];
    handler->posTimer.start();
    handler->orientation.w() = t.quat[0];
    handler->orientation.x() = t.quat[1];
    handler->orientation.y() = t.quat[2];
    handler->orientation.z() = t.quat[3];
    handler->quatTimer.start();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNDeviceClient::analogChangeHandler(void* userData, const vrpn_ANALOGCB a)
{
    auto handler = reinterpret_cast< VRPNDeviceClient * >( userData );

    if(a.num_channel > 0)
    {
        handler->position << a.channel[0], -a.channel[1], -a.channel[2];
        handler->posTimer.start();
    }
    if(a.num_channel > 3)
    {
        handler->orientation = Eigen::AngleAxisd(a.channel[3]*M_PI,core::Vec3d::UnitX())*
                               Eigen::AngleAxisd(a.channel[4]*M_PI,core::Vec3d::UnitY())*
                               Eigen::AngleAxisd(a.channel[5]*M_PI,core::Vec3d::UnitZ());
        handler->quatTimer.start();
    }
    return;
}
