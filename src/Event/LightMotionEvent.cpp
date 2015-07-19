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

#include "LightMotionEvent.h"

namespace mstk {
namespace Event {

EventType smLightMotionEvent::EventName = EventType::LightMotion;

smLightMotionEvent::smLightMotionEvent(const int& lightIndex): index(lightIndex)
{
    position = smVec3d::Zero();
    direction = smVec3d::Zero();
}
void smLightMotionEvent::setPosition(const smVec3d& lightPosition)
{
    this->position = lightPosition;
}
const smVec3d& smLightMotionEvent::getPosition()
{
    return this->position;
}
void smLightMotionEvent::setDirection(const smVec3d& lightDirection)
{
    this->direction = lightDirection;
}
const smVec3d& smLightMotionEvent::getDirection()
{
    return this->direction;
}
void smLightMotionEvent::setLightIndex(const int& lightIndex)
{
    this->index = lightIndex;
}
const int& smLightMotionEvent::getLightIndex()
{
    return this->index;
}

} // Event namespace
} // mstk namespace
