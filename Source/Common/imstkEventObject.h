/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
static void connect(std::shared_ptr<EventObject>,std::string(*)(),
                    std::shared_ptr<RecieverType>,void(RecieverType::*)(T*));
template<class T>
static void connect(std::shared_ptr<EventObject>, std::string (*)(),
                    std::function<void(T*)>);

template<class T, class RecieverType>
static void queueConnect(std::shared_ptr<EventObject>, std::string (*)(),
                         std::shared_ptr<RecieverType>, void (RecieverType::*)(T*));
template<class T>
static void queueConnect(std::shared_ptr<EventObject>, std::string (*)(),
                         std::shared_ptr<EventObject>, std::function<void(T*)>);

static void disconnect(std::shared_ptr<EventObject>,
                       std::shared_ptr<EventObject>, std::string (*)());

///
/// \class EventObject
///
/// \brief EventObject is the base class for all objects in iMSTK that
/// can receive and emit events. It supports direct and queued observer functions.
/// Direct observers receive events immediately on the same thread
/// This can either be posted on an object or be a function pointer
/// Queued observers receive events within their queue which they can process whenever
/// they like.
/// These can be connected with the connect/queuedConnect/disconnect functions
/// Lambda recievers cannot be disconnected unless all receivers to a signal are removed
/// \todo ThreadObject affinity
///
class EventObject
{
public:
    // tuple<IsLambda, Receiver, Receiving Function
    using Observer = std::tuple<bool, std::weak_ptr<EventObject>, std::function<void (Event*)>>;

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
                for (std::vector<Observer>::iterator j = observers.begin(); j != observers.end();)
                {
                    bool                        isLambda      = std::get<0>(*j);
                    std::function<void(Event*)> receivingFunc = std::get<2>(*j);

                    // If the receiver or receiving function is nullptr, cleanup
                    // This would occur on deconstruction of a receiver
                    if ((!isLambda && std::get<1>(*j).expired()) || receivingFunc == nullptr)
                    {
                        j = i->second.erase(j);
                    }
                    else
                    {
                        // Call the receiving function
                        receivingFunc(ePtr.get());
                        j++;
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
                for (std::vector<Observer>::iterator j = observers.begin(); j != observers.end();)
                {
                    bool                        isLambda      = std::get<0>(*j);
                    std::function<void(Event*)> receivingFunc = std::get<2>(*j);

                    // If the receiver or receiving function is nullptr, cleanup
                    // This would occur on deconstruction of a receiver
                    if ((!isLambda && std::get<1>(*j).expired()) || receivingFunc == nullptr)
                    {
                        j = i->second.erase(j);
                    }
                    else
                    {
                        // Queue the command
                        std::shared_ptr<EventObject> receivingObj = std::get<1>(*j).lock();
                        receivingObj->eventQueueLock.lock();
                        receivingObj->eventQueue.push_back(Command(receivingFunc, ePtr));
                        receivingObj->eventQueueLock.unlock();
                        j++;
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
    friend void connect(std::shared_ptr<EventObject>, std::string (*)(),
                        std::shared_ptr<RecieverType>, void (RecieverType::*)(T*));
    template<typename T>
    friend void connect(std::shared_ptr<EventObject>,
                        std::string (*)(), std::function<void(T*)>);

    template<typename T, class RecieverType>
    friend void queueConnect(std::shared_ptr<EventObject>, std::string (*)(),
                             std::shared_ptr<RecieverType>, void (RecieverType::*)(T*));
    template<class T>
    friend void queueConnect(std::shared_ptr<EventObject>, std::string (*)(),
                             std::shared_ptr<EventObject>, std::function<void(T*)>);

    friend void disconnect(std::shared_ptr<EventObject>,
                           std::shared_ptr<EventObject>, std::string (*)());

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
/// \brief Direct connection for member functions. When sender invokes senderFunc
/// through postEvent, the receiverFunc is called.
///
/// \param sender The object sending the signal
/// \param senderFunc The function called to send
/// \param receiver The object receiving the signal
/// \param receiver The function called when it is sent
///
template<class T, class ReceiverType>
static void
connect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
        std::shared_ptr<ReceiverType> receiver, void (ReceiverType::* receiverFunc)(T*))
{
    static_assert(std::is_base_of<EventObject, ReceiverType>::value, "receiver not derived from EventObject");

    std::function<void(T*)> receiverStdFunc = std::bind(receiverFunc, receiver.get(), std::placeholders::_1);
    sender->addDirectObserver(senderFunc(), EventObject::Observer(false, receiver, [ = ](Event* e) { receiverStdFunc(static_cast<T*>(e)); }));
}

///
/// \brief Direct connection for lambda functions. When sender invokes senderFunc
/// through postEvent, the receiverFunc is called.
/// Note: Lambda's do not contain any ids or equivalence, the only way to disconnect this event
/// is to disconnect all receivers of a given signal.
///
/// \param sender The object sending the signal
/// \param senderFunc The function called to send
/// \param receiver The function called when it is sent
///
template<class T>
static void
connect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
        std::function<void(T*)> receiverFunc)
{
    sender->addDirectObserver(senderFunc(), EventObject::Observer(true,
        std::weak_ptr<EventObject>(), [ = ](Event* e) { receiverFunc(static_cast<T*>(e)); }));
}

///
/// \brief Queued connection for member functions. When sender invokes senderFunc
/// through postEvent, the receiverFunc is placed in receivers queue of events.
/// Call doAllEvents to invoke in your own time. Good for multithreaded scenarios.
///
/// \param sender The object sending the signal
/// \param senderFunc The function called to send to receivers queue
/// \param receiver The object receiving the signal in its queue
/// \param receiver The function called when it is invoked from queue
///
template<class T, class ReceiverType>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
             std::shared_ptr<ReceiverType> receiver, void (ReceiverType::* recieverFunc)(T*))
{
    // Ensure sender and reciever are EventObjects
    static_assert(std::is_base_of<EventObject, ReceiverType>::value, "receiver not derived from EventObject");

    std::function<void(T*)> recieverStdFunc = std::bind(recieverFunc, receiver.get(), std::placeholders::_1);
    sender->addQueuedObserver(senderFunc(), EventObject::Observer(false, receiver, [ = ](Event* e) { recieverStdFunc(static_cast<T*>(e)); }));
}

///
/// \brief Queued connection for member functions. When sender invokes senderFunc
/// through postEvent, the receiverFunc is placed in receivers queue of events.
/// Call doAllEvents to invoke in your own time. Good for multithreaded scenarios.
/// The difference here is you can use a lambda, that does not need to exist on the
/// receiver object.
/// To disconnect, disconnect all receivers from signal or clear/delete the receiver
///
/// \param sender The object sending the signal
/// \param senderFunc The function called to send to receivers queue
/// \param receiver The object receiving the signal in its queue
/// \param receiver The function called when it is invoked from queue
///
template<class T>
static void
queueConnect(std::shared_ptr<EventObject> sender, std::string (* senderFunc)(),
             std::shared_ptr<EventObject> receiver, std::function<void(T*)> recieverFunc)
{
    sender->addQueuedObserver(senderFunc(), EventObject::Observer(true, receiver, [ = ](Event* e) { recieverFunc(static_cast<T*>(e)); }));
}

///
/// \brief Remove an observer from the sender
///
/// Note: Direct lambda connections cannot be removed
/// Note: Specific sender, senderFunc, receiver, receiverFunc cannot be removed because
/// the recieverFunc cannot be id'd
///
/// \param sender The sender to remove on
/// \param receiver The receiver to remove on
/// \param senderFunc the signal for which all receivers will not receive
///
static void
disconnect(std::shared_ptr<EventObject> sender,
           std::shared_ptr<EventObject> reciever, std::string (* senderFunc)())
{
    const std::string eventType = senderFunc();

    auto i1 = std::find_if(sender->directObservers.begin(), sender->directObservers.end(),
        [eventType](const std::pair<std::string, std::vector<EventObject::Observer>>& j) { return j.first == eventType; });
    if (i1 != sender->directObservers.end())
    {
        auto j = std::find_if(i1->second.begin(), i1->second.end(), [reciever](const EventObject::Observer& k) { return std::get<1>(k).lock() == reciever; });
        i1->second.erase(j);
    }

    auto i2 = std::find_if(sender->queuedObservers.begin(), sender->queuedObservers.end(),
        [eventType](const std::pair<std::string, std::vector<EventObject::Observer>>& j) { return j.first == eventType; });
    if (i2 != sender->queuedObservers.end())
    {
        auto j = std::find_if(i2->second.begin(), i2->second.end(), [reciever](const EventObject::Observer& k) { return std::get<1>(k).lock() == reciever; });
        i2->second.erase(j);
    }
}

#ifdef WIN32
#pragma warning(pop)
#endif
} // namespace imstk
