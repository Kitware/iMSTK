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
/// \brief Resolves an edge given by two pbd particles to coincide with
/// another edge
///
class PbdEdgeEdgeConstraint : public PbdCollisionConstraint
{
public:
    PbdEdgeEdgeConstraint() : PbdCollisionConstraint(2, 2) { }
    ~PbdEdgeEdgeConstraint() override = default;

    IMSTK_TYPE_NAME(PbdEdgeEdgeConstraint)

public:
    ///
    /// \brief Initialize constraint
    /// \param First point of edge A
    /// \param Second point of edge A
    /// \param First point of edge B
    /// \param Second point of edge B
    /// \param Stiffness of A
    /// \param Stiffness of B
    ///
    void initConstraint(
        const PbdParticleId& ptA1, const PbdParticleId& ptA2,
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