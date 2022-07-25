/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class RbdAxesLockingConstraint
///
/// \brief Constraints the line p, q to the fixedPt by rotating p and q
///
class RbdLineToPointRotationConstraint : public RbdConstraint
{
public:
    RbdLineToPointRotationConstraint(
        std::shared_ptr<RigidBody> obj,
        const Vec3d& fixedPt,
        Vec3d* p, Vec3d* q,
        const double beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_beta(beta), m_fixedPt(fixedPt),
        m_p(p), m_q(q)
    {
    }

    ~RbdLineToPointRotationConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            // Gives the rotation to bring the line p,q to pass through point fixedPt
            const Vec3d axes    = (*m_q - m_obj1->getPosition()).normalized();
            const Vec3d diff    = m_fixedPt - m_obj1->getPosition();
            const Vec3d rot     = axes.cross(diff.normalized());
            const Vec3d rotAxes = rot.normalized();

            // rot.norm gives area of crossed vectors, should be 0 when rotated to each other
            vu      = rot.norm() * m_beta / dt;
            J(0, 0) = 0.0; J(0, 1) = rotAxes[0];
            J(1, 0) = 0.0; J(1, 1) = rotAxes[1];
            J(2, 0) = 0.0; J(2, 1) = rotAxes[2];
        }
    }

private:
    double m_beta = 0.05;

    Vec3d  m_fixedPt;
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;
};