/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#include "imstkPbdConstantDensityConstraint.h"
#include "imstkParallelUtils.h"

namespace imstk
{
void
PbdConstantDensityConstraint::initConstraint(const StdVectorOfVec3d& initVertexPositions, const double)
{
    const size_t numParticles = initVertexPositions.size();

    // constraint parameters
    // Refer: Miller, et al 2003 "Particle-Based Fluid Simulation for Interactive Applications."
    /// \todo Check if these numbers can be variable
    m_wPoly6Coeff = 315.0 / (64.0 * PI * pow(m_maxDist, 9));
    m_wSpikyCoeff = 15.0 / (PI * pow(m_maxDist, 6));

    m_lambdas.resize(numParticles);
    m_densities.resize(numParticles);
    m_deltaPositions.resize(numParticles);
    m_neighborList.resize(numParticles);

    // Initialize neighbor searcher
    m_NeighborSearcher = std::make_shared<NeighborSearch>(m_NeighborSearchMethod, m_maxDist);
}

bool
PbdConstantDensityConstraint::solvePositionConstraint(
    StdVectorOfVec3d&      currVertexPositions,
    const StdVectorOfReal& currInvMasses)
{
    const size_t numParticles = currVertexPositions.size();

    // Search neighbor for each particle
    m_NeighborSearcher->getNeighbors(m_neighborList, currVertexPositions);

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            computeDensity(currVertexPositions[idx], idx, currVertexPositions);
    });

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            computeLambdaScalingFactor(currVertexPositions[idx], idx, currVertexPositions);
    });

    ParallelUtils::parallelFor(numParticles,
        [&](const size_t idx) {
            updatePositions(currVertexPositions[idx], idx, currVertexPositions);
    });

    return true;
}

double
PbdConstantDensityConstraint::wPoly6(const Vec3d& pi, const Vec3d& pj)
{
    double rLengthSqr = (Vec3d(pi - pj)).squaredNorm();

    return (rLengthSqr > m_maxDistSqr || rLengthSqr < 1e-20) ?
           0 :
           m_wPoly6Coeff* pow(m_maxDistSqr - rLengthSqr, 3);
}

Vec3d
PbdConstantDensityConstraint::gradSpiky(const Vec3d& pi, const Vec3d& pj)
{
    Vec3d        r = pi - pj;
    const double rLengthSqr = r.squaredNorm();

    if (rLengthSqr > m_maxDistSqr || rLengthSqr < 1e-20)
    {
        return Vec3d(0., 0., 0.);
    }

    const double rLength = std::sqrt(rLengthSqr);
    return r * (m_wSpikyCoeff * (-3.0) * (m_maxDist - rLength) * (m_maxDist - rLength));
}

void
PbdConstantDensityConstraint::computeDensity(const Vec3d&            pi,
                                             const size_t            index,
                                             const StdVectorOfVec3d& positions)
{
    double densitySum = 0.0;
    for (auto q : m_neighborList[index])
    {
        densitySum += wPoly6(pi, positions[q]);
    }

    m_densities[index] = densitySum;
}

void
PbdConstantDensityConstraint::computeLambdaScalingFactor(const Vec3d&            pi,
                                                         const size_t            index,
                                                         const StdVectorOfVec3d& positions)
{
    const double densityConstraint = (m_densities[index] / m_restDensity) - 1;
    double       gradientSum       = 0.0;
    for (auto q : m_neighborList[index])
    {
        gradientSum += gradSpiky(pi, positions[q]).squaredNorm() / m_restDensity;
    }

    m_lambdas[index] = densityConstraint / (gradientSum + m_relaxationParameter);
}

void
PbdConstantDensityConstraint::updatePositions(const Vec3d&      pi,
                                              const size_t      index,
                                              StdVectorOfVec3d& positions)
{
    //Make sure the point is valid
    Vec3d gradientLambdaSum(0., 0., 0.);
    for (auto q : m_neighborList[index])
    {
        double lambdasDiff = (m_lambdas[index] + m_lambdas[q]);
        Vec3d  gradKernal  = gradSpiky(pi, positions[q]);
        gradientLambdaSum += (gradKernal * lambdasDiff);
    }

    m_deltaPositions[index] = gradientLambdaSum / m_restDensity;
    positions[index] += m_deltaPositions[index];
}

void
PbdConstantDensityConstraint::setMaxNeighborDistance(const double dist)
{
    m_maxDist    = dist;
    m_maxDistSqr = dist * dist;
    if (m_NeighborSearcher)
    {
        m_NeighborSearcher->setSearchRadius(m_maxDist);
    }
}
} // imstk
