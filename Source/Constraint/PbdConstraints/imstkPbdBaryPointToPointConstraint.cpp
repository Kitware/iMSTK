/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdBaryPointToPointConstraint.h"

namespace imstk
{
Vec3d
PbdBaryPointToPointConstraint::computeInterpolantDifference(PbdState& bodies) const
{
    Vec3d p1 = Vec3d::Zero();
    Vec3d p2 = Vec3d::Zero();
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        if (m_bodiesSides[i])
        {
            p2 += bodies.getPosition(m_particles[i]) * m_weights[i];
        }
        else
        {
            p1 += bodies.getPosition(m_particles[i]) * m_weights[i];
        }
    }
    return p2 - p1;
}

void
PbdBaryPointToPointConstraint::initConstraintToRest(
    PbdState& bodies,
    const std::vector<PbdParticleId>& ptIdsA,
    const std::vector<double>& weightsA,
    const std::vector<PbdParticleId>& ptIdsB,
    const std::vector<double>& weightsB,
    const double stiffnessA, const double stiffnessB)
{
    initConstraint(ptIdsA, weightsA, ptIdsB, weightsB, stiffnessA, stiffnessB, 0.0);
    setRestLength(computeInterpolantDifference(bodies).norm());
}

void
PbdBaryPointToPointConstraint::initConstraint(
    const std::vector<PbdParticleId>& ptIdsA,
    const std::vector<double>& weightsA,
    const std::vector<PbdParticleId>& ptIdsB,
    const std::vector<double>& weightsB,
    const double stiffnessA, const double stiffnessB,
    const double restLength)
{
    m_particles.resize(ptIdsA.size() + ptIdsB.size());
    m_dcdx.resize(m_particles.size());
    m_weights.resize(m_dcdx.size());
    m_bodiesSides.resize(m_dcdx.size());

    for (size_t i = 0; i < ptIdsA.size(); i++)
    {
        m_particles[i]   = ptIdsA[i];
        m_weights[i]     = weightsA[i];
        m_bodiesSides[i] = false;
    }
    for (size_t i = 0, j = ptIdsA.size(); i < ptIdsB.size(); i++, j++)
    {
        m_particles[j]   = ptIdsB[i];
        m_weights[j]     = weightsB[i];
        m_bodiesSides[j] = true;
    }

    m_restLength   = restLength;
    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
PbdBaryPointToPointConstraint::computeValueAndGradient(PbdState& bodies,
                                                       double& c, std::vector<Vec3d>& dcdx)
{
    // Compute the difference between the interpolant points (points in the two cells)
    Vec3d diff = computeInterpolantDifference(bodies);

    const double length = diff.norm();
    c = length - m_restLength;

    // Save constraint value
    m_C = c;

    if (length < 1.0e-16)
    {
        diff = Vec3d::Zero();
        return false;
    }
    diff /= length;

    for (size_t i = 0; i < dcdx.size(); i++)
    {
        if (m_bodiesSides[i])
        {
            dcdx[i] = -diff * m_weights[i];
        }
        else
        {
            dcdx[i] = diff * m_weights[i];
        }
    }

    return true;
}
} // namespace imstk