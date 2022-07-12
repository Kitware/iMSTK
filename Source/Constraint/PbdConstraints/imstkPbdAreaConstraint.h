/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
////
/// \class PbdAreaConstraint
///
/// \brief Area constraint for triangular face
///
class PbdAreaConstraint : public PbdConstraint
{
public:
    PbdAreaConstraint() : PbdConstraint(3) { }

    ///
    /// \brief Initialize the constraint
    ///
    void initConstraint(
        const Vec3d& p0, const Vec3d& p1, const Vec3d& p2,
        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1, const PbdParticleId& pIdx2,
        const double k = 2.5);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

public:
    double m_restArea = 0.0;  ///< Area at the rest position
};
} // namespace imstk