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
/// \class RbdFrictionConstraint
///
/// \brief A rigid body constraint to apply friction
///
class RbdFrictionConstraint : public RbdConstraint
{
public:
    RbdFrictionConstraint(
        std::shared_ptr<RigidBody> obj1,
        std::shared_ptr<RigidBody> obj2,
        const Vec3d&               contactPt,
        const Vec3d&               contactNormal,
        const double               contactDepth,
        const double               frictionCoefficient,
        const Side                 side = Side::AB);
    ~RbdFrictionConstraint() override = default;

    IMSTK_TYPE_NAME(RbdFrictionConstraint);

public:
    void compute(double dt) override;

private:
    Vec3d  m_contactPt;
    Vec3d  m_contactN;
    double m_contactDepth;
    double m_frictionCoefficient;
};
} // namespace imstk