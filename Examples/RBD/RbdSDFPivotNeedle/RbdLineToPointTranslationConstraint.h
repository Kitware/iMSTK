/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class RbdLineToPointTranslationConstraint
///
/// \brief Constraints the line p, q to the fixedPt by rotating p and q
///
class RbdLineToPointTranslationConstraint : public RbdConstraint
{
public:
    RbdLineToPointTranslationConstraint(
        std::shared_ptr<RigidBody> obj,
        const Vec3d& fixedPt,
        Vec3d* p, Vec3d* q,
        const double beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_beta(beta), m_fixedPt(fixedPt),
        m_p(p), m_q(q)
    {
    }

    ~RbdLineToPointTranslationConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            // Gives the translation to bring the line p,q to pass through point fixedPt
            const Vec3d axes      = (*m_q - *m_p).normalized();
            const Vec3d diff      = m_fixedPt - *m_p;
            const Vec3d vecToLine = (diff - diff.dot(axes) * axes);
            const Vec3d dirToLine = vecToLine.normalized();

            vu      = vecToLine.norm() * m_beta / dt;
            J(0, 0) = dirToLine[0]; J(0, 1) = 0.0;
            J(1, 0) = dirToLine[1]; J(1, 1) = 0.0;
            J(2, 0) = dirToLine[2]; J(2, 1) = 0.0;
        }
    }

private:
    double m_beta = 0.05;

    Vec3d  m_fixedPt;
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;
};