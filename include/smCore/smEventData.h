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

#ifndef SMEVENTDATA_H
#define SMEVENTDATA_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smKey.h"
#include "smCore/smMouse.h"
#include "smUtilities/smMatrix.h"
#include "smUtilities/smVector.h"
#include "smCore/smKeyGLFWInterface.h" //contains an interface to convert GLFW keys to smKeys

struct smEventData {};

/// \brief keyboard event data
struct smKeyboardEventData : public smEventData
{
    smKey keyBoardKey; ///< Key that was pressed
    smBool pressed; ///< if the key was pressed or released in this event
    smModKey modKeys; ///< modifier keys.  See smModKey for values
};

/// \brief mouse event data
struct smMouseButtonEventData  : public smEventData
{
    smMouseButton mouseButton; ///<Which mouse button was pressed
    smBool pressed; ///< if the button was pressed or released in this event
    smDouble windowX; ///< window X coorindate relative to left edge
    smDouble windowY; ///< window Y coorindate relative to top edge
};

struct smMouseMoveEventData  : public smEventData
{
    smDouble windowX; ///< window X coorindate relative to left edge
    smDouble windowY; ///< window Y coorindate relative to top edge
};

/// \brief object click event related
struct smObjectClickedData  : public smEventData
{
    smInt objectId;
    smVec3f clickedPosition;
};

/// \brief for synchronization use
struct smSynchEventData  : public smEventData
{
};

/// \brief  haptic out event data. Event that is sent from haptic device
struct smHapticOutEventData  : public smEventData
{
    /// \brief device id
    smInt deviceId;
    /// \brief device name
    smString deviceName;
    ///\brief  position
    smVec3f position;
    /// \brief  velocity of the tip
    smVec3f velocity;
    /// \brief angles
    smVec3f angles;
    /// \brief  transformation matrix
    smMatrix44f transform;
    smBool buttonState[4];//will be chnage later on
};
/// \brief haptic that is sent to the device
struct smHapticInEventData  : public smEventData
{
    /// \brief  device id
    smInt deviceId;
    /// \brief  force
    smVec3f force;
    /// \brief  torque
    smVec3f torque;

};
/// \brief the collision that that has information of primitives corresponds to the objects.
template <class smCollisionData>
struct smObjectHitData  : public smEventData
{
    smInt objectSourceId;
    smInt objectTargetId;
    smCollisionData data;
};
/// \brief  virtual camera motion
struct smCameraEventData  : public smEventData
{
    /// \brief  camera position
    smVec3f pos;
    /// \brief direction
    smVec3f direction;
    /// \brief  upward direction
    smVec3f upDirection;
};
/// \brief  light motion evet data
struct smLightMotionEventData  : public smEventData
{
    /// \brief  light index
    smInt lightIndex;
    /// \brief  position
    smVec3f pos;
    /// \brief view direction
    smVec3f direction;
};

#endif
