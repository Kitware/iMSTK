/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkRigidObject2.h"

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
    NeedleObject(const std::string& name) : RigidObject2(name) { }
    virtual ~NeedleObject() = default;

    IMSTK_TYPE_NAME(NeedleObject)

public:
    void setCollisionState(const CollisionState state) { m_collisionState = state; }
    CollisionState getCollisionState() const { return m_collisionState; }

    ///
    /// \brief Set the force threshold for the needle
    ///
    void setForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    double getForceThreshold() const { return m_forceThreshold; }

    ///
    /// \brief Returns the current axes of the needle (tip-tail)
    ///
    const Vec3d getAxes() const
    {
        return (-getCollidingGeometry()->getRotation().col(1)).normalized();
    }

protected:
    CollisionState m_collisionState = CollisionState::REMOVED;
    double m_forceThreshold = 10.0;
};