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

// STL includes
#include <memory>
#include <atomic>

// SimMedTK includes
#include "smCore/smCoreClass.h"
#include "smCore/smEvent.h"
#include "smCore/smDispatcher.h"

class smEvent;
class smEventType;

struct smEventHolder
{
    std::shared_ptr<smEvent> myEvent;
    smBool isActive;
};

/// \brief  event handler interface. this needs to be inherited by handlers.
class smEventHandler: public smInterface
{

public:
    /// \brief  this function is called for any event
    virtual void handleEvent(std::shared_ptr<smEvent> p_event) = 0;

};

/// \brief event holder
class smEventHandlerHolder
{
public:
    smEventHandlerHolder();

public:
    std::shared_ptr<smEventHandler> handler;
    smEventType  registeredEventType;
    smBool enabled;
};

/// \brief event dispatcher
class smEventDispatcher
{

public:
    /// \brief constructor. Id is initialized to 1.
    smEventDispatcher();

    /// \brief constructor
    smEventDispatcher(std::shared_ptr<smDispatcher> p_dispatcher);

    /// \brief  register event handler
    void registerEventHandler(std::shared_ptr<smEventHandler> handler, const smEventType &p_eventType);

    /// \brief  enable event handler
    void enableEventHandler(std::shared_ptr<smEventHandler> p_handler, const smEventType &p_eventType);

    /// \brief  disable event handler
    void disableEventHandler(std::shared_ptr<smEventHandler> p_handler, const smEventType &p_eventType);

    /// \brief send event in asynchronous mode
    void asyncSendEvent(std::shared_ptr<smEvent> p_event);

    /// \brief send and delete the event
    void sendEventAndDelete(std::shared_ptr<smEvent> p_event);

    /// \brief send stream event. Event is not deleted after sent.
    void sendStreamEvent(std::shared_ptr<smEvent> p_event);

    /// \brief fetch event from the queue
    void fetchEvent();

    /// \brief this calls the handler in synchronization
    void callHandlers(std::shared_ptr<smEvent> p_event);

private:
    std::shared_ptr<smDispatcher> dispatcher; // pointer to the event dispatcher
    std::vector<std::shared_ptr<smEventHandlerHolder>> handlers; // handlers in the dispatcher
    std::atomic_int messageId; // message id counter. used to assign an ID for each message
    smEventHolder eventHolder[SIMMEDTK_MAX_EVENTSBUFFER]; // events holder
};

#endif
