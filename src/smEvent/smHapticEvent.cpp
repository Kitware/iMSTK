/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#include "smEvent/smHapticEvent.h"

namespace smtk {
namespace Event {

constexpr EventType smHapticEvent::EventName;

smHapticEvent::smHapticEvent(const size_t& deviceId, const std::string& deviceName): id(deviceId), name(deviceName)
{}

void smHapticEvent::setPosition(const smVec3d& coordinates)
{
    this->position = coordinates;
}

const smVec3d& smHapticEvent::getPosition()
{
    return this->position;
}
void smHapticEvent::setVelocity(const smVec3d& deviceVelocity)
{
    this->velocity = deviceVelocity;
}
const smVec3d& smHapticEvent::getVelocity()
{
    return this->velocity;
}
void smHapticEvent::setAngles(const smVec3d& deviceAngles)
{
    this->angles = deviceAngles;
}
const smVec3d& smHapticEvent::getAngles()
{
    return this->angles;
}
const smMatrix44d& smHapticEvent::getTransform()
{
    return this->transform;
}
void smHapticEvent::setTransform(const smMatrix44d& deviceTransform)
{
    this->transform = deviceTransform;
}
const smVec3d& smHapticEvent::getForce()
{
    return this->force;
}
void smHapticEvent::setForce(const smVec3d& deviceForce)
{
    this->force = deviceForce;
}
const smVec3d& smHapticEvent::getTorque()
{
    return this->torque;
}
void smHapticEvent::setTorque(const smVec3d& deviceTorque)
{
    this->torque = deviceTorque;
}

} // Event namespace
} // smtk namespace
