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
/// \class PbdEdgeEdgeCCDConstraint
///
/// \brief Pushes an edge "outside" the other edge
///
class PbdEdgeEdgeCCDConstraint : public PbdCollisionConstraint
{
public:
    PbdEdgeEdgeCCDConstraint() : PbdCollisionConstraint(2, 2) { }
    ~PbdEdgeEdgeCCDConstraint() override = default;

public:
    ///
    /// \brief Initialize constraint
    ///
    void initConstraint(
        Vec3d* prevPtA0, Vec3d* prevPtA1,
        Vec3d* prevPtB0, Vec3d* prevPtB1,
        const PbdParticleId& ptA0, const PbdParticleId& ptA1,
        const PbdParticleId& ptB0, const PbdParticleId& ptB1,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    // Extra particles used but not solved for
    std::array<Vec3d*, 2> m_prevEdgeA;
    std::array<Vec3d*, 2> m_prevEdgeB;
};
} // namespace imstk