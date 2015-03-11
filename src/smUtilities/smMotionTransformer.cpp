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
    newEvent = new smEvent();
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
    static smMatrix33f mat;
    mat = p_mat.block<3,3>(0,0);
    transFormedDirection = (mat * defaultDirection);
    transFormedUpDirection = (mat * defaultUpDirection);
}
void smHapticTrans::sendEvent()
{
    dispatch->sendStreamEvent(newEvent);
}
smHapticCameraTrans::smHapticCameraTrans(smInt p_deviceID)
{
    deviceId = p_deviceID;
    newEvent->data = new smCameraEventData();
    newEvent->eventType = SIMMEDTK_EVENTTYPE_CAMERA_UPDATE;
    dispatch->registerEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
    offsetAngle_RightDirection = 0;
    offsetAngle_UpDirection = 0;
}
void smHapticCameraTrans::handleEvent(smEvent* p_event)
{
    smHapticOutEventData *hapticEventData;
    smVec3f rightVector;

    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        if (!enabled)
        {
            return;
        }

        hapticEventData = (smHapticOutEventData *)p_event->data;

        if (hapticEventData->deviceId == deviceId)
        {
            ((smCameraEventData*)newEvent->data)->pos = hapticEventData->position * motionScale;
            computeTransformation(hapticEventData->transform);

            ((smCameraEventData*)newEvent->data)->direction = transFormedDirection;
            ((smCameraEventData*)newEvent->data)->upDirection = transFormedUpDirection;
            rightVector = transFormedDirection.cross(transFormedUpDirection);
            rightVector.normalize();

            quat = getRotationQuaternion(float(SM_DEGREES2RADIANS(offsetAngle_RightDirection)), rightVector);

            ((smCameraEventData*)newEvent->data)->direction = quat*((smCameraEventData*)newEvent->data)->direction;
            ((smCameraEventData*)newEvent->data)->upDirection = quat*((smCameraEventData*)newEvent->data)->upDirection;
            sendEvent();
        }

        break;
    }
}
void smHapticLightTrans::setLightIndex(smInt p_lightIndex)
{
    lightIndex = p_lightIndex;
}
smHapticLightTrans::smHapticLightTrans(smInt p_id)
{
    deviceId = p_id;
    lightIndex = 0;
    newEvent->data = new smLightMotionEventData();
    newEvent->eventType = SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE;
    dispatch->registerEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
}
void smHapticLightTrans::handleEvent(smEvent* p_event)
{
    smHapticOutEventData *hapticEventData;

    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        if (!enabled)
        {
            return;
        }

        hapticEventData = (smHapticOutEventData *)p_event->data;

        if (hapticEventData->deviceId == deviceId)
        {
            ((smLightMotionEventData*)newEvent->data)->lightIndex = lightIndex;
            ((smLightMotionEventData*)newEvent->data)->pos = hapticEventData->position * motionScale;
            computeTransformation(hapticEventData->transform);
            ((smLightMotionEventData*)newEvent->data)->direction = transFormedDirection;

            sendEvent();

        }

        break;
    }
}
