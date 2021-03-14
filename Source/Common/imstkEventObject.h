/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkSpinLock.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#define SIGNAL(className,signalName) static std::string signalName() { return #className "::"#signalName; }

namespace imstk
{
class EventObject;

///
/// \class Event
///
/// \brief Base class for events which contain a type, priority, and data
/// priority defaults to 0 and uses a greater than comparator. Negatives are
/// allowed.
///
class Event
{
public:
    Event(const std::string type) : m_type(type),m_sender(nullptr) { }
    virtual~Event() = default;

public:
    std::string  m_type;
    EventObject* m_sender;
};

///
/// \brief Stores everything needed to invoke an event
/// A call may not be present, in which case invoke doesn't do anything
///
class Command
{
public:
    Command() : m_call(nullptr),m_event(nullptr) { }
    Command(std::function<void(Event*)> call,std::shared_ptr<Event> event) : m_call(call),m_event(event) { }

public:
    ///
    /// \brief Call the underlying function if present
    /// then delete the event data
    ///
    void invoke()
    {
        if (m_event != nullptr)
        {
            if (m_call != nullptr)
            {
                m_call(m_event.get());
            }
        }
    }

public:
    std::function<void(Event*)> m_call  = nullptr;
    std::shared_ptr<Event>      m_event = nullptr;
};

template<class T,class RecieverType>
static void connect(EventObject*,std::string(*)(),RecieverType*,void(RecieverType::*)(T*));
template<class T>
static void connect(EventObject*, std::string (*)(), std::function<void(T*)>);

template<class T, class RecieverType>
static void queueConnect(EventObject*, std::string (*)(), RecieverType*, void (RecieverType::*)(T*));
template<class T>
static void queueConnect(EventObject*, std::string (*)(), EventObject*, std::function<void(T*)>);

static void disconnect(EventObject*, EventObject*, std::string (*)());

///
/// \class EventObject
///
/// \brief EventObject is the base class for all objects in iMSTK that
/// can recieve and emit events. It supports direct and queued observer functions.
/// Direct observers recieve events immediately on the same thread
/// This can either be posted on an object or be a function pointer
/// Queued observers recieve events within their queue which they can process whenever
/// they like.
/// These can be connected with the connect/queuedConnect/disconnect functions
/// \todo ThreadObject affinity
///
class EventObject
{
public:
    using Observer = std::pair<EventObject*, std::function<void (Event*)>>;

public:
    virtual ~EventObject() = default;

public:
    ///
    /// \brief Emits the event
    /// Direct observers will be immediately called, in sync
    /// Queued observers will receive the Command in their queue for later
    /// execution, reciever must implement doEvent
    ///
    template<typename T>
    void postEvent(const T& e)
    {
        std::shared_ptr<T> ePtr = std::make_shared<T>(e);
        // Don't overwrite the sender if the user provided one
        if (ePtr->m_sender == nullptr)
        {
            ePtr->m_sender = this;
        }

        // For every direct observer
        // Directly call its function
        for (auto i = directObservers.begin(); i != directObservers.end(); i++)
        {
            if (i->first == e.m_type)
            {
                std::vector<Observer>& observers = i->second;
                for (std::vector<Observer>::iterator j = observers.begin(); j != observers.end(); j++)
                {
                    // If function of observer does not exist, remove observer
                    if (j->second != nullptr)
                    {
                        // Call the function
                        j->second(ePtr.get());
                    }
                    else
                    {
                        j = i->second.erase(j);
                    }
                }
            }
        }

        // For every queued observer
        for (auto i = queuedObservers.begin(); i != queuedObservers.end(); i++)
        {
            if (i->first == e.m_type)
            {
                std::vector<Observer>& observers = i->second;
                for (std::vector<Observer>::iterator j = observers.begin(); j != observers.end(); j++)
                {
                    // As long as the object exists
                    // Push to its queue, otherwise remove observer
                    if (j->first != nullptr)
                    {
                        // Queue the command
                        j->first->eventQueueLock.lock();
                        j->first->eventQueue.push_back(Command(j->second, ePtr));
                        j->first->eventQueueLock.unlock();
                    }
                    else
                    {
                        j = i->second.erase(j);
                    }
                }
            }
        }
    }

    ///
    /// \brief Queues event directly to this
    ///
    template<typename T>
    void queueEvent(const T& e)
    {
        std::shared_ptr<T> ePtr = std::make_shared<T>(e);
        // Don't overwrite the sender if the user provided one
        if (ePtr->m_sender == nullptr)
        {
            ePtr->m_sender = this;
        }

        eventQueueLock.lock();
        eventQueue.push_back(Command(nullptr, ePtr));
        eventQueueLock.unlock();
    }

    ///
    /// \brief Do an event, if none exists return
    ///
    void doEvent()
    {
        // Avoid calling the function within the lock
        eventQueueLock.lock();
        if (eventQueue.empty())
        {
            eventQueueLock.unlock();
            return;
        }

        Command command = eventQueue.front();
        eventQueue.pop_front();

        eventQueueLock.unlock();

        // Do the calls
        command.invoke();
    }

