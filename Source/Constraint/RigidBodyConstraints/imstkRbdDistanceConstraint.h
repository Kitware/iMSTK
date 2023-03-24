/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class RbdDistanceConstraint
///
/// \brief A rigid body constraint to keep objects at a specified distance
/// from each other given two local points on the bodies
///
class RbdDistanceConstraint : public RbdConstraint
{
public:
    RbdDistanceConstraint(
        std::shared_ptr<RigidBody> obj1,
        std::shared_ptr<RigidBody> obj2,
        const Vec3d& p1, const Vec3d& p2,
        double dist,
        const Side side = Side::AB);
    ~RbdDistanceConstraint() override = default;

    IMSTK_TYPE_NAME(RbdDistanceConstraint);

public:
    void compute(double dt) override;

private:
    Vec3d  m_p1   = Vec3d(0.0, 0.0, 0.0);
    Vec3d  m_p2   = Vec3d(0.0, 0.0, 0.0);
    double m_dist = 1.0;
};
} // namespace imstk