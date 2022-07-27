/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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