    ///
    /// \brief Do all the events in the event queue
    ///
    void doAllEvents()
    {
        // Avoid calling the function within the lock
        std::list<Command> cmds;
        eventQueueLock.lock();
        {
            while (!eventQueue.empty())
            {
                cmds.push_back(eventQueue.front());
                eventQueue.pop_front();
            }
        }
        eventQueueLock.unlock();

        // Do the calls
        for (auto i : cmds)
        {
            i.invoke();
        }
    }

    ///
    /// \brief Thread safe loop over all event commands, one can implement a custom handler
    ///
    void foreachEvent(std::function<void(Command cmd)> func)
    {
        eventQueueLock.lock();
        for (std::deque<Command>::iterator i = eventQueue.begin(); i != eventQueue.end(); i++)
        {
            func(*i);
        }
        while (!eventQueue.empty())
        {
            Command command = eventQueue.back();
            eventQueue.pop_back();
        }
        eventQueueLock.unlock();
    }

    ///
    /// \brief thread safe reverse loop over all event commands, one can implement a custom handler
    ///
    void rforeachEvent(std::function<void(Command cmd)> func)
    {
        eventQueueLock.lock();
        for (std::deque<Command>::reverse_iterator i = eventQueue.rbegin(); i != eventQueue.rend(); i++)
        {
            func(*i);
        }
        while (!eventQueue.empty())
        {
            Command command = eventQueue.back();
            eventQueue.pop_back();
        }
        eventQueueLock.unlock();
    }

    ///
    /// \brief Removes all events from queue
    /// cleans up copies of the event
    ///
    void clearEvents()
    {
        eventQueueLock.lock();
        {
            while (!eventQueue.empty())
            {
                Command command = eventQueue.back();
                eventQueue.pop_back();
            }
        }
        eventQueueLock.unlock();
    }

public:
    template<class T, class RecieverType>
    friend void connect(EventObject*, std::string (*)(), RecieverType*, void (RecieverType::*)(T*));
    template<typename T>
    friend void connect(EventObject*, std::string (*)(), std::function<void(T*)>);

    template<typename T, class RecieverType>
    friend void queueConnect(EventObject*, std::string (*)(), RecieverType*, void (RecieverType::*)(T*));
    template<class T>
    friend void queueConnect(EventObject*, std::string (*)(), EventObject*, std::function<void(T*)>);

    friend void disconnect(EventObject*, EventObject*, std::string (*)());

// Use the connect functions
private:
    void addDirectObserver(std::string eventType, Observer observer)
    {
        std::vector<std::pair<std::string, std::vector<Observer>>>::iterator i =
            std::find_if(directObservers.begin(), directObservers.end(),
                [eventType](const std::pair<std::string, std::vector<Observer>>& j)
                { return j.first == eventType; });
        if (i == directObservers.end())
        {
            std::pair<std::string, std::vector<Observer>> test = std::pair<std::string, std::vector<Observer>>(eventType, std::vector<Observer>());
            test.second.push_back(observer);
            directObservers.push_back(test);
        }
        else
        {
            i->second.push_back(observer);
        }
    }

    void addQueuedObserver(std::string eventType, Observer observer)
    {
        std::vector<std::pair<std::string, std::vector<Observer>>>::iterator i =
            std::find_if(queuedObservers.begin(), queuedObservers.end(),
                [eventType](const std::pair<std::string, std::vector<Observer>>& j)
                { return j.first == eventType; });
        if (i == queuedObservers.end())
        {
            std::pair<std::string, std::vector<Observer>> test = std::pair<std::string, std::vector<Observer>>(eventType, std::vector<Observer>());
            test.second.push_back(observer);
            queuedObservers.push_back(test);
        }
        else
        {
            i->second.push_back(observer);
        }
    }

protected:
    ParallelUtils::SpinLock eventQueueLock; // Data lock for the event queue
    std::deque<Command>     eventQueue;

    // Vectors used as size is generally small
    std::vector<std::pair<std::string, std::vector<Observer>>> queuedObservers;
    std::vector<std::pair<std::string, std::vector<Observer>>> directObservers;
};

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4505)
#endif
///
/// \brief Direct connection for member functions
///
template<class T, class RecieverType>
static void
connect(EventObject* sender, std::string (* senderFunc)(),
        RecieverType* reciever, void (RecieverType::* recieverFunc)(T*))
{
    static_assert(std::is_base_of<EventObject, RecieverType>::value, "reciever not derived from EventObject");

    std::function<void(T*)> recieverStdFunc = std::bind(recieverFunc, reciever, std::placeholders::_1);
    sender->addDirectObserver(senderFunc(), EventObject::Observer(reciever, [ = ](Event* e) { recieverStdFunc(static_cast<T*>(e)); }));
}

template<class T, class RecieverType>
static void
connect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
        std::shared_ptr<EventObject> reciever, void (RecieverType::* recieverFunc)(T*))
{
    connect<T>(sender.get(), senderFunc, reciever.get(), recieverFunc);
}

template<class T, class RecieverType>
static void
connect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
        RecieverType* reciever, void (RecieverType::* recieverFunc)(T*))
{
    connect<T>(sender.get(), senderFunc, reciever, recieverFunc);
}

