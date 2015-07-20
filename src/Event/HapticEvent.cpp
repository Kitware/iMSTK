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

#include "HapticEvent.h"

namespace event {

core::EventType HapticEvent::EventName = core::EventType::Haptic;

HapticEvent::HapticEvent(const size_t& deviceId, const std::string& deviceName): id(deviceId), name(deviceName)
{}

void HapticEvent::setPosition(const core::Vec3d& coordinates)
{
    this->position = coordinates;
}

const core::Vec3d& HapticEvent::getPosition()
{
    return this->position;
}
void HapticEvent::setVelocity(const core::Vec3d& deviceVelocity)
{
    this->velocity = deviceVelocity;
}
const core::Vec3d& HapticEvent::getVelocity()
{
    return this->velocity;
}
void HapticEvent::setAngles(const core::Vec3d& deviceAngles)
{
    this->angles = deviceAngles;
}
const core::Vec3d& HapticEvent::getAngles()
{
    return this->angles;
}
const Matrix44d& HapticEvent::getTransform()
{
    return this->transform;
}
void HapticEvent::setTransform(const Matrix44d& deviceTransform)
{
    this->transform = deviceTransform;
}
const core::Vec3d& HapticEvent::getForce()
{
    return this->force;
}
void HapticEvent::setForce(const core::Vec3d& deviceForce)
{
    this->force = deviceForce;
}
const core::Vec3d& HapticEvent::getTorque()
{
    return this->torque;
}
void HapticEvent::setTorque(const core::Vec3d& deviceTorque)
{
    this->torque = deviceTorque;
}

} // event namespace
