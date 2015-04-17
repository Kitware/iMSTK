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

#include "smCore/smEventHandler.h"
#include "smCore/smEvent.h"

///calls all the Event Handlers and then Delete the event
void smEventDispatcher::registerEventHandler(std::shared_ptr<smEventHandler> p_handler,
        const smEventType &p_eventType)
{
    std::shared_ptr<smEventHandlerHolder> holder = std::make_shared<smEventHandlerHolder>();
    holder->handler = p_handler;
    holder->registeredEventType = p_eventType;
    handlers.push_back(holder);
}

void smEventDispatcher::disableEventHandler(std::shared_ptr<smEventHandler> p_handler,
        const smEventType &p_eventType)
{
    for (size_t i = 0; i < handlers.size(); i++)
    {
        if (handlers[i]->handler == p_handler && handlers[i]->registeredEventType == p_eventType)
        {
            handlers[i]->enabled = false;
        }
    }
}

void smEventDispatcher::enableEventHandler(std::shared_ptr<smEventHandler> p_handler,
        const smEventType &p_eventType)
{

    for (size_t i = 0; i < handlers.size(); i++)
    {
        if (handlers[i]->handler == p_handler && handlers[i]->registeredEventType == p_eventType)
        {
            handlers[i]->enabled = true;
        }
    }
}

///Synchronous Event calling..Be aware that the calling of the functions are not serialized.
///The serialization should be done within the handler function.
void smEventDispatcher::sendStreamEvent(std::shared_ptr<smEvent> p_event)
{
    callHandlers(p_event);
}

void smEventDispatcher::sendEventAndDelete(std::shared_ptr<smEvent> p_event)
{
    callHandlers(p_event);
    p_event.reset();
}

///asynchronous Event calling
///not implemented yet
void smEventDispatcher::asyncSendEvent(std::shared_ptr<smEvent> /*p_event*/)
{

}

void smEventDispatcher::callHandlers(std::shared_ptr<smEvent> p_event)
{
    int v = handlers.size();

    for ( smInt i = 0; i < v; i++ )
    {
        if ( (handlers[i]->enabled && p_event->eventType == handlers[i]->registeredEventType) ||
                handlers[i]->registeredEventType == SIMMEDTK_EVENTTYPE_ALL )
        {
            handlers[i]->handler->handleEvent( p_event );
        }
    }
}
smEventHandlerHolder::smEventHandlerHolder()
{
    enabled = true;
    handler.reset();
    registeredEventType = SIMMEDTK_EVENTTYPE_NONE;
}
smEventDispatcher::smEventDispatcher()
{
    messageId = 1;
}
