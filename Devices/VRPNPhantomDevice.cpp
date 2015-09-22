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

#include "Devices/VRPNPhantomDevice.h"

VRPNPhantomDevice::VRPNPhantomDevice()
{
    this->setDeviceURL("Phantom0@localhost");
}

//---------------------------------------------------------------------------
VRPNPhantomDevice::~VRPNPhantomDevice(){}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNPhantomDevice::openDevice()
{
    this->vrpnForce = std::make_shared<vrpn_ForceDevice_Remote>(this->deviceURL.c_str());
    this->vrpnForce->register_force_change_handler(this, forceChangeHandler);
    return VRPNDeviceClient::openDevice();
}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNPhantomDevice::closeDevice()
{
    this->vrpnForce->unregister_force_change_handler(this, forceChangeHandler);
    this->vrpnForce.reset();
    return VRPNDeviceClient::closeDevice();
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::processChanges()
{
    VRPNDeviceClient::processChanges();
    this->vrpnForce->mainloop();
}

//---------------------------------------------------------------------------
void VRPNPhantomDevice::init()
{
    this->buttons.resize(2);
    this->buttonTimers.resize(2);
    DeviceInterface::init();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNPhantomDevice::forceChangeHandler(void *userData, const vrpn_FORCECB f)
{
    auto handler = reinterpret_cast<VRPNPhantomDevice*>(userData);

    handler->force << f.force[0],f.force[1],f.force[2];
    handler->forceTimer.start();
}