template<class T, class RecieverType>
static void
connect(EventObject* sender, std::string (* senderFunc)(),
        std::shared_ptr<RecieverType> reciever, void (RecieverType::* recieverFunc)(T*))
{
    connect<T>(sender, senderFunc, reciever.get(), recieverFunc);
}

///
/// \brief Direct connection for lambda functions
///
template<class T>
static void
connect(EventObject* sender, std::string (* senderFunc)(),
        std::function<void(T*)> recieverFunc)
{
    sender->addDirectObserver(senderFunc(), EventObject::Observer(nullptr, [ = ](Event* e) { recieverFunc(static_cast<T*>(e)); }));
}

template<class T>
static void
connect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
        std::function<void(T*)> recieverFunc)
{
    connect(sender.get(), senderFunc, recieverFunc);
}

///
/// \brief Queued connection for member functions
///
template<class T, class RecieverType>
static void
queueConnect(EventObject* sender, std::string (* senderFunc)(), RecieverType* reciever, void (RecieverType::* recieverFunc)(T*))
{
    // Ensure sender and reciever are EventObjects
    static_assert(std::is_base_of<EventObject, RecieverType>::value, "reciever not derived from EventObject");

    std::function<void(T*)> recieverStdFunc = std::bind(recieverFunc, reciever, std::placeholders::_1);
    sender->addQueuedObserver(senderFunc(), EventObject::Observer(reciever, [ = ](Event* e) { recieverStdFunc(static_cast<T*>(e)); }));
}

template<class T, class RecieverType>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(), std::shared_ptr<RecieverType> reciever, void (RecieverType::* recieverFunc)(T*))
{
    queueConnect<T>(sender.get(), senderFunc, reciever.get(), recieverFunc);
}

template<class T, class RecieverType>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(), RecieverType* reciever, void (RecieverType::* recieverFunc)(T*))
{
    queueConnect<T>(sender.get(), senderFunc, reciever, recieverFunc);
}

template<class T, class RecieverType>
static void
queueConnect(EventObject* sender, std::string (* senderFunc)(), std::shared_ptr<RecieverType> reciever, void (RecieverType::* recieverFunc)(T*))
{
    queueConnect<T>(sender, senderFunc, reciever.get(), recieverFunc);
}

///
/// \brief Queue connection for lambda functions
///
template<class T>
static void
queueConnect(EventObject* sender, std::string (* senderFunc)(), EventObject* reciever, std::function<void(T*)> recieverFunc)
{
    sender->addQueuedObserver(senderFunc(), EventObject::Observer(reciever, [ = ](Event* e) { recieverFunc(static_cast<T*>(e)); }));
}

template<class T>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(), std::shared_ptr<EventObject> reciever, std::function<void(T*)> recieverFunc)
{
    queueConnect(sender.get(), senderFunc, reciever.get(), recieverFunc);
}

template<class T>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(), EventObject* reciever, std::function<void(T*)> recieverFunc)
{
    queueConnect(sender.get(), senderFunc, reciever, recieverFunc);
}

template<class T>
static void
queueConnect(EventObject* sender, std::string (* senderFunc)(), std::shared_ptr<EventObject> reciever, std::function<void(T*)> recieverFunc)
{
    queueConnect(sender, senderFunc, reciever.get(), recieverFunc);
}

///
/// \brief Remove an observer from the sender
/// Note: lambda connections cannot be removed
///
static void
disconnect(EventObject* sender, EventObject* reciever, std::string (* senderFunc)())
{
    const std::string eventType = senderFunc();

    auto i1 = std::find_if(sender->directObservers.begin(), sender->directObservers.end(),
        [eventType](const std::pair<std::string, std::vector<EventObject::Observer>>& j) { return j.first == eventType; });
    if (i1 != sender->directObservers.end())
    {
        auto j = std::find_if(i1->second.begin(), i1->second.end(), [reciever](const EventObject::Observer& j) { return j.first == reciever; });
        i1->second.erase(j);
    }

    auto i2 = std::find_if(sender->queuedObservers.begin(), sender->queuedObservers.end(),
        [eventType](const std::pair<std::string, std::vector<EventObject::Observer>>& j) { return j.first == eventType; });
    if (i2 != sender->queuedObservers.end())
    {
        auto j = std::find_if(i2->second.begin(), i2->second.end(), [reciever](const EventObject::Observer& j) { return j.first == reciever; });
        i2->second.erase(j);
    }
}

static void
disconnect(std::shared_ptr<EventObject> sender, std::shared_ptr<EventObject> reciever, std::string (* senderFunc)())
{
    disconnect(sender.get(), reciever.get(), senderFunc);
}

static void
disconnect(std::shared_ptr<EventObject> sender, EventObject* reciever, std::string (* senderFunc)())
{
    disconnect(sender.get(), reciever, senderFunc);
}

static void
disconnect(EventObject* sender, std::shared_ptr<EventObject> reciever, std::string (* senderFunc)())
{
    disconnect(sender, reciever.get(), senderFunc);
}

#ifdef WIN32
#pragma warning(pop)
#endif
}
