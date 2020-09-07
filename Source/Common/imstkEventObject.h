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

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
//#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

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
    KeyPress,
    VRButtonPress,
    MouseEvent,
    PreUpdate,
    PostUpdate,
    Configure
    //?? add one for collision?
};

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
    Event() : m_type(EventType::AnyEvent), m_priority(0) { }
    Event(EventType type, int priority = 0) : m_type(type), m_priority(priority) { }
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
    EventType m_type;
    int       m_priority;
};

///
/// \brief Stores everything needed to invoke an event
/// ie: where and with what
///
class Command
{
public:
    Command() : m_call(nullptr), m_event(nullptr) { }
    Command(std::function<void(Event*)> call, Event* event) : m_call(call), m_event(event) { }

public:
    bool operator<(const Command& other) const
    {
        return *m_event > *other.m_event;
    }

public:
    std::function<void(Event*)> m_call;
    Event* m_event;
};

class EventObject;

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
/// \class imstkEventObject
///
/// \brief imstkEventObject is the base abstract class for all objects in iMSTK that
/// can recieve and emit events. It supports direct and queued observer functions.
/// Direct observers recieve events immediately on the same thread
/// This can either be posted on an object or be a function pointer
/// Queued observers recieve events within their queue
/// These can be connected with the connect function
/// \todo If objects are given affinity, we can automatically decide whether it could
/// be dangerous to make a certain type of connection
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
    /// Direct observers will be immediately called
    /// Queued observers will receive the EventFunctionPair in their queue
    /// That is, the function to call and data of the event
    /// The event will be copied before sent, and cleaned up after handled
    /// to allow for polymorphic event types
    ///
    template<typename T>
    void emit(const T& e)
    {
        // For every direct observer
        for (std::list<Observer>::iterator i = directObservers[e.m_type].begin(); i != directObservers[e.m_type].end(); i++)
        {
            // We don't need an object (i->first) to call
            // Call its function, if it exists, otherwise remove observer
            if (i->second != nullptr)
            {
                i->second(new T(e));
            }
            else
            {
                i = directObservers[e.m_type].erase(i);
            }
        }

        // For every queued observer
        for (std::list<Observer>::iterator i = queuedObservers[e.m_type].begin(); i != queuedObservers[e.m_type].end(); i++)
        {
            // Both object and callback function must be present
            // Push to its queue, if it exists, otherwise remote observer
            if (i->first != nullptr && i->second != nullptr)
            {
                i->first->eventQueue.push(Command(i->second, new T(e)));
            }
            else
            {
                i = queuedObservers[e.m_type].erase(i);
            }
        }
    }

    ///
    /// \brief Try to do an event from the priority queue
    ///
    void doEvent()
    {
        Command command;
        if (eventQueue.try_pop(command))
        {
            command.m_call(command.m_event);
            delete command.m_event;
        }
    }

    ///
    /// \brief Not thread safe, could run forever if events are constantly
    /// being posted
    /// \todo: need lock
    ///
    void doAllEvents()
    {
        while (!eventQueue.empty())
        {
            doEvent();
        }
        eventQueue.clear();
    }

    ///
    /// \brief Pops all events and does the last one
    ///
    void doLastEvent()
    {
        Command command;
        while (!eventQueue.empty())
        {
            eventQueue.try_pop(command);
        }
        if (command.m_call != nullptr)
        {
            command.m_call(command.m_event);
            delete command.m_event;
            eventQueue.clear();
        }
    }

protected:
    ///
    /// \biref Constructor, object must be subclassed
    ///
    EventObject() = default;

protected:
    ///
    /// \brief Removes all events from queue
    /// cleans up copies of the event
    ///
    void clearEvents()
    {
        // Despite empty not being thread safe, it will
        // still not be able to try_pop, which will only
        // cause a short delay, whilst another thread is
        // performing an push/pop from the queue
        while (!eventQueue.empty())
        {
            Command command;
            if (eventQueue.try_pop(command))
            {
                delete command.m_event;
            }
        }
        eventQueue.clear();
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

private:
    tbb::concurrent_queue<Command> eventQueue;
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
/// When sender emits eventType, func will be queued to reciever in thread safe manner
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
/// \brief Remove all connections of type
/// \todo: It is not possible to tell if a function pointer
/// is equal to another, without an using a handle, its impossible
/// to remove
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
