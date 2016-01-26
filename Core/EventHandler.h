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

#ifndef CORE_EVENTHANDLER_H
#define CORE_EVENTHANDLER_H

// STL includes
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <list>
#include <algorithm>

// iMSTK includes
#include "Core/Event.h"

namespace imstk {

class CoreClass;

///
/// @brief Event handler. This class implement a generic observer design
///     pattern using c++11 constructs. The basic idea was taken in part from this
///     blog post:
/// https://juanchopanzacpp.wordpress.com/2013/02/24/simple-observer-pattern-implementation-c11/
///
/// The only requirement is that the observer function to bind has
/// the following signature: void handleEvent(std::shared_ptr<Event> e)
/// This means that anything inheriting from the CoreClass can be
/// binded to an event.
///
class EventHandler
{
public:
    using FunctionType = std::function<void ( std::shared_ptr<Event> )>;
    using FunctionContainerType = std::list<FunctionType>;

public:
    ///
    /// @brief Construct and initilize map
    ///
    EventHandler() : observers() {}

    ///
    /// @brief Register event and function eventhandler
    /// @param eventName The event name
    /// @param observer Generic observer, this can be a function, functor or lambda
    /// @return index of the registered event
    ///
    template <typename EventMethodType>
    inline FunctionContainerType::iterator
    registerEvent ( const EventType& eventName, EventMethodType&& observer )
    {
        observers[eventName].emplace_front( std::forward<EventMethodType> ( observer ) );
        return std::begin(observers[eventName]);
    }

    ///
    /// @brief Remove a registered event from the list
    /// @param eventName The event name
    /// @param component Observer component to be unregistered, this commponent holds the key
    ///  to unregister.
    ///
    inline void unregisterEvent( const EventType& eventName, FunctionContainerType::iterator iterator )
    {
        auto i = observers.find ( eventName );
        if ( i == std::end(observers) || observers[eventName].size() == 0)
        {
            return;
        }

        i->second.erase ( iterator );
    }

    ///
    /// @brief Triger all events correspontding to the event name (EventType::EventName).
    /// @param event Event to be triggered, see @Event and derived classes.
    ///
    /// @tparam EventObserverType The event that trigges evaluation, child class of Event.
    ///
    /// @Note If EventType::EventName is not a key in the event map, then an insertion is
    ///     automatically performed.
    ///
    template<typename EventObserverType>
    inline void triggerEvent ( std::shared_ptr<EventObserverType> event )
    {
        for ( const auto& obs : observers[EventObserverType::EventName] )
        {
            obs ( event );
        }
    }

    ///
    /// @brief Helper function to facilitate attachment of events.
    /// @param eventType Event name
    /// @param component Listener (or observer) of triggered events to attach
    ///
    void attachEvent ( const EventType& eventType, std::shared_ptr<CoreClass> component );

    ///
    /// @brief Helper function to facilitate detachment of events.
    /// @param eventType Event name
    /// @param component Listener (or observer) of triggered events to detach
    ///
    void detachEvent ( const EventType& eventType, std::shared_ptr<CoreClass> component );

    ///
    /// @brief Verify if the event has been stored.
    /// @param eventType Event name
    /// @param component Listener (or observer) of triggered events to detach
    ///
    bool isAttached( const EventType& eventType, std::shared_ptr<CoreClass> component );

    ///
    /// @brief Verify if the event has been stored.
    /// @param eventType Event name
    /// @param index the index of the function to query
    ///
    bool isAttached( const EventType& eventType, FunctionContainerType::iterator index );

private:
    std::map<EventType, FunctionContainerType> observers; // Container of events
                                                          // to be triggered
};

} // core namespace

#endif // CORE_EVENTHANDLER_H
