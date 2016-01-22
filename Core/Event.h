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

#ifndef CORE_EVENT_H
#define CORE_EVENT_H

// STL includes
#include <memory>

namespace core {

enum class EventType
{
    All,
    Terminate,
    ObjectClicked,
    ObjectToObjectHit,
    ObjectToCameraHit,
    Keyboard,
    Sync,
    Haptic,
    CameraUpdate,
    Audio,
    MouseButton,
    MouseMove,
    Reserve,
    LightMotion,
    Other,
    None
};

enum class EventPriority
{
    Lowest,
    Low,
    Normal,
    High,
    Highest
};

enum class EventSender
{
    Module,
    SceneObject,
    EventSource,
    Other
};

///
/// @brief Base event class
/// Base class from which all events should derive.
///
class Event
{
public:
    using Pointer = std::shared_ptr<Event>;

public:
    static EventType EventName;

public:
    Event();
    virtual ~Event();

    void setPriority(const EventPriority &eventPriority);

    const EventPriority &getPriority();

    void setSender(const EventSender &eventSender);

    const EventSender &getSender();

    void setEnabled(const bool &eventEnabled);

    const bool &getEnabled();

private:
    EventPriority priority; // priority of event
    EventSender sender; // sender type
    bool enabled; // allows to disable this event
};

} // core namespace

#endif // CORE_EVENT_H
