/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class RbdAngularLockingConstraint
///
/// \brief Constrains the orientation to some fixed orientation
///
class RbdAngularLockingConstraint : public RbdConstraint
{
public:
    RbdAngularLockingConstraint(
        std::shared_ptr<RigidBody> obj,
        const Quatd&               fixedOrientation,
        const double               beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_fixedOrientation(fixedOrientation),
        m_beta(beta)
    {
    }

    ~RbdAngularLockingConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            const Quatd dq = m_fixedOrientation * m_obj1->getOrientation().inverse();
            const Rotd  angleAxes = Rotd(dq);
            const Vec3d rotAxes   = angleAxes.axis();
            vu      = angleAxes.angle() * m_beta / dt;
            J(0, 0) = 0.0; J(0, 1) = rotAxes[0];
            J(1, 0) = 0.0; J(1, 1) = rotAxes[1];
            J(2, 0) = 0.0; J(2, 1) = rotAxes[2];
        }
    }

private:
    Quatd  m_fixedOrientation; ///< Orientation to fix too
    double m_beta = 0.05;
};