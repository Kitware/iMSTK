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

#include <bandit/bandit.h>
#include <memory>

// iMSTK includes
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

struct MyObserver : public imstk::CoreClass
{
    MyObserver(imstk::EventType _eventType) : success(false), eventType(_eventType) {}

    void handleEvent(std::shared_ptr<imstk::Event> event) override
    {
        std::shared_ptr<imstk::KeyboardEvent> keyboardEvent = std::static_pointer_cast<imstk::KeyboardEvent>(event);
        if(keyboardEvent != nullptr && imstk::KeyboardEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a keyboard event..." << std::endl;
        }
        std::shared_ptr<imstk::AudioEvent> audioEvent = std::static_pointer_cast<imstk::AudioEvent>(event);
        if(audioEvent != nullptr && imstk::AudioEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an audio event..." << std::endl;
        }
        std::shared_ptr<imstk::CameraEvent> cameraEvent = std::static_pointer_cast<imstk::CameraEvent>(event);
        if(cameraEvent != nullptr && imstk::CameraEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a camera event..." << std::endl;
        }
        std::shared_ptr<imstk::HapticEvent> hapticEvent = std::static_pointer_cast<imstk::HapticEvent>(event);
        if(hapticEvent != nullptr && imstk::HapticEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a haptic event..." << std::endl;
        }
        std::shared_ptr<imstk::LightMotionEvent> lightEvent = std::static_pointer_cast<imstk::LightMotionEvent>(event);
        if(lightEvent != nullptr && imstk::LightMotionEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a light event..." << std::endl;
        }
        std::shared_ptr<imstk::MouseButtonEvent> mouseButtonEvent = std::static_pointer_cast<imstk::MouseButtonEvent>(event);
        if(mouseButtonEvent != nullptr && imstk::MouseButtonEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<imstk::MouseMoveEvent> mouseMoveEvent = std::static_pointer_cast<imstk::MouseMoveEvent>(event);
        if(mouseMoveEvent != nullptr && imstk::MouseMoveEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<imstk::ObjectClickedEvent> objectClickedEvent = std::static_pointer_cast<imstk::ObjectClickedEvent>(event);
        if(objectClickedEvent != nullptr && imstk::ObjectClickedEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an object clicked event..." << std::endl;
        }

    }
    bool success;
    imstk::EventType eventType;
};

go_bandit([](){

    describe("Event handler", []() {
        it("constructs ", []() {
            auto eventHandler = std::make_shared<imstk::EventHandler>();
            AssertThat(eventHandler != nullptr, IsTrue());
        });
        it("attaches events ", []() {
            std::shared_ptr<imstk::EventHandler> eventHandler = std::make_shared<imstk::EventHandler>();

            auto event = std::make_shared<imstk::Event>();
            auto observer = std::make_shared<imstk::CoreClass>();

            eventHandler->attachEvent(imstk::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(imstk::EventType::Audio,observer), IsTrue());

        });
        it("detaches events ", []() {
            std::shared_ptr<imstk::EventHandler> eventHandler = std::make_shared<imstk::EventHandler>();

            imstk::Event::Pointer event = std::make_shared<imstk::Event>();
            imstk::CoreClass::Pointer observer = std::make_shared<imstk::CoreClass>();

            eventHandler->attachEvent(imstk::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(imstk::EventType::Audio,observer), IsTrue());
            auto index = observer->getEventIndex(imstk::EventType::Audio);

            eventHandler->detachEvent(imstk::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(imstk::EventType::Audio,index), IsFalse());
        });
        it("dispatches events ", []() {
            std::shared_ptr<imstk::EventHandler> eventHandler = std::make_shared<imstk::EventHandler>();

            imstk::Event::Pointer event = std::make_shared<imstk::Event>();
            std::shared_ptr<MyObserver> observer;

            observer = std::make_shared<MyObserver>(imstk::EventType::None);
            eventHandler->attachEvent(imstk::EventType::None,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::Event>());
            AssertThat(observer->success, IsFalse());

            observer = std::make_shared<MyObserver>(imstk::EventType::Audio);
            eventHandler->attachEvent(imstk::EventType::Audio,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::AudioEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::Keyboard);
            eventHandler->attachEvent(imstk::EventType::Keyboard,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::KeyboardEvent>(imstk::Key::A));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::CameraUpdate);
            eventHandler->attachEvent(imstk::EventType::CameraUpdate,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::CameraEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::Haptic);
            eventHandler->attachEvent(imstk::EventType::Haptic,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::HapticEvent>(0,"HapticDevice"));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::LightMotion);
            eventHandler->attachEvent(imstk::EventType::LightMotion,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::LightMotionEvent>(0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::MouseButton);
            eventHandler->attachEvent(imstk::EventType::MouseButton,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::MouseButtonEvent>(imstk::MouseButton::Button0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::MouseMove);
            eventHandler->attachEvent(imstk::EventType::MouseMove,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::MouseMoveEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(imstk::EventType::ObjectClicked);
            eventHandler->attachEvent(imstk::EventType::ObjectClicked,observer);
            eventHandler->triggerEvent(std::make_shared<imstk::ObjectClickedEvent>(0));
            AssertThat(observer->success, IsTrue());

        });

    });

});

