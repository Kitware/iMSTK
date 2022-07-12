/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
////
/// \class PbdPointPointConstraint
///
/// \brief This constraint resolves two vertices to each other
///
class PbdPointPointConstraint : public PbdCollisionConstraint
{
public:
    PbdPointPointConstraint() : PbdCollisionConstraint(1, 1) { }
    ~PbdPointPointConstraint() override = default;

public:
    ///
    /// \brief Initialize constraint
    ///
    void initConstraint(
        const PbdParticleId& ptA, const PbdParticleId& ptB,
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
} // imstk