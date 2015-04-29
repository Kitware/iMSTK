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

// SimMedTK includes
#include "smCore/smCoreClass.h"
#include "smEvent/smEventHandler.h"
#include "smEvent/smAudioEvent.h"
#include "smEvent/smCameraEvent.h"
#include "smEvent/smHapticEvent.h"
#include "smEvent/smKeyboardEvent.h"
#include "smEvent/smLightMotionEvent.h"
#include "smEvent/smMouseButtonEvent.h"
#include "smEvent/smMouseMoveEvent.h"
#include "smEvent/smObjectClickedEvent.h"

using namespace bandit;
using namespace smtk::Event;

struct MyTestClass : public smCoreClass
{

    MyTestClass(smtk::Event::EventType _eventType) : success(false), eventType(_eventType) {}

    void handleEvent(std::shared_ptr<smEvent> /*e*/) override
    {
//         std::cout << int(eventType) << std::endl;
        success = true;
    }
    bool success;
    smtk::Event::EventType eventType;
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

            eventHandler->attachEvent(smtk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(smtk::Event::EventType::Audio,observer), IsTrue());

        });
        it("detaches events ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();

            smEvent::Pointer event = std::make_shared<smEvent>();
            smCoreClass::Pointer observer = std::make_shared<smCoreClass>();

            eventHandler->attachEvent(smtk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(smtk::Event::EventType::Audio,observer), IsTrue());
            auto index = observer->getEventIndex(smtk::Event::EventType::Audio);

            eventHandler->detachEvent(smtk::Event::EventType::Audio,observer);

            AssertThat(eventHandler->isAttached(smtk::Event::EventType::Audio,index), IsFalse());
        });
        it("dispatches events ", []() {
            std::shared_ptr<smEventHandler> eventHandler = std::make_shared<smEventHandler>();

            smEvent::Pointer event = std::make_shared<smEvent>();

            std::shared_ptr<MyTestClass> observer;
            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::Audio);
            eventHandler->attachEvent(smtk::Event::EventType::Audio,observer);
            eventHandler->triggerEvent(std::make_shared<smAudioEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::Keyboard);
            eventHandler->attachEvent(smtk::Event::EventType::Keyboard,observer);
            eventHandler->triggerEvent(std::make_shared<smKeyboardEvent>(smKey::A));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::CameraUpdate);
            eventHandler->attachEvent(smtk::Event::EventType::CameraUpdate,observer);
            eventHandler->triggerEvent(std::make_shared<smCameraEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::Haptic);
            eventHandler->attachEvent(smtk::Event::EventType::Haptic,observer);
            eventHandler->triggerEvent(std::make_shared<smHapticEvent>(0,"HapticDevice"));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::LightMotion);
            eventHandler->attachEvent(smtk::Event::EventType::LightMotion,observer);
            eventHandler->triggerEvent(std::make_shared<smLightMotionEvent>(0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::MouseButton);
            eventHandler->attachEvent(smtk::Event::EventType::MouseButton,observer);
            eventHandler->triggerEvent(std::make_shared<smMouseButtonEvent>(smMouseButton::Button0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::MouseMove);
            eventHandler->attachEvent(smtk::Event::EventType::MouseMove,observer);
            eventHandler->triggerEvent(std::make_shared<smMouseMoveEvent>());
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::ObjectClicked);
            eventHandler->attachEvent(smtk::Event::EventType::ObjectClicked,observer);
            eventHandler->triggerEvent(std::make_shared<smObjectClickedEvent>(0));
            AssertThat(observer->success, IsTrue());

            observer = std::make_shared<MyTestClass>(smtk::Event::EventType::None);
            eventHandler->attachEvent(smtk::Event::EventType::None,observer);
            eventHandler->triggerEvent(std::make_shared<smEvent>());
            AssertThat(observer->success, IsTrue());

        });

    });

});

