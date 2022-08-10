/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdEdgeEdgeConstraint
///
/// \brief Resolves an edge to coincide with a point & point to edge
///
class PbdPointEdgeConstraint : public PbdCollisionConstraint
{
public:
    PbdPointEdgeConstraint() : PbdCollisionConstraint(1, 2) { }
    ~PbdPointEdgeConstraint() override = default;

public:
    ///
    /// \brief Initialize the constraint
    ///
    void initConstraint(
        const PbdParticleId& ptA1,
        const PbdParticleId& ptB1, const PbdParticleId& ptB2,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;
};
} // namespace imstk