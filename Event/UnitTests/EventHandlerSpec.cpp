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

#include <bandit/bandit.h>
#include <memory>

// SimMedTK includes
#include "Core/CoreClass.h"
#include "Core/EventHandler.h"
#include "Event/AudioEvent.h"
#include "Event/CameraEvent.h"
#include "Event/HapticEvent.h"
#include "Event/KeyboardEvent.h"
#include "Event/LightMotionEvent.h"
#include "Event/MouseButtonEvent.h"
#include "Event/MouseMoveEvent.h"
#include "Event/ObjectClickedEvent.h"

using namespace bandit;
using namespace core;
using namespace event;

struct MyObserver : public CoreClass
{
    MyObserver(core::EventType _eventType) : success(false), eventType(_eventType) {}

    void handleEvent(std::shared_ptr<Event> event) override
    {
        std::shared_ptr<KeyboardEvent> keyboardEvent = std::static_pointer_cast<KeyboardEvent>(event);
        if(keyboardEvent != nullptr && KeyboardEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a keyboard event..." << std::endl;
        }
        std::shared_ptr<AudioEvent> audioEvent = std::static_pointer_cast<AudioEvent>(event);
        if(audioEvent != nullptr && AudioEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an audio event..." << std::endl;
        }
        std::shared_ptr<CameraEvent> cameraEvent = std::static_pointer_cast<CameraEvent>(event);
        if(cameraEvent != nullptr && CameraEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a camera event..." << std::endl;
        }
        std::shared_ptr<HapticEvent> hapticEvent = std::static_pointer_cast<HapticEvent>(event);
        if(hapticEvent != nullptr && HapticEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a haptic event..." << std::endl;
        }
        std::shared_ptr<LightMotionEvent> lightEvent = std::static_pointer_cast<LightMotionEvent>(event);
        if(lightEvent != nullptr && LightMotionEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a light event..." << std::endl;
        }
        std::shared_ptr<MouseButtonEvent> mouseButtonEvent = std::static_pointer_cast<MouseButtonEvent>(event);
        if(mouseButtonEvent != nullptr && MouseButtonEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<MouseMoveEvent> mouseMoveEvent = std::static_pointer_cast<MouseMoveEvent>(event);
        if(mouseMoveEvent != nullptr && MouseMoveEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<ObjectClickedEvent> objectClickedEvent = std::static_pointer_cast<ObjectClickedEvent>(event);
        if(objectClickedEvent != nullptr && ObjectClickedEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an object clicked event..." << std::endl;
        }

    }
    bool success;
    core::EventType eventType;
};

go_bandit([](){

    describe("Event handler", []() {
        it("constructs ", []() {
            std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
            AssertThat(eventHandler != nullptr, IsTrue());
        });
        it("attaches events ", []() {
            std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();

            Event::Pointer event = std::make_shared<Event>();
            CoreClass::Pointer observer = std::make_shared<CoreClass>();

            eventHandler->attachEvent(core::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(core::EventType::Audio,observer), IsTrue());

        });
        it("detaches events ", []() {
            std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();

            Event::Pointer event = std::make_shared<Event>();
            CoreClass::Pointer observer = std::make_shared<CoreClass>();

            eventHandler->attachEvent(core::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(core::EventType::Audio,observer), IsTrue());
            auto index = observer->getEventIndex(core::EventType::Audio);

            eventHandler->detachEvent(core::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(core::EventType::Audio,index), IsFalse());
        });
        it("dispatches events ", []() {
            std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();

            Event::Pointer event = std::make_shared<Event>();
            std::shared_ptr<MyObserver> observer;

            observer = std::make_shared<MyObserver>(core::EventType::None);
            eventHandler->attachEvent(core::EventType::None,observer);
            eventHandler->triggerEvent(std::make_shared<Event>());
            AssertThat(observer->success, IsFalse());

            observer = std::make_shared<MyObserver>(core::EventType::Audio);
            eventHandler->attachEvent(core::EventType::Audio,observer);
            eventHandler->triggerEvent(std::make_shared<AudioEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::Keyboard);
            eventHandler->attachEvent(core::EventType::Keyboard,observer);
            eventHandler->triggerEvent(std::make_shared<KeyboardEvent>(Key::A));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::CameraUpdate);
            eventHandler->attachEvent(core::EventType::CameraUpdate,observer);
            eventHandler->triggerEvent(std::make_shared<CameraEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::Haptic);
            eventHandler->attachEvent(core::EventType::Haptic,observer);
            eventHandler->triggerEvent(std::make_shared<HapticEvent>(0,"HapticDevice"));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::LightMotion);
            eventHandler->attachEvent(core::EventType::LightMotion,observer);
            eventHandler->triggerEvent(std::make_shared<LightMotionEvent>(0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::MouseButton);
            eventHandler->attachEvent(core::EventType::MouseButton,observer);
            eventHandler->triggerEvent(std::make_shared<MouseButtonEvent>(MouseButton::Button0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::MouseMove);
            eventHandler->attachEvent(core::EventType::MouseMove,observer);
            eventHandler->triggerEvent(std::make_shared<MouseMoveEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(core::EventType::ObjectClicked);
            eventHandler->attachEvent(core::EventType::ObjectClicked,observer);
            eventHandler->triggerEvent(std::make_shared<ObjectClickedEvent>(0));
            AssertThat(observer->success, IsTrue());

        });

    });

});

