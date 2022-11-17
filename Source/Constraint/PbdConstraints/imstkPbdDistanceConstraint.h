/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdDistanceConstraint
///
/// \brief Distance constraints between two nodal points
///
class PbdDistanceConstraint : public PbdConstraint
{
public:
    PbdDistanceConstraint() : PbdConstraint(2) { }

    ///
    /// \brief Initialize the constraint with resting length
    /// as the length between the two points
    ///
    void initConstraint(
        const Vec3d& p0, const Vec3d& p1,
        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
        const double k = 1e5)
    {
        initConstraint((p0 - p1).norm(), pIdx0, pIdx1, k);
    }

    ///
    /// \brief Initialize the constraint with provided resting length
    ///
    void initConstraint(const double restLength,
                        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
                        const double k = 1e5);

    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

    ///
    /// \brief Return the rest configuration for the constraint
    ///
    double getRestValue() const { return m_restLength; }

public:
    double m_restLength = 0.0; ///< Rest length between the nodes
};
} // namespace imstk