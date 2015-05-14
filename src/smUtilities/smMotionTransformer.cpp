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

#include "smUtilities/smMotionTransformer.h"

// SimMedTK includes
#include "smEvent/smEvent.h"
#include "smEvent/smEventHandler.h"
#include "smEvent/smCameraEvent.h"
#include "smEvent/smHapticEvent.h"
#include "smEvent/smLightMotionEvent.h"
#include "smUtilities/smMatrix.h"

smHapticTrans::smHapticTrans()
{
    defaultDirection = -smVec3d::UnitZ();
    defaultUpDirection = smVec3d::UnitY();
    if(nullptr == eventHandler)
        eventHandler = std::make_shared<smtk::Event::smEventHandler>();
    motionScale = 1.0;
    this->listening = true;
}

void smHapticTrans::setDeviceIdToListen(const size_t &p_id)
{
    deviceId = p_id;
}

void smHapticTrans::setMotionScale(const float &p_scale)
{
    motionScale = p_scale;
}

const size_t &smHapticTrans::getDeviceId()
{
    return deviceId;
}
void smHapticTrans::setDeviceId(const size_t &id)
{
    deviceId = id;
}

std::shared_ptr< smtk::Event::smEventHandler > smHapticTrans::getEventHandler()
{
    return eventHandler;
}
void smHapticTrans::setEventHandler(std::shared_ptr< smtk::Event::smEventHandler > cameraEventHandler)
{
    eventHandler = cameraEventHandler;
}
smHapticCameraTrans::smHapticCameraTrans(const size_t &p_deviceID)
{
    this->deviceId = p_deviceID;

    offsetAngle_RightDirection = 0;
    offsetAngle_UpDirection = 0;

    // TODO: Detach any events previously attached

    // Update event event handler
    eventHandler->attachEvent(smtk::Event::EventType::CameraUpdate,shared_from_this());
}

void smHapticCameraTrans::handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event)
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<smtk::Event::smHapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == deviceId)
    {
        auto cameraEvent = std::make_shared<smtk::Event::smCameraEvent>();
        cameraEvent->setPosition(motionScale*hapticEvent->getPosition());

        smMatrix33d transformMatrix = hapticEvent->getTransform().block<3,3>(0,0);

        cameraEvent->setDirection(transformMatrix*this->defaultDirection);
        cameraEvent->setUpDirection(transformMatrix*this->defaultUpDirection);

        auto rotationAxis = cameraEvent->getDirection().cross(
                cameraEvent->getUpDirection()).normalized();

        auto quat = getRotationQuaternion(offsetAngle_RightDirection,rotationAxis);

        cameraEvent->applyRotation(quat);

        this->eventHandler->triggerEvent(cameraEvent);
    }
}

void smHapticLightTrans::setLightIndex(const size_t &p_lightIndex)
{
    lightIndex = p_lightIndex;
}

smHapticLightTrans::smHapticLightTrans(const size_t &p_id)
{
    this->deviceId = p_id;

    lightIndex = 0;

    eventHandler->attachEvent(smtk::Event::EventType::LightMotion,shared_from_this());
}

void smHapticLightTrans::handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event)
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<smtk::Event::smHapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == deviceId)
    {
        auto motionEvent = std::make_shared<smtk::Event::smLightMotionEvent>(lightIndex);
        motionEvent->setPosition(motionScale*hapticEvent->getPosition());

        auto transformMatrix = hapticEvent->getTransform().block<3,3>(0,0);

        motionEvent->setDirection(transformMatrix*this->defaultDirection);

        this->eventHandler->triggerEvent(motionEvent);
    }
}

