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
public:
    PbdRigidBaryPointToPointConstraint(std::shared_ptr<RigidBody> obj1);
    ~PbdRigidBaryPointToPointConstraint() override = default;

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

    // Constraint call for RBD
    // Give change in impulse in direction of desired deformation
    void compute(double dt) override;

private:
    double m_beta = 0.0001;
    mutable Vec3d m_diff = Vec3d::Zero();
};
} // namespace imstk