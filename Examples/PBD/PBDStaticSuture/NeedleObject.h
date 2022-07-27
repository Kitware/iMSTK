/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRigidObject2.h"
#include "imstkMacros.h"

using namespace imstk;

class NeedleObject : public RigidObject2
{
public:
    enum class CollisionState
    {
        REMOVED,
        TOUCHING,
        INSERTED
    };

public:
    NeedleObject();
    virtual ~NeedleObject() = default;

    IMSTK_TYPE_NAME(NeedleObject)

public:
    // *INDENT-OFF*
    SIGNAL(NeedleObject, inserted);
    SIGNAL(NeedleObject, removed);
    // *INDENT-ON*

public:
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

    CollisionState getCollisionState() const { return m_collisionState; }

    ///
    /// \brief Set the force threshold for the needle
    ///
    void setForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    double getForceThreshold() const { return m_forceThreshold; }

    void setArc(const Vec3d& arcCenter, const Mat3d& arcBasis,
                double arcRadius, double beginRad, double endRad)
    {
        m_arcCenter = arcCenter;
        m_arcBasis  = arcBasis;
        m_beginRad  = beginRad;
        m_endRad    = endRad;
        m_arcRadius = arcRadius;
    }

    ///
    /// \brief Get the basis post transformation of the rigid body
    ///
    const Mat3d getArcBasis();
    ///
    /// \brief Get the arc center post transformation of the rigid body
    ///
    const Vec3d getArcCenter();
    const double getBeginRad() const { return m_beginRad; }
    const double getEndRad() const { return m_endRad; }
    const double getArcRadius() const { return m_arcRadius; }

protected:
    CollisionState m_collisionState = CollisionState::REMOVED;
    double m_forceThreshold = 5.0;

    Mat3d  m_arcBasis  = Mat3d::Identity();
    Vec3d  m_arcCenter = Vec3d::Zero();
    double m_arcRadius = 1.0;
    double m_beginRad  = 0.0;
    double m_endRad    = PI * 2.0;
};