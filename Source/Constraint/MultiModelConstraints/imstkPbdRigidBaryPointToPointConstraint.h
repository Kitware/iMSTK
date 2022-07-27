/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkRbdConstraint.h"

namespace imstk
{
// Define constraint  Note: can be given to either solver
// Single point, build multiple of them for each entity in contact
class PbdRigidBaryPointToPointConstraint : public PbdBaryPointToPointConstraint, public RbdConstraint
{
private:
    double m_beta = 0.0001;

public:
    PbdRigidBaryPointToPointConstraint(std::shared_ptr<RigidBody> obj1);
    ~PbdRigidBaryPointToPointConstraint() override = default;

public:
    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdxA constraint gradient for A
    /// \param[inout] dcdxB constraint gradient for B
    /// Call for RBD, push point on mesh to the fixed point halfway
    /// between the rigid body and the PBD object
    bool computeValueAndGradient(
        double&             c,
        std::vector<Vec3d>& dcdxA,
        std::vector<Vec3d>& dcdxB) const override;

public:
    // Constraint call for RBD
    // Give change in impulse in direction of desired deformation
    void compute(double dt) override;
};
} // namespace imstk