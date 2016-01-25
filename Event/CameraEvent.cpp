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

#include "Event/CameraEvent.h"

namespace imstk {

EventType CameraEvent::EventName = EventType::CameraUpdate;

CameraEvent::CameraEvent()
{
    position = Vec3d::Zero();
    direction = Vec3d::Zero();
    upDirection = Vec3d::Zero();
}
void CameraEvent::setPosition(const Vec3d& cameraPosition)
{
    this->position = cameraPosition;
}
const Vec3d& CameraEvent::getPosition()
{
    return this->position;
}
void CameraEvent::setDirection(const Vec3d& cameraDirection)
{
    this->direction = cameraDirection;
}
const Vec3d& CameraEvent::getDirection()
{
    return this->direction;
}
void CameraEvent::setUpDirection(const Vec3d& cameraUpDirection)
{
    this->upDirection = cameraUpDirection;
}
const Vec3d& CameraEvent::getUpDirection()
{
    return this->upDirection;
}

} // event namespace
