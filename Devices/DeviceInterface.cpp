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


#include "Devices/DeviceInterface.h"


DeviceInterface::DeviceInterface(): driverInstalled(false), pollDelay(100)
{
    this->name = "DeviceInterface";
}

//---------------------------------------------------------------------------
DeviceInterface::~DeviceInterface()
{

}

//---------------------------------------------------------------------------
void DeviceInterface::setPollDelay(const std::chrono::milliseconds &d)
{
    this->pollDelay = d;
}

//---------------------------------------------------------------------------
const std::chrono::milliseconds &DeviceInterface::getPollDelay() const
{
    return this->pollDelay;
}

//---------------------------------------------------------------------------
long double DeviceInterface::getForceETime()
{
    return this->forceTimer.elapsed();
}

//---------------------------------------------------------------------------
long double DeviceInterface::getPositionETime()
{
    return this->posTimer.elapsed();
}

//---------------------------------------------------------------------------
long double DeviceInterface::getOrientationETime()
{
    return this->quatTimer.elapsed();
}

//---------------------------------------------------------------------------
const core::Vec3d &DeviceInterface::getForce() const
{
    return this->force;
}

//---------------------------------------------------------------------------
void DeviceInterface::setForce(const core::Vec3d& f)
{
    this->force = f;
}

//---------------------------------------------------------------------------
const core::Vec3d &DeviceInterface::getTorque() const
{
    return this->torque;
}

//---------------------------------------------------------------------------
const core::Vec3d &DeviceInterface::getPosition() const
{
    return this->position;
}

//---------------------------------------------------------------------------
const core::Vec3d &DeviceInterface::getVelocity() const
{
    return this->velocity;
}

//---------------------------------------------------------------------------
const core::Quaterniond &DeviceInterface::getOrientation() const
{
    return this->orientation;
}

//---------------------------------------------------------------------------
bool DeviceInterface::getButton(size_t i) const
{
    auto numButtons = this->buttons.size();
    if (i < numButtons)
        return this->buttons[i];
    else
        return false;
}

//---------------------------------------------------------------------------
long double DeviceInterface::getButtonETime(size_t i)
{
    auto numButtons = this->buttonTimers.size();
    if (i < numButtons)
        return this->buttonTimers[i].elapsed();
    else
        return -1;
}

//---------------------------------------------------------------------------
bool DeviceInterface::init()
{
    this->forceTimer.start();
    this->posTimer.start();
    this->quatTimer.start();
    this->velTimer.start();
    for(auto &buttonTimer : this->buttonTimers)
    {
        buttonTimer.start();
    }
    return true;
}

//---------------------------------------------------------------------------
void DeviceInterface::beginFrame()
{
}

//---------------------------------------------------------------------------
void DeviceInterface::endFrame()
{
}

//---------------------------------------------------------------------------
void DeviceInterface::exec()
{
}

//---------------------------------------------------------------------------
DeviceInterface::Message DeviceInterface::openDevice()
{
    return Message::Unknown;
}

//---------------------------------------------------------------------------
DeviceInterface::Message DeviceInterface::closeDevice()
{
    return Message::Unknown;
}

//---------------------------------------------------------------------------
DeviceInterface::Message DeviceInterface::write(void*, int, void*)
{
    return  Message::Unknown;
}

//---------------------------------------------------------------------------
DeviceInterface::Message DeviceInterface::read(void*, int, void*)
{
    return Message::Unknown;
}
