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

#ifndef SM_VRPNPHANTOMDEVICE_H
#define SM_VRPNPHANTOMDEVICE_H

#include "Device.h"

#include <vrpn_Connection.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Tracker.h>
#include <server_src/vrpn_Phantom.h>

#include <string>
#include <memory>
#include <chrono>
#include <array>

class VRPNPhantomDevice : Device
{
public:
    VRPNPhantomDevice();
    virtual ~VRPNPhantomDevice();
    Device::Message open() override;
    Device::Message close() override;
    void exec() override;

    void setDeviceURL(const string s);
    const std::string getDeviceURL();
    void setPollDelay(const std::chrono::duration d);
    const std::chrono::duration getPollDelay();

private:
    bool terminate;
    std::string deviceURL;
    std::chrono::duration delay;

    //Need getters and setters for these
    std::array<bool, 2> buttons; //< Buttons: true = pressed/false = not pressed
    std::array<vprn_float64, 3> force; //this should really be a vec3 or something
    std::array<vprn_float64, 3> pos; //this should really be a vec3 or something
    std::array<vprn_float64, 4> quat; //this should really be a quat or something

    std::shared_ptr<vrpn_Button_Remote> vrpnButton;
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce;
    std::shared_ptr<vrpn_Tracker_Remote> vrpnTracker;

    void processChanges();
    void VRPN_CALLBACK buttonChangeHandler(void *userData, const vrpn_BUTTONCB b);
    void VRPN_CALLBACK forceChangeHandler(void *userData, const vrpn_FORCECB f);
    void VRPN_CALLBACK trackerChangeHandler(void *userData, const vrpn_TRACKERCB b);
};


class VRPNPhantomDeviceServer : Device
{
public:
    VRPNPhantomDeviceServer();
    virtual ~VRPNPhantomDeviceServer();

    Device::Message open() override;
    Device::Message close() override;
    void exec() override;

    void setDeviceName(const string s);
    const std::string getDeviceName();
    void setPollDelay(const std::chrono::duration d);
    const std::chrono::duration getPollDelay();

private:
    bool terminate;
    std::string deviceName;
    std::chrono::duration delay;

    vrpn_Connection connection;
    std::shared_ptr<vrpn_Phantom> phantom;
};

#endif
