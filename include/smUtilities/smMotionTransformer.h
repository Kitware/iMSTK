/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#ifndef SMMOTIONTRANSFORMER_H
#define SMMOTIONTRANSFORMER_H

#include "smCore/smSDK.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smUtilities/smQuat.h"
/// \brief motion transformation type
enum smMotionTransType
{
    SM_TRANSMOTION_HAPTIC2CAM,
};
/// \brief  motion transformation basec class
class smMotionTransformer: public smCoreClass, public smEventHandler
{
public:
    smBool enabled;
};
/// \brief motion transformation using a haptic device
class smHapticTrans: public smMotionTransformer
{
protected:
    /// \brief event that will be sent
    smEvent *newEvent;
    /// \brief motion scale
    smFloat motionScale;
    /// \brief device id that will be used
    smInt deviceId;
    /// \brief default directions
    smVec3<smDouble> defaultDirection;
    smVec3<smDouble> defaultUpDirection;
    /// \brief transformaed directions
    smVec3<smDouble> transFormedDirection;
    smVec3<smDouble> transFormedUpDirection;
    /// \brief event dispatcher
    smEventDispatcher *dispatch;
public:
    /// \brief constructor
    smHapticTrans()
    {
        newEvent = new smEvent();
        defaultDirection.setValue(0, 0, -1);
        defaultUpDirection.setValue(0, 1.0, 0);
        dispatch = (smSDK::getInstance())->getEventDispatcher();
        motionScale = 1.0;
        enabled = true;
    }
    /// \brief sets the device to listen
    void setDeviceIdToListen(smInt p_id)
    {
        deviceId = p_id;
    }
    /// \brief sets the motion scale
    void setMotionScale(smFloat p_scale)
    {
        motionScale = p_scale;
    }
    /// \brief compute the transformation usign the p_mat. it transforms the default directions
    inline void computeTransformation(smMatrix44<smDouble> &p_mat)
    {
        static smMatrix33<smDouble> mat;
        mat = p_mat;
        transFormedDirection = (mat * defaultDirection);
        transFormedUpDirection = (mat * defaultUpDirection);
    }
    /// \brief send motion transformation event
    inline void sendEvent()
    {
        dispatch->sendStreamEvent(newEvent);
    }
};
/// \brief class that manipulates the camera motion using the haptic device
class smHapticCameraTrans: public smHapticTrans
{
protected:
    /// \brief  quaternion
    smQuatd quat;
public:
    /// \brief  any offset in the transverse (X) direction
    smDouble offsetAngle_RightDirection;
    /// \brief any offser in the transverse (Y) direction
    smDouble offsetAngle_UpDirection;
    /// \brief constructor gets device id
    smHapticCameraTrans(smInt p_deviceID = 0)
    {
        deviceId = p_deviceID;
        newEvent->data = new smCameraEventData();
        newEvent->eventType = SIMMEDTK_EVENTTYPE_CAMERA_UPDATE;
        dispatch->registerEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
        offsetAngle_RightDirection = 0;
        offsetAngle_UpDirection = 0;
    }
    /// \brief event handler
    inline void handleEvent(smEvent *p_event)
    {
        smHapticOutEventData *hapticEventData;
        smVec3d rightVector;

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

                quat.fromAxisAngle(rightVector, SM_DEGREES2RADIANS(offsetAngle_RightDirection));

                ((smCameraEventData*)newEvent->data)->direction = quat.rotate(((smCameraEventData*)newEvent->data)->direction);
                ((smCameraEventData*)newEvent->data)->upDirection = quat.rotate(((smCameraEventData*)newEvent->data)->upDirection);
                sendEvent();
            }

            break;
        }
    }
};

/// \brief class that manipulates the light with haptic motions
class smHapticLightTrans: public smHapticTrans
{
protected:
    /// \brief  light index that will be transformed
    smInt lightIndex;
public:
    /// \brief  light index can be changed
    void setLightIndex(smInt p_lightIndex)
    {
        lightIndex = p_lightIndex;
    }
    /// \brief constructor that gets device id
    smHapticLightTrans(smInt p_id = 0)
    {
        deviceId = p_id;
        lightIndex = 0;
        newEvent->data = new smLightMotionEventData();
        newEvent->eventType = SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE;
        dispatch->registerEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
    }
    /// \brief handled event
    inline void handleEvent(smEvent *p_event)
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
};

#endif
