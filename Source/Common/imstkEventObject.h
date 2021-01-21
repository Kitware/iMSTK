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
#include <functional>
#include <list>
#include <memory>
//#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <deque>

namespace imstk
{
enum class EventType
{
    AnyEvent,

    Start,
    End,
    Resume,
    Pause,

    Modified,

    KeyEvent,
    DeviceButtonEvent,
    MouseEvent,

    PreUpdate,
    PostUpdate,
    Configure,

    Contact
};

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
    Event() : m_type(EventType::AnyEvent), m_priority(0), m_sender(nullptr) { }
    Event(EventType type, int priority = 0) : m_type(type), m_priority(priority), m_sender(nullptr) { }
    virtual ~Event() = default;

public:
    bool operator<(const Event& other) const
    {
        return m_priority < other.m_priority;
    }

    bool operator>(const Event& other) const
    {
        return m_priority > other.m_priority;
    }

public:
    EventType    m_type;
    int          m_priority;
    EventObject* m_sender;
};

///
/// \brief Stores everything needed to invoke an event
/// A call may not be present, in which case invoke doesn't do anything
///
class Command
{
public:
    Command() : m_call(nullptr), m_event(nullptr) { }
    Command(std::function<void(Event*)> call, std::shared_ptr<Event> event) : m_call(call), m_event(event) { }

public:
    bool operator<(const Command& other) const
    {
        return *m_event > *other.m_event;
    }

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

template<class T, class RecieverType>
static void connect(EventObject*, EventType, RecieverType*, void (RecieverType::* func)(T*));
template<class T>
static void connect(EventObject*, EventType, std::function<void(T*)>);

template<class T, class RecieverType>
static void queueConnect(EventObject*, EventType, RecieverType*, void (RecieverType::* func)(T*));
template<class T>
static void queueConnect(EventObject*, EventType, EventObject*, std::function<void(T*)>);

static void disconnect(EventObject*, EventObject*, EventType);

///
/// \class EventObject
///
/// \brief EventObject is the base class for all objects in iMSTK that
/// can recieve and emit events. It supports direct and queued observer functions.
/// Direct observers recieve events immediately on the same thread
/// This can either be posted on an object or be a function pointer
/// Queued observers recieve events within their queue which they can process whenever
/// they like. One can implement the type of handling.
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
        for (std::list<Observer>::iterator i = directObservers[e.m_type].begin(); i != directObservers[e.m_type].end(); i++)
        {
            // If function of observer does not exist, remove observer
            if (i->second != nullptr)
            {
                // Call the function
                i->second(ePtr.get());
            }
            else
            {
                i = directObservers[e.m_type].erase(i);
            }
        }

        // For every queued observer
        for (std::list<Observer>::iterator i = queuedObservers[e.m_type].begin(); i != queuedObservers[e.m_type].end(); i++)
        {
            // As long as the object exists
            // Push to its queue, otherwise remove observer
            if (i->first != nullptr)
            {
                // Queue the command
                i->first->eventQueueLock.lock();
                i->first->eventQueue.push_back(Command(i->second, ePtr));
                i->first->eventQueueLock.unlock();
            }
            else
            {
                i = queuedObservers[e.m_type].erase(i);
            }
        }
    }

    ///
    /// \brief Queues event directly to this
    ///
    template<typename T>
    void queueEvent(const T& e)
    {
        T* et = new T(e);
        eventQueueLock.lock();
        eventQueue.push_back(Command(nullptr, et));
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
    friend void connect(EventObject*, EventType, RecieverType*, void (RecieverType::* func)(T*));
    template<typename T>
    friend void connect(EventObject*, EventType, std::function<void(T*)>);

    template<typename T, class RecieverType>
    friend void queueConnect(EventObject*, EventType, RecieverType*, void (RecieverType::* func)(T*));
    template<class T>
    friend void queueConnect(EventObject*, EventType, EventObject*, std::function<void(T*)>);

    friend void disconnect(EventObject*, EventObject*, EventType);

protected:
    ParallelUtils::SpinLock eventQueueLock; // Data lock for the event queue
    std::deque<Command>     eventQueue;
    //tbb::concurrent_priority_queue<Command> eventQueue;
    tbb::concurrent_unordered_map<EventType, std::list<Observer>> queuedObservers;
    tbb::concurrent_unordered_map<EventType, std::list<Observer>> directObservers;
};

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4505)
#endif
///
/// \brief Direct connection
/// When sender emits eventType, func will be called immediately/synchronously/within the same thread
/// Reciever function must be a member of reciever object
///
template<class T, class RecieverType>
static void
connect(EventObject* sender, EventType eventType,
        RecieverType* reciever, void (RecieverType::* func)(T*))
{
    std::function<void(T*)> bindFunc = std::bind(func, reciever, std::placeholders::_1);
    sender->directObservers[eventType].push_back(EventObject::Observer(nullptr, [ = ](Event* e) { bindFunc(static_cast<T*>(e)); }));
}

