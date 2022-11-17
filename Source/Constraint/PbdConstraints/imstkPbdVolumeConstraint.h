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
/// \class PbdVolumeConstraint
///
/// \brief Volume constraint for tetrahedral element
///
class PbdVolumeConstraint : public PbdConstraint
{
public:
    PbdVolumeConstraint() : PbdConstraint(4) { }

    ///
    /// \brief Initializes the volume constraint
    ///
    void initConstraint(
        const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
        const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
        const double k = 2.0);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

    ///
    /// \brief Return the rest configuration for the constraint
    ///
    double getRestValue() const { return m_restVolume; }

protected:
    double m_restVolume = 0.0; ///< Rest volume
};
} // namespace imstk