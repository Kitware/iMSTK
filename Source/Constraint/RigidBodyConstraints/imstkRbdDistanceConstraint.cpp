/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRbdDistanceConstraint.h"

namespace imstk
{
RbdDistanceConstraint::RbdDistanceConstraint(
    std::shared_ptr<RigidBody> obj1,
    std::shared_ptr<RigidBody> obj2,
    const Vec3d& p1, const Vec3d& p2,
    double dist,
    const Side side) : RbdConstraint(obj1, obj2, side),
    m_p1(p1), m_p2(p2), m_dist(dist)
{
}

void
RbdDistanceConstraint::compute(double imstkNotUsed(dt))
{
    J = Eigen::Matrix<double, 3, 4>::Zero();
    if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
    {
        // Displacement from center of mass
        const Vec3d r1   = m_p1 - m_obj1->getPosition();
        const Vec3d diff = m_p2 - m_p1;
        const Vec3d c    = r1.cross(diff);
        J(0, 0) = -diff[0]; J(0, 1) = -c[0];
        J(1, 0) = -diff[1]; J(1, 1) = -c[1];
        J(2, 0) = -diff[2]; J(2, 1) = -c[2];
    }
    if ((m_side == Side::AB || m_side == Side::B) && !m_obj2->m_isStatic)
    {
        const Vec3d r2   = m_p2 - m_obj2->getPosition();
        const Vec3d diff = m_p2 - m_p1;
        const Vec3d c    = r2.cross(diff);
        J(0, 0) = diff[0]; J(0, 1) = c[0];
        J(1, 0) = diff[1]; J(1, 1) = c[1];
        J(2, 0) = diff[2]; J(2, 1) = c[2];
    }
}
} // namespace imstk