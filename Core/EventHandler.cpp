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

#include "Core/EventHandler.h"
#include "Core/CoreClass.h"

namespace imstk {

void EventHandler::attachEvent(const EventType& eventType, std::shared_ptr<CoreClass> component)
{
    // Bind handleEvent to a void(std::shared_ptr<Event>) function
    std::function<void(std::shared_ptr<Event>)>
    fn = std::bind(&CoreClass::handleEvent,component,std::placeholders::_1);

    // Register the function and return index
    FunctionContainerType::iterator index = this->registerEvent(eventType, fn);

    // Add index to the component observer
    component->setEventIndex(eventType,index);
}
void EventHandler::detachEvent(const EventType& eventType, std::shared_ptr<CoreClass> component)
{
    auto index = component->getEventIndex(eventType);
    this->unregisterEvent(eventType,index);
    component->removeEventIndex(eventType);
}
bool EventHandler::isAttached(const EventType& eventType, std::shared_ptr<CoreClass> component)
{
    auto index = component->getEventIndex(eventType);

    return this->isAttached(eventType,index);
}
bool EventHandler::isAttached(const EventType& eventType,
                                EventHandler::FunctionContainerType::iterator index)
{
    auto i = observers.find(eventType);

    if(i == std::end(observers) || observers[eventType].size() == 0)
    {
        return false;
    }

    for(auto fn = std::begin(i->second); fn != std::end(i->second); ++fn)
    {
        if(fn == index)
        {
            return true;
        }
    }

    return false;
}

} // core namespace
