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

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix44.h"
#include <QKeyEvent>

using namespace std;

/// \brief keyboard event data
struct smKeyboardEventData
{
    smInt keyBoardKey;
};

/// \brief mouse event data
struct smMouseEventData
{
    /// \brief window X coordinate
    smInt windowX;
    /// \brief window Y coorindate
    smInt windowY;
};

/// \brief object click event related
struct smObjectClickedData
{
    smInt objectId;
    smVec3<smFloat> clickedPosition;
};

/// \brief for synchronization use
struct smSynchEventData
{
};

/// \brief  haptic out event data. Event that is sent from haptic device
struct smHapticOutEventData
{
    /// \brief device id
    smInt deviceId;
    /// \brief device name
    smString deviceName;
    ///\brief  position
    smVec3 <smDouble> position;
    /// \brief  velocity of the tip
    smVec3 <smDouble> velocity;
    /// \brief angles
    smVec3 <smDouble> angles;
    /// \brief  transformation matrix
    smMatrix44<smDouble> transform;
    smBool buttonState[4];//will be chnage later on
};
/// \brief haptic that is sent to the device
struct smHapticInEventData
{
    /// \brief  device id
    smInt deviceId;
    /// \brief  force
    smVec3 <smDouble> force;
    /// \brief  torque
    smVec3 <smDouble> torque;

};
/// \brief the collision that that has information of primitives corresponds to the objects.
template <class smCollisionData>
struct smObjectHitData
{
    smInt objectSourceId;
    smInt objectTargetId;
    smCollisionData data;
};
/// \brief  virtual camera motion
struct smCameraEventData
{
    /// \brief  camera position
    smVec3<smDouble> pos;
    /// \brief direction
    smVec3<smDouble> direction;
    /// \brief  upward direction
    smVec3<smDouble> upDirection;
};
/// \brief  light motion evet data
struct smLightMotionEventData
{
    /// \brief  light index
    smInt lightIndex;
    /// \brief  position
    smVec3<smDouble> pos;
    /// \brief view direction
    smVec3<smDouble> direction;
};

#endif
