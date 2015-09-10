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

#include "Core/Vector.h"
#include "Core/Quaternion.h"
#include "Core/Timer.h"
#include "Devices/DeviceInterface.h"

#include <vrpn_Types.h>
#include <vrpn_Button.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Tracker.h>

#include <string>
#include <memory>
#include <chrono>
#include <array>

class VRPNPhantomDevice : public DeviceInterface
{
public:
    VRPNPhantomDevice();
    virtual ~VRPNPhantomDevice();
    DeviceInterface::Message openDevice() override;
    DeviceInterface::Message closeDevice() override;

    void init() override;
    void exec() override;
    void beginFrame() override;
    void endFrame() override;

    void setDeviceURL(const std::string s);
    std::string getDeviceURL();
    void setPollDelay(const std::chrono::milliseconds d);
    std::chrono::milliseconds getPollDelay();
    core::Vec3d getForce();
    core::Vec3d getPosition();
    core::Quaterniond getOrientation();
    bool getButton(size_t i);
    long double getForceETime();
    long double getPositionETime();
    long double getOrientationETime();
    long double getButtonETime(size_t i);

private:
    std::string deviceURL;
    std::chrono::milliseconds delay;

    std::array<bool, 2> buttons; //< Buttons: true = pressed/false = not pressed
    core::Vec3d force; //this should really be a vec3 or something
    core::Vec3d pos; //this should really be a vec3 or something
    core::Quaterniond quat; //this should really be a quat or something

    std::array<core::Timer, 2> buttonTimers;
    core::Timer forceTimer;
    core::Timer posTimer;
    core::Timer quatTimer;

    std::shared_ptr<vrpn_Button_Remote> vrpnButton;
    std::shared_ptr<vrpn_ForceDevice_Remote> vrpnForce;
    std::shared_ptr<vrpn_Tracker_Remote> vrpnTracker;

    void processChanges();

    static void VRPN_CALLBACK buttonChangeHandler(void *userData, const vrpn_BUTTONCB b);
    static void VRPN_CALLBACK forceChangeHandler(void *userData, const vrpn_FORCECB f);
    static void VRPN_CALLBACK trackerChangeHandler(void *userData, const vrpn_TRACKERCB b);
};

#endif
