/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRbdConstraint.h"

namespace imstk
{
///
/// \class RbdContactConstraint
///
/// \brief A hard rigid body constraint to prevent intersection
///
class RbdContactConstraint : public RbdConstraint
{
public:
    RbdContactConstraint(
        std::shared_ptr<RigidBody> obj1,
        std::shared_ptr<RigidBody> obj2,
        const Vec3d&               contactN,
        const Vec3d&               contactPt,
        const double               contactDepth,
        const double               beta = 0.05,
        const Side                 side = Side::AB) : RbdConstraint(obj1, obj2, side),
        m_contactPt(contactPt), m_contactN(contactN),
        m_contactDepth(contactDepth), m_beta(beta)
    {
    }

    ~RbdContactConstraint() override = default;

public:
    void compute(double dt) override;

private:
    Vec3d  m_contactPt;
    Vec3d  m_contactN;
    double m_contactDepth;
    double m_beta = 0.05;
};
} // namespace imstk