template<class T, class RecieverType>
static void
connect(std::shared_ptr<EventObject> sender, EventType eventType,
        std::shared_ptr<EventObject> reciever, void (RecieverType::* func)(T*))
{
    connect<T>(sender.get(), eventType, reciever.get(), func);
}

template<class T, class RecieverType>
static void
connect(std::shared_ptr<EventObject> sender, EventType eventType,
        RecieverType* reciever, void (RecieverType::* func)(T*))
{
    connect<T>(sender.get(), eventType, reciever, func);
}

template<class T, class RecieverType>
static void
connect(EventObject* sender, EventType eventType,
        std::shared_ptr<RecieverType> reciever, void (RecieverType::* func)(T*))
{
    connect<T>(sender, eventType, reciever.get(), func);
}

///
/// \brief Direct connection (static, no object required)
/// When sender emits eventType, func will be called immediately/synchronously/within the same thread
///
template<class T>
static void
connect(EventObject* sender, EventType eventType,
        std::function<void(T*)> func)
{
    sender->directObservers[eventType].push_back(EventObject::Observer(nullptr, [ = ](Event* e) { func(static_cast<T*>(e)); }));
}

template<class T>
static void
connect(std::shared_ptr<EventObject> sender, EventType eventType,
        std::function<void(T*)> func)
{
    connect(sender.get(), eventType, func);
}

///
/// \brief Queued connection
/// When sender emits eventType, the function will be queued to reciever in thread safe manner
/// Reciever function must be member of reciever object
///
template<class T, class RecieverType>
static void
queueConnect(EventObject* sender, EventType type, RecieverType* reciever, void (RecieverType::* func)(T*))
{
    // \todo Concept check that the function exists on reciever obj
    std::function<void(T*)> bindFunc = std::bind(func, reciever, std::placeholders::_1);
    sender->queuedObservers[type].push_back(EventObject::Observer(reciever, [ = ](Event* e) { bindFunc(static_cast<T*>(e)); }));
}

template<class T, class RecieverType>
static void
queueConnect(std::shared_ptr<EventObject> sender, EventType type, std::shared_ptr<RecieverType> reciever, void (RecieverType::* func)(T*))
{
    queueConnect<T>(sender.get(), type, reciever.get(), func);
}

template<class T, class RecieverType>
static void
queueConnect(std::shared_ptr<EventObject> sender, EventType type, RecieverType* reciever, void (RecieverType::* func)(T*))
{
    queueConnect<T>(sender.get(), type, reciever, func);
}

template<class T, class RecieverType>
static void
queueConnect(EventObject* sender, EventType type, std::shared_ptr<RecieverType> reciever, void (RecieverType::* func)(T*))
{
    queueConnect<T>(sender, type, reciever.get(), func);
}

template<class T>
static void
queueConnect(EventObject* sender, EventType type, EventObject* reciever, std::function<void(T*)> func)
{
    sender->queuedObservers[type].push_back(EventObject::Observer(reciever, [ = ](Event* e) { func(static_cast<T*>(e)); }));
}

template<class T>
static void
queueConnect(std::shared_ptr<EventObject> sender, EventType type, std::shared_ptr<EventObject> reciever, std::function<void(T*)> func)
{
    queueConnect(sender.get(), type, reciever.get(), func);
}

template<class T>
static void
queueConnect(std::shared_ptr<EventObject> sender, EventType type, EventObject* reciever, std::function<void(T*)> func)
{
    queueConnect(sender.get(), type, reciever, func);
}

template<class T>
static void
queueConnect(EventObject* sender, EventType type, std::shared_ptr<EventObject> reciever, std::function<void(T*)> func)
{
    queueConnect(sender, type, reciever.get(), func);
}

///
/// \brief Remove *ALL* connections of type
/// \todo: It is not possible to tell if a function pointer
/// is equal to another, without an using a handle, its currently impossible
/// to remove a specific one (need to wrap in another object with handle).
///
static void
disconnect(EventObject* sender, EventObject* reciever, EventType eventType)
{
    if (sender->queuedObservers.find(eventType) == sender->queuedObservers.end())
    {
        return;
    }
    std::list<EventObject::Observer>::iterator i =
        std::find_if(sender->queuedObservers[eventType].begin(), sender->queuedObservers[eventType].end(),
            [&](const EventObject::Observer& observer) { return observer.first == reciever; });
    if (i != sender->queuedObservers[eventType].end())
    {
        sender->queuedObservers[eventType].erase(i);
    }
}

static void
disconnect(std::shared_ptr<EventObject> sender, std::shared_ptr<EventObject> reciever, EventType eventType)
{
    disconnect(sender.get(), reciever.get(), eventType);
}

static void
disconnect(std::shared_ptr<EventObject> sender, EventObject* reciever, EventType eventType)
{
    disconnect(sender.get(), reciever, eventType);
}

static void
disconnect(EventObject* sender, std::shared_ptr<EventObject> reciever, EventType eventType)
{
    disconnect(sender, reciever.get(), eventType);
}

#ifdef WIN32
#pragma warning(pop)
#endif
}
