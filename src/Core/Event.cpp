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

#include "Event.h"

namespace core {

EventType Event::EventName = EventType::None;

Event::Event(): priority(EventPriority::Normal), sender(EventSender::Other), enabled(true) {}

Event::~Event() {}

void Event::setPriority(const EventPriority& eventPriority)
{
    this->priority = eventPriority;
}
const EventPriority& Event::getPriority()
{
    return this->priority;
}
void Event::setSender(const EventSender& eventSender)
{
    this->sender = eventSender;
}
const EventSender& Event::getSender()
{
    return this->sender;
}
void Event::setEnabled(const bool& eventEnabled)
{
    this->enabled = eventEnabled;
}
const bool& Event::getEnabled()
{
    return this->enabled;
}

} // core namespace

