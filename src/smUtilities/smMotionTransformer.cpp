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

// SimMedTK includes
#include "smUtilities/smMotionTransformer.h"

smHapticTrans::smHapticTrans()
{
    newEvent = std::make_shared<smEvent>();
    defaultDirection << 0, 0, -1;
    defaultUpDirection << 0, 1.0, 0;
    dispatch = smSDK::getInstance()->getEventDispatcher();
    motionScale = 1.0;
    enabled = true;
}

void smHapticTrans::setDeviceIdToListen(smInt p_id)
{
    deviceId = p_id;
}

void smHapticTrans::setMotionScale(smFloat p_scale)
{
    motionScale = p_scale;
}

void smHapticTrans::computeTransformation(smMatrix44f& p_mat)
{
    auto mat = p_mat.block<3,3>(0,0);
    transFormedDirection = (mat * defaultDirection);
    transFormedUpDirection = (mat * defaultUpDirection);
}

void smHapticTrans::sendEvent()
{
    dispatch->sendStreamEvent(newEvent);
}

smHapticCameraTrans::smHapticCameraTrans(smInt p_deviceID)
{
    smEventType eventType(SIMMEDTK_EVENTTYPE_HAPTICOUT);
    this->setDeviceId(p_deviceID);
    newEvent->setEventData(std::make_shared<smCameraEventData>());
    newEvent->setEventType(eventType);
    dispatch->registerEventHandler(std::static_pointer_cast<smEventHandler>(safeDownCast<smHapticCameraTrans>()),
                                   eventType);
    offsetAngle_RightDirection = 0;
    offsetAngle_UpDirection = 0;
}

void smHapticCameraTrans::handleEvent(std::shared_ptr<smEvent>  p_event)
{
    if(!enabled)
    {
        return;
    }
    switch(p_event->getEventType().eventTypeCode)
    {
        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        {
            auto hapticEventData = std::static_pointer_cast<smHapticOutEventData>(p_event->getEventData());
            auto cameraEvent = std::static_pointer_cast<smCameraEventData>(newEvent->getEventData());
            if(hapticEventData->deviceId == deviceId)
            {
                cameraEvent->pos = hapticEventData->position * motionScale;
                computeTransformation(hapticEventData->transform);

                cameraEvent->direction = transFormedDirection;
                cameraEvent->upDirection = transFormedUpDirection;
                smVec3f rightVector = transFormedDirection.cross(transFormedUpDirection);
                rightVector.normalize();

                quat = getRotationQuaternion(float(SM_DEGREES2RADIANS(offsetAngle_RightDirection)), rightVector);

                cameraEvent->direction = quat * cameraEvent->direction;
                cameraEvent->upDirection = quat * cameraEvent->upDirection;
                sendEvent();
            }

            break;
        }
    }
}

void smHapticLightTrans::setLightIndex(smInt p_lightIndex)
{
    lightIndex = p_lightIndex;
}

smHapticLightTrans::smHapticLightTrans(smInt p_id)
{
    smEventType eventType(SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE);
    deviceId = p_id;
    lightIndex = 0;
    newEvent->setEventData(std::make_shared<smLightMotionEventData>());
    newEvent->setEventType(eventType);
    dispatch->registerEventHandler(safeDownCast<smEventHandler>(), eventType);
}

void smHapticLightTrans::handleEvent(std::shared_ptr<smEvent> p_event)
{
    if(!enabled)
    {
        return;
    }

    switch(p_event->getEventType().eventTypeCode)
    {
        case SIMMEDTK_EVENTTYPE_HAPTICOUT:

            auto hapticEventData = std::static_pointer_cast<smHapticOutEventData>(p_event->getEventData());
            auto motionEvent = std::static_pointer_cast<smLightMotionEventData>(p_event->getEventData());

            if(hapticEventData->deviceId == deviceId)
            {
                motionEvent->lightIndex = lightIndex;
                motionEvent->pos = hapticEventData->position * motionScale;
                computeTransformation(hapticEventData->transform);
                motionEvent->direction = transFormedDirection;

                sendEvent();

            }

            break;
    }
}
