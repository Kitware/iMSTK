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

#include "CameraEvent.h"

namespace event {

core::EventType CameraEvent::EventName = core::EventType::CameraUpdate;

CameraEvent::CameraEvent()
{
    position = core::Vec3d::Zero();
    direction = core::Vec3d::Zero();
    upDirection = core::Vec3d::Zero();
}
void CameraEvent::setPosition(const core::Vec3d& cameraPosition)
{
    this->position = cameraPosition;
}
const core::Vec3d& CameraEvent::getPosition()
{
    return this->position;
}
void CameraEvent::setDirection(const core::Vec3d& cameraDirection)
{
    this->direction = cameraDirection;
}
const core::Vec3d& CameraEvent::getDirection()
{
    return this->direction;
}
void CameraEvent::setUpDirection(const core::Vec3d& cameraUpDirection)
{
    this->upDirection = cameraUpDirection;
}
const core::Vec3d& CameraEvent::getUpDirection()
{
    return this->upDirection;
}

} // event namespace
