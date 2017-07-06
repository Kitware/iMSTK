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
#include "imstkPbdModel.h"

namespace imstk
{
void
PbdConstantDensityConstraint::initConstraint(PbdModel& model, const double k)
{
    const auto state = model.getCurrentState();
    const auto np = state->getPositions().size();

    // constraint parameters
    // Refer: Miller, et al 2003 "Particle-Based Fluid Simulation for Interactive Applications."
    // TODO: Check if these numbers can be variable
    m_wPoly6Coeff = 315.0 / (64.0 * PI * pow(m_maxDist, 9));
    m_wSpikyCoeff = 15.0 / (PI * pow(m_maxDist, 6));

    m_lambdas.resize(np);
    m_densities.resize(np);
    m_numNeighbors.resize(np);
    m_deltaPositions.resize(np);
    m_neighbors.resize(np * m_maxNumNeighbors);

    m_xPosIndexes.resize(np);
    m_yPosIndexes.resize(np);
    m_zPosIndexes.resize(np);
}

bool
PbdConstantDensityConstraint::solvePositionConstraint(PbdModel& model)
{
    const auto state = model.getCurrentState();
    auto& pos = state->getPositions();
    const auto np = pos.size();

    clearNeighbors(np);

    for (auto index = 0; index < np; ++index)
    {
        this->updateNeighborsBruteForce(pos[index], index, pos);
    }

    for (auto index = 0; index < np; ++index)
    {
        this->calculateDensityEstimate(pos[index], index, pos);
    }

    for (auto index = 0; index < np; ++index)
    {
        this->calculateLambdaScalingFactor(pos[index], index, pos);
    }

    for (auto index = 0; index < np; ++index)
    {
        this->updatePositions(pos[index], index, pos);
    }

    return true;
}

inline double
PbdConstantDensityConstraint::wPoly6(const Vec3d &pi, const Vec3d &pj)
{
    double rLength = (pi - pj).norm();

    return (rLength > m_maxDist || rLength == 0) ?
           0 :
           m_wPoly6Coeff * pow((m_maxDist * m_maxDist) - (rLength * rLength), 3);
}

inline Vec3d
PbdConstantDensityConstraint::gradSpiky(const Vec3d &pi, const Vec3d &pj)
{
    Vec3d r = pi - pj;
    const double rLength = r.norm();

    return (rLength > m_maxDist || rLength == 0) ?
           Vec3d(0., 0., 0.) :
           r * (m_wSpikyCoeff * (-3.0) * (m_maxDist - rLength) * (m_maxDist - rLength));
}

inline void
PbdConstantDensityConstraint::clearNeighbors(const size_t &np)
{
    m_numNeighbors.clear();
    m_neighbors.clear();
    m_numNeighbors.resize(np);
    m_neighbors.resize(np * m_maxNumNeighbors);
}

inline void
PbdConstantDensityConstraint::updateNeighborsBruteForce(const Vec3d &pi,
                                                        const size_t &index,
                                                        const StdVectorOfVec3d &positions)
{
    const double neighborRadius = m_maxDist;
    int neighborCount = 0;

    //loop over all points
    for (auto j = 0; j < positions.size(); ++j)
    {
        if (j != index)
        {
            if (neighborCount >= m_maxNumNeighbors)
            {
                LOG(WARNING) << "Neighbor count reached max. for point: " << index;
                continue;
            }

            if ((pi - positions[j]).norm() < neighborRadius)
            {
                m_neighbors[index * m_maxNumNeighbors + neighborCount] = j;
                neighborCount++;
            }
        }
    }
    m_numNeighbors[index] = neighborCount;
}


inline void
PbdConstantDensityConstraint::calculateDensityEstimate(const Vec3d &pi,
                                                       const size_t &index,
                                                       const StdVectorOfVec3d &positions)
{
    double densitySum = 0.0;
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        densitySum += wPoly6(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]]);
    }

    m_densities[index] = densitySum;
}

inline void
PbdConstantDensityConstraint::calculateLambdaScalingFactor(const Vec3d &pi,
                                                           const size_t &index,
                                                           const StdVectorOfVec3d &positions)
{
    const double densityConstraint = (m_densities[index] / m_restDensity) - 1;
    double gradientSum = 0.0;
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        gradientSum += gradSpiky(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]]).squaredNorm() / m_restDensity;
    }

    m_lambdas[index] = densityConstraint / (gradientSum + m_relaxationParameter);
}

inline void
PbdConstantDensityConstraint::updatePositions(const Vec3d &pi,
                                              const size_t &index,
                                              StdVectorOfVec3d &positions)
{
    //Make sure the point is valid
    Vec3d gradientLambdaSum(0., 0., 0.);
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        double lambdasDiff = (m_lambdas[index] + m_lambdas[m_neighbors[index * m_maxNumNeighbors + j]]);
        Vec3d gradKernal = gradSpiky(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]]);
        gradientLambdaSum += (gradKernal * lambdasDiff);
    }

    m_deltaPositions[index] = gradientLambdaSum / m_restDensity;
    positions[index] += m_deltaPositions[index];
}
} // imstk