// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include "Devices/VRPNForceDevice.h"
#include "Core/Matrix.h"

// VRPN includes
#include <vrpn_ForceDevice.h>

namespace imstk {

VRPNForceDevice::VRPNForceDevice(std::string deviceURL)
    : VRPNDeviceClient(DeviceType::PHANTOM_OMNI, deviceURL),
    enableForce(true),
    contactPlane(Vec4f(0.0,1.0,0.0,100)),
    dampingCoefficient(0.5),
    dynamicFriction(0.0),
    springCoefficient(1.0),
    staticFriction(0.0),
    vrpnForce(nullptr)
{
    this->name = "VRPNForceDevice";
}

//---------------------------------------------------------------------------
VRPNForceDevice::~VRPNForceDevice(){}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNForceDevice::openDevice()
{
    this->vrpnForce = std::make_shared<vrpn_ForceDevice_Remote>(this->deviceURL.c_str());
    this->vrpnForce->register_force_change_handler(this, forceChangeHandler);
    return VRPNDeviceClient::openDevice();
}

//---------------------------------------------------------------------------
DeviceInterface::Message VRPNForceDevice::closeDevice()
{
    this->vrpnForce->unregister_force_change_handler(this, forceChangeHandler);
    this->vrpnForce.reset();
    return VRPNDeviceClient::closeDevice();
}

//---------------------------------------------------------------------------
void VRPNForceDevice::processChanges()
{
    VRPNDeviceClient::processChanges();
    this->vrpnForce->mainloop();
}

//---------------------------------------------------------------------------
void VRPN_CALLBACK
VRPNForceDevice::forceChangeHandler(void *userData, const vrpn_FORCECB f)
{
    auto handler = reinterpret_cast<VRPNForceDevice*>(userData);

    handler->force << f.force[0],f.force[1],f.force[2];
    handler->forceTimer.start();

    handler->vrpnForce->set_plane(handler->contactPlane.data());
    handler->vrpnForce->sendSurface();

    // Update other force settings
    handler->vrpnForce->setSurfaceFstatic( handler->staticFriction );
    handler->vrpnForce->setSurfaceFdynamic( handler->dynamicFriction );
    handler->vrpnForce->setSurfaceKspring( handler->springCoefficient );
    handler->vrpnForce->setSurfaceKdamping( handler->dampingCoefficient );
    handler->vrpnForce->setSurfaceBuzzAmplitude( 0.0 );
    handler->vrpnForce->setSurfaceBuzzFrequency( 60.0 ); // Hz
    handler->vrpnForce->setSurfaceTextureAmplitude( 0.00 ); // meters
    handler->vrpnForce->setSurfaceTextureWavelength( 0.01f ); // meters
    handler->vrpnForce->setRecoveryTime( 10 );
}

//---------------------------------------------------------------------------
bool VRPNForceDevice::init()
{
    this->buttons.resize(2);
    this->buttonTimers.resize(2);
    return DeviceInterface::init();
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setEnableForce(const bool &enable)
{
    this->enableForce = enable;
}

//---------------------------------------------------------------------------
const bool &VRPNForceDevice::getEnableForce() const
{
    return this->enableForce;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setContactPlane(const Vec4f &plane)
{
    this->contactPlane = plane;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setContactPlane(const Vec3f &plane, const float d)
{
    this->contactPlane << plane, d;
}

//---------------------------------------------------------------------------
const Vec4f &VRPNForceDevice::getContactPlane() const
{
    return this->contactPlane;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setDampingCoefficient(const double &coeff)
{
    this->dampingCoefficient = coeff;
}

//---------------------------------------------------------------------------
const double &VRPNForceDevice::getDampingCoefficient() const
{
    return this->dampingCoefficient;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setDynamicFriction(const double &coeff)
{
    this->dynamicFriction = coeff;
}

//---------------------------------------------------------------------------
const double &VRPNForceDevice::getDynamicFriction() const
{
    return this->dynamicFriction;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setSpringCoefficient(const double &coeff)
{
    this->springCoefficient = coeff;
}

//---------------------------------------------------------------------------
const double &VRPNForceDevice::getSpringCoefficient() const
{
    return this->springCoefficient;
}

//---------------------------------------------------------------------------
void VRPNForceDevice::setStaticFriction(const double &coeff)
{
    this->staticFriction = coeff;
}

//---------------------------------------------------------------------------
const double &VRPNForceDevice::getStaticFriction() const
{
    return this->staticFriction;
}

}
