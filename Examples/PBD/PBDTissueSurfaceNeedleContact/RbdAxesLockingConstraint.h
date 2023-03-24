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
/// \brief Constrains the body center of mass to a fixed axes
///
class RbdAxesLockingConstraint : public RbdConstraint
{
public:
    RbdAxesLockingConstraint(
        std::shared_ptr<RigidBody> obj,
        const Vec3d&               axesPt,
        const Vec3d&               axesDir,
        const double               beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_axesPt(axesPt),
        m_axesDir(axesDir),
        m_beta(beta)
    {
    }

    ~RbdAxesLockingConstraint() override = default;

    IMSTK_TYPE_NAME(RbdAxesLockingConstraint)

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            // Displacement to needle Axes, constrain it to the axes
            const Vec3d  diff = m_obj1->getPosition() - m_axesPt;
            const Vec3d  perpDisplacement = diff - m_axesDir.dot(diff) * m_axesDir;
            const double displacement     = perpDisplacement.norm();
            if (displacement != 0)
            {
                const Vec3d displacementDir = perpDisplacement / displacement;
                vu = displacement * m_beta / dt;

                // Displacement from center of mass
                J(0, 0) = -displacementDir[0]; J(0, 1) = 0.0;
                J(1, 0) = -displacementDir[1]; J(1, 1) = 0.0;
                J(2, 0) = -displacementDir[2]; J(2, 1) = 0.0;
            }
            else
            {
                vu = 0.0;
            }
        }
    }

private:
    Vec3d  m_axesPt;  ///< Point on the axes to constrain too
    Vec3d  m_axesDir; ///< Axes to constrain too
    double m_beta = 0.05;
};