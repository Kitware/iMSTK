/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkMacros.h"
#include "imstkRigidObject2.h"

using namespace imstk;

///
/// \class NeedleObject
///
/// \brief Defines a curved needle using the C6 suture needle standard. The class creates both a rigid object to represent
/// the hull of the needle and a line mesh that closely follows the curvature of the needle. The class also defines axis of
/// needle for certain rigid object embedding constraints.
///

class NeedleObject : public RigidObject2
{
public:
    enum class CollisionState
    {
        REMOVED,
        TOUCHING,
        INSERTED
    };

    enum class PrevCollisionState
    {
        REMOVED,
        INSERTED
    };

    NeedleObject();
    virtual ~NeedleObject() = default;

    IMSTK_TYPE_NAME(NeedleObject)

    // *INDENT-OFF*
    SIGNAL(NeedleObject, inserted);
    SIGNAL(NeedleObject, removed);
    // *INDENT-ON*

    void setCollisionState(const CollisionState state)
    {
        // If current state is inserted and previous was not inserted
        if (m_collisionState == CollisionState::INSERTED && state != CollisionState::INSERTED)
        {
            this->postEvent(Event(removed()));
        }
        // If current state not inserted and previous inserted
        else if (m_collisionState != CollisionState::INSERTED && state == CollisionState::INSERTED)
        {
            this->postEvent(Event(inserted()));
        }
        m_collisionState = state;
    }

    void setPrevCollisionState(const PrevCollisionState state)
    {
        // If current state is inserted and previous was not inserted
        if (m_prevCollisionState == PrevCollisionState::INSERTED && state != PrevCollisionState::INSERTED)
        {
            this->postEvent(Event(removed()));
        }
        // If current state not inserted and previous inserted
        else if (m_prevCollisionState != PrevCollisionState::INSERTED && state == PrevCollisionState::INSERTED)
        {
            this->postEvent(Event(inserted()));
        }
        m_prevCollisionState = state;
    }

    CollisionState getCollisionState() const { return m_collisionState; }
    PrevCollisionState getPrevCollisionState() const { return m_prevCollisionState; }

    ///
    /// \brief sets/gets the minimum force that needs to be applied for puncture to occru.
    /// Note: These are only useful if haptics are being used. Otherwise another metric must
    /// be used.
    /// @{
    void setForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    double getForceThreshold() const { return m_forceThreshold; }
///  @}

protected:
    CollisionState     m_collisionState     = CollisionState::REMOVED;
    PrevCollisionState m_prevCollisionState = PrevCollisionState::REMOVED;

    double m_forceThreshold = 5.0;
};