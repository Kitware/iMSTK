/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#ifndef SMEVENTHANDLER2_H
#define SMEVENTHANDLER2_H

// STL includes
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

// SimMedTK includes
#include "smEvent/smEvent.h"

class smCoreClass;

namespace smtk {
namespace Event {

///
/// @brief Event handler. This class implement a generic observer design
///     pattern using c++11 constructs. The basic idea was taken in part from this
///     blog post:
/// https://juanchopanzacpp.wordpress.com/2013/02/24/simple-observer-pattern-implementation-c11/
///
/// The only requirement is that the observer function to bind has
/// the following signature: void handleEvent(std::shared_ptr<smtk::Event::smEvent> e)
/// This means that anything inheriting from the smCoreClass can be
/// binded to an event.
///
class smEventHandler
{
public:
    ///
    /// @brief Register event and function eventhandler
    /// @param eventName The event name
    /// @param observer Generic observer, this can be a function, functor or lambda
    /// @return index of the registered event
    ///
    template <typename EventMethodType>
    inline size_t registerEvent ( const EventType& eventName, EventMethodType&& observer )
    {
        observers[eventName].emplace_back ( std::forward<EventMethodType> ( observer ) );
        return observers[eventName].size()-1;
    }

    ///
    /// @brief Remove a registered event from the list
    /// @param eventName The event name
    /// @param component Observer component to be unregistered, this commponent holds the key
    ///  to unregister.
    ///
    template <typename ComponentType>
    inline void unregisterEvent ( const EventType& eventName, const ComponentType& component )
    {
        auto i = observers.find ( eventName );
        if ( i == observers.end() ) {
            return;
        }

        // TODO: Implement a more robust un-register based on ComponentType::handleEvent()
        // Something like:
        //  auto functionHandle = i->second.find(ComponentType::handleEvent);
        //  if (functionHandle != i->second.end())
        //      i->second.erase(functionHandle)
        // In order for this to work one will have to pass the handleEvent function directly
        // in attachEvent.
        //
        auto begin = i->second.begin();
        i->second.erase ( begin+component->getEventIndex() );
    }

    ///
    /// @brief Triger all events correspontding to the event name (EventType::EventName).
    /// @param event Event to be triggered, see @smEvent and derived classes.
    ///
    /// @tparam EventObserverType The event that trigges evaluation, child class of smEvent.
    ///
    /// @Note If EventType::EventName is not a key in the event map, then an insertion is automatically performed.
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
    size_t attachEvent ( const EventType& eventType, std::shared_ptr<smCoreClass> component );

    ///
    /// @brief Helper function to facilitate detachment of events.
    /// @param eventType Event name
    /// @param component Listener (or observer) of triggered events to detach
    ///
    void detachEvent ( const EventType& eventType, std::shared_ptr<smCoreClass> component );

private:
    std::map<EventType, std::vector<std::function<void ( std::shared_ptr<smEvent> ) >>> observers; // Container of events
                                                                                                    // to be triggered
};

} // Event namespace
} // smtk namespace

#endif // SMEVENTHANDLER_H
