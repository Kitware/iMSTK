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

#include "Event/LightMotionEvent.h"

namespace event {

core::EventType LightMotionEvent::EventName = core::EventType::LightMotion;

LightMotionEvent::LightMotionEvent(const int& lightIndex): index(lightIndex)
{
    position = core::Vec3d::Zero();
    direction = core::Vec3d::Zero();
}
void LightMotionEvent::setPosition(const core::Vec3d& lightPosition)
{
    this->position = lightPosition;
}
const core::Vec3d& LightMotionEvent::getPosition()
{
    return this->position;
}
void LightMotionEvent::setDirection(const core::Vec3d& lightDirection)
{
    this->direction = lightDirection;
}
const core::Vec3d& LightMotionEvent::getDirection()
{
    return this->direction;
}
void LightMotionEvent::setLightIndex(const int& lightIndex)
{
    this->index = lightIndex;
}
const int& LightMotionEvent::getLightIndex()
{
    return this->index;
}

} // event namespace
