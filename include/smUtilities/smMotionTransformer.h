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

#ifndef SMMOTIONTRANSFORMER_H
#define SMMOTIONTRANSFORMER_H


#include "smCore/smSDK.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smUtilities/smQuaternion.h"

/// \brief motion transformation type
enum smMotionTransType
{
    SM_TRANSMOTION_HAPTIC2CAM,
};

/// \brief  motion transformation basec class
class smMotionTransformer: public smEventHandler
{
public:
    smBool enabled;
};

/// \brief motion transformation using a haptic device
class smHapticTrans: public smMotionTransformer
{
public:
    /// \brief constructor
    smHapticTrans();

    /// \brief sets the device to listen
    void setDeviceIdToListen(smInt p_id);

    /// \brief sets the motion scale
    void setMotionScale(smFloat p_scale);

    /// \brief compute the transformation usign the p_mat. it transforms the default directions
    void computeTransformation(smMatrix44f &p_mat);

    /// \brief send motion transformation event
    void sendEvent();

    /// \brief Get devide id
    smInt getDeviceId()
    {
        return deviceId;
    }

    /// \brief s devide id
    void setDeviceId(smInt id)
    {
        deviceId = id;
    }

    /// \brief Get devide id
    std::shared_ptr<smEvent> &getNewEvent()
    {
        return newEvent;
    }

    /// \brief s devide id
    void setNewEvent(std::shared_ptr<smEvent> event)
    {
        newEvent = event;
    }

    /// \brief Get devide id
    std::shared_ptr<smEventDispatcher> &getDispatch()
    {
        return dispatch;
    }

    /// \brief s devide id
    void setDispatch(std::shared_ptr<smEventDispatcher> newDispatch)
    {
        dispatch = newDispatch;
    }

protected:
    smInt deviceId; // device id that will be used
    smFloat motionScale; // motion scale
    smVec3f transFormedDirection; // transformaed directions
    smVec3f transFormedUpDirection;
    smVec3f defaultDirection; // default directions
    smVec3f defaultUpDirection;
    std::shared_ptr<smEvent> newEvent; // event that will be sent
    std::shared_ptr<smEventDispatcher> dispatch; // event dispatcher

};

/// \brief class that manipulates the camera motion using the haptic device
class smHapticCameraTrans: public smHapticTrans
{
public:
    smHapticCameraTrans(smInt p_deviceID = 0); // constructor gets device id

    void handleEvent(std::shared_ptr<smEvent> p_event);// event handler

protected:
    smQuaternionf quat; // quaternion

public:
    smDouble offsetAngle_RightDirection; // any offset in the transverse (X) direction
    smDouble offsetAngle_UpDirection; // any offser in the transverse (Y) direction
};

/// \brief class that manipulates the light with haptic motions
class smHapticLightTrans: public smHapticTrans
{
protected:
    smInt lightIndex; // light index that will be transformed

public:
    /// \brief  light index can be changed
    void setLightIndex(smInt p_lightIndex);

    /// \brief constructor that gets device id
    smHapticLightTrans(smInt p_id = 0);

    /// \brief handled event
    void handleEvent(std::shared_ptr<smEvent> p_event);
};

#endif
