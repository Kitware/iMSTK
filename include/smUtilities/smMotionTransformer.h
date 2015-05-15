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

// SimMedTK includes
#include "smCore/smCoreClass.h"
#include "smUtilities/smQuaternion.h"

namespace smtk {
namespace Event {
class smEvent;
class smEventHandler;
class smCameraEvent;
}
}

/// \brief motion transformation type
enum smMotionTransType
{
    SM_TRANSMOTION_HAPTIC2CAM,
};


/// \brief motion transformation using a haptic device
class smHapticTrans: public smCoreClass
{
public:
    /// \brief constructor
    smHapticTrans();

    /// \brief sets the device to listen
    void setDeviceIdToListen(const size_t &p_id);

    /// \brief sets the motion scale
    void setMotionScale(const float &p_scale);

    /// \brief compute the transformation usign the p_mat. it transforms the default directions
    void computeTransformation(smMatrix44f &p_mat);

    /// \brief send motion transformation event
    void sendEvent();

    /// \brief Get devide id
    const size_t &getDeviceId();

    /// \brief s devide id
    void setDeviceId(const size_t &id);

    /// \brief Get devide id
    std::shared_ptr<smtk::Event::smEvent> getNewEvent();

    /// \brief s devide id
    void setNewEvent(std::shared_ptr<smtk::Event::smEvent> event);

    /// \brief Get devide id
    std::shared_ptr<smtk::Event::smEventHandler> getEventHandler();

    /// \brief s devide id
    void setEventHandler(std::shared_ptr<smtk::Event::smEventHandler> newEventHandler);

protected:
    size_t deviceId; // device id that will be used
    float motionScale; // motion scale
    smVec3d defaultDirection; // default directions
    smVec3d defaultUpDirection;
    std::shared_ptr<smtk::Event::smEventHandler> eventHandler; // event dispatcher

};

/// \brief class that manipulates the camera motion using the haptic device
class smHapticCameraTrans: public smHapticTrans
{
public:
    smHapticCameraTrans(const size_t &p_deviceID = size_t()); // constructor gets device id

    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;// event handler

protected:
    smQuaterniond quat; // quaternion

public:
    smDouble offsetAngle_RightDirection; // any offset in the transverse (X) direction
    smDouble offsetAngle_UpDirection; // any offser in the transverse (Y) direction
};

/// \brief class that manipulates the light with haptic motions
class smHapticLightTrans: public smHapticTrans
{
protected:
    size_t lightIndex; // light index that will be transformed

public:
    /// \brief  light index can be changed
    void setLightIndex(const size_t &p_lightIndex);

    /// \brief constructor that gets device id
    smHapticLightTrans(const size_t &p_id = size_t());

    /// \brief handled event
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;
};

#endif
