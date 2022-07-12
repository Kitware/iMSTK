/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstantDensityConstraint.h"
#include "imstkParallelUtils.h"

namespace imstk
{
PbdConstantDensityConstraint::PbdConstantDensityConstraint() : PbdConstraint()
{
    m_particleRadiusSqr = m_particleRadius * m_particleRadius;
    m_wPoly6Coeff       = 315.0 / (64.0 * PI * pow(m_particleRadius, 9));
    m_wSpikyCoeff       = 15.0 / (PI * pow(m_particleRadius, 6)) * -3.0;
}

void
PbdConstantDensityConstraint::initConstraint(const int numParticles,
                                             const int bodyHandle,
                                             const double particleRadius, const double density)
{
    m_lambdas.resize(numParticles);
    m_densities.resize(numParticles);
    m_deltaPositions.resize(numParticles);
    m_neighborList.resize(numParticles);
    m_bodyHandle = bodyHandle;

    m_restDensity       = density;
    m_particleRadius    = particleRadius;
    m_particleRadiusSqr = m_particleRadius * m_particleRadius;
    m_wPoly6Coeff       = 315.0 / (64.0 * PI * pow(m_particleRadius, 9));
    m_wSpikyCoeff       = 15.0 / (PI * pow(m_particleRadius, 6)) * -3.0;

    // Initialize neighbor searcher
    m_NeighborSearcher = std::make_shared<NeighborSearch>(m_NeighborSearchMethod, m_particleRadius);
}

void
PbdConstantDensityConstraint::projectConstraint(PbdState& state,
                                                const double imstkNotUsed(dt), const SolverType& imstkNotUsed(type))
{
    const size_t             numParticles = state.m_bodies[m_bodyHandle]->vertices->size();
    VecDataArray<double, 3>& vertices     = *state.m_bodies[m_bodyHandle]->vertices;

    // Search neighbor for each particle
    m_NeighborSearcher->getNeighbors(m_neighborList, vertices);

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            computeDensity(vertices[idx], idx, vertices);
    });

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            computeLambdaScalingFactor(vertices[idx], idx, vertices);
    });

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            updatePositions(vertices[idx], idx, vertices);
    });
}

void
PbdConstantDensityConstraint::computeDensity(const Vec3d& pi,
                                             const size_t index,
                                             const VecDataArray<double, 3>& positions)
{
    double densitySum = 0.0;
    for (auto q : m_neighborList[index])
    {
        densitySum += wPoly6(pi, positions[q]);
    }

    m_densities[index] = densitySum;
}

void
PbdConstantDensityConstraint::computeLambdaScalingFactor(const Vec3d& pi,
                                                         const size_t index,
                                                         const VecDataArray<double, 3>& positions)
{
    const double invRestDensity    = 1.0 / m_restDensity;
    const double densityConstraint = (m_densities[index] * invRestDensity) - 1.0;
    double       gradientSum       = 0.0;

    for (auto q : m_neighborList[index])
    {
        gradientSum += gradSpiky(pi, positions[q]).squaredNorm() * invRestDensity;
    }

    m_lambdas[index] = densityConstraint / (gradientSum + m_relaxationParameter);
}

void
PbdConstantDensityConstraint::updatePositions(const Vec3d& pi,
                                              const size_t index,
                                              VecDataArray<double, 3>& positions)
{
    // Make sure the point is valid
    Vec3d gradientLambdaSum(0.0, 0.0, 0.0);
    for (auto q : m_neighborList[index])
    {
        const double lambdasDiff = (m_lambdas[index] + m_lambdas[q]);
        const Vec3d  gradKernal  = gradSpiky(pi, positions[q]);
        gradientLambdaSum += (gradKernal * lambdasDiff);
    }

    m_deltaPositions[index] = gradientLambdaSum / m_restDensity;
    positions[index] += m_deltaPositions[index];
}
} // namespace imstk