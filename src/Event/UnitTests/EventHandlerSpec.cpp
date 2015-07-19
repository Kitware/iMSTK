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
using namespace mstk::Event;

struct MyObserver : public smCoreClass
{
    MyObserver(mstk::Event::EventType _eventType) : success(false), eventType(_eventType) {}

    void handleEvent(std::shared_ptr<smEvent> event) override
    {
        std::shared_ptr<smKeyboardEvent> keyboardEvent = std::static_pointer_cast<smKeyboardEvent>(event);
        if(keyboardEvent != nullptr && smKeyboardEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a keyboard event..." << std::endl;
        }
        std::shared_ptr<smAudioEvent> audioEvent = std::static_pointer_cast<smAudioEvent>(event);
        if(audioEvent != nullptr && smAudioEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an audio event..." << std::endl;
        }
        std::shared_ptr<smCameraEvent> cameraEvent = std::static_pointer_cast<smCameraEvent>(event);
        if(cameraEvent != nullptr && smCameraEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a camera event..." << std::endl;
        }
        std::shared_ptr<smHapticEvent> hapticEvent = std::static_pointer_cast<smHapticEvent>(event);
        if(hapticEvent != nullptr && smHapticEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a haptic event..." << std::endl;
        }
        std::shared_ptr<smLightMotionEvent> lightEvent = std::static_pointer_cast<smLightMotionEvent>(event);
        if(lightEvent != nullptr && smLightMotionEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a light event..." << std::endl;
        }
        std::shared_ptr<smMouseButtonEvent> mouseButtonEvent = std::static_pointer_cast<smMouseButtonEvent>(event);
        if(mouseButtonEvent != nullptr && smMouseButtonEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<smMouseMoveEvent> mouseMoveEvent = std::static_pointer_cast<smMouseMoveEvent>(event);
        if(mouseMoveEvent != nullptr && smMouseMoveEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its a mouse button event..." << std::endl;
        }
        std::shared_ptr<smObjectClickedEvent> objectClickedEvent = std::static_pointer_cast<smObjectClickedEvent>(event);
        if(objectClickedEvent != nullptr && smObjectClickedEvent::EventName == eventType)
        {
            success = true;
            std::cout << "its an object clicked event..." << std::endl;
        }

    }
    bool success;
    mstk::Event::EventType eventType;
};

go_bandit([](){

    describe("Event handler", []() {
        it("constructs ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();
            AssertThat(eventHandler != nullptr, IsTrue());
        });
        it("attaches events ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();

            smEvent::Pointer event = std::make_shared<smEvent>();
            smCoreClass::Pointer observer = std::make_shared<smCoreClass>();

            eventHandler->attachEvent(mstk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(mstk::Event::EventType::Audio,observer), IsTrue());

        });
        it("detaches events ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();

            smEvent::Pointer event = std::make_shared<smEvent>();
            smCoreClass::Pointer observer = std::make_shared<smCoreClass>();

            eventHandler->attachEvent(mstk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(mstk::Event::EventType::Audio,observer), IsTrue());
            auto index = observer->getEventIndex(mstk::Event::EventType::Audio);

            eventHandler->detachEvent(mstk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(mstk::Event::EventType::Audio,index), IsFalse());
        });
        it("dispatches events ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();

            smEvent::Pointer event = std::make_shared<smEvent>();
            std::shared_ptr<MyObserver> observer;

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::None);
            eventHandler->attachEvent(mstk::Event::EventType::None,observer);
            eventHandler->triggerEvent(std::make_shared<smEvent>());
            AssertThat(observer->success, IsFalse());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::Audio);
            eventHandler->attachEvent(mstk::Event::EventType::Audio,observer);
            eventHandler->triggerEvent(std::make_shared<smAudioEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::Keyboard);
            eventHandler->attachEvent(mstk::Event::EventType::Keyboard,observer);
            eventHandler->triggerEvent(std::make_shared<smKeyboardEvent>(smKey::A));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::CameraUpdate);
            eventHandler->attachEvent(mstk::Event::EventType::CameraUpdate,observer);
            eventHandler->triggerEvent(std::make_shared<smCameraEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::Haptic);
            eventHandler->attachEvent(mstk::Event::EventType::Haptic,observer);
            eventHandler->triggerEvent(std::make_shared<smHapticEvent>(0,"HapticDevice"));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::LightMotion);
            eventHandler->attachEvent(mstk::Event::EventType::LightMotion,observer);
            eventHandler->triggerEvent(std::make_shared<smLightMotionEvent>(0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::MouseButton);
            eventHandler->attachEvent(mstk::Event::EventType::MouseButton,observer);
            eventHandler->triggerEvent(std::make_shared<smMouseButtonEvent>(smMouseButton::Button0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::MouseMove);
            eventHandler->attachEvent(mstk::Event::EventType::MouseMove,observer);
            eventHandler->triggerEvent(std::make_shared<smMouseMoveEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyObserver>(mstk::Event::EventType::ObjectClicked);
            eventHandler->attachEvent(mstk::Event::EventType::ObjectClicked,observer);
            eventHandler->triggerEvent(std::make_shared<smObjectClickedEvent>(0));
            AssertThat(observer->success, IsTrue());

        });

    });

});

