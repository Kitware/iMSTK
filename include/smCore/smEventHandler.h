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
 
#ifndef SMEVENTHANDLER_H
#define SMEVENTHANDLER_H

#include "smCore/smCoreClass.h"
#include "smCore/smEvent.h"
#include "smCore/smDispatcher.h"

#include <atomic>

class smEvent;
class smEventType;

struct smEventHolder
{
    smEvent *myEvent;
    smBool isActive;
};
/// \brief  event handler interface. this needs to be inherited by handlers.
class smEventHandler: smInterface
{

public:
    /// \brief  this function is called for any event
    virtual void handleEvent(smEvent *p_event) = 0;

};

/// \brief event holder
class smEventHandlerHolder
{

public:
    smEventHandlerHolder()
    {
        enabled = true;
        handler = NULL;
        registeredEventType = SIMMEDTK_EVENTTYPE_NONE;
    }
    smEventHandler *handler;
    smEventType  registeredEventType;
    smBool enabled;
};
/// \brief event dispatcher
class smEventDispatcher
{

private:
    /// \brief pointer to the event dispatcher
    smDispatcher *dispatcher;
    /// \brief handlers in the dispatcher
    vector<smEventHandlerHolder*>handlers;
    /// \brief message id counter. used to assign an ID for each message
    std::atomic_int messageId;
    /// \brief event holder. holds SIMMEDTK_MAX_EVENTSBUFFER of number holder
    smEventHolder eventHolder[SIMMEDTK_MAX_EVENTSBUFFER];

public:
    /// \brief constructor. Id is initialized to 1.
    smEventDispatcher()
    {
        messageId = 1;
    }
    /// \brief constructor
    smEventDispatcher(smDispatcher *p_dispatcher);
    /// \brief  register event handler
    void registerEventHandler(smEventHandler *handler, smEventType p_eventType);
    /// \brief  enable event handler
    void enableEventHandler(smEventHandler *p_handler, smEventType p_eventType);
    /// \brief  disable event handler
    void disableEventHandler(smEventHandler *p_handler, smEventType p_eventType);
    /// \brief send event in asynchronous mode
    void asyncSendEvent(smEvent *p_event);
    /// \brief send and delete the event
    void sendEventAndDelete(smEvent *p_event);
    /// \brief send stream event. Event is not deleted after sent.
    void sendStreamEvent(smEvent *p_event);
    /// \brief fetch event from the queue
    void fetchEvent();
    /// \brief this calls the handler in synchronization
    inline void callHandlers(smEvent *p_event)
    {
        int v = handlers.size();

        for (smInt i = 0; i < v; i++)
        {
            if (handlers[i]->enabled && p_event->eventType == handlers[i]->registeredEventType ||
                    handlers[i]->registeredEventType == SIMMEDTK_EVENTTYPE_ALL)
            {
                handlers[i]->handler->handleEvent(p_event);
            }
        }
    }
};

#endif
