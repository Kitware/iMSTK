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
    auto state = model.getCurrentState();
    auto np = state->getPositions().size();

    //constraint parameters
    m_maxNumNeighbors = 100;
    m_maxDist = 0.2;
    m_relaxationParameter = 600.0;
    m_restDensity = 6378.0;
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
    auto state = model.getCurrentState();
    auto& pos = state->getPositions();
    auto np = pos.size();

    ClearNeighbors(np);

    //This loop should be replaced with parallellization
    /*for (int index = 0; index < np; index++)
        {
                PointTable(pos[index], index);
        }*/
    for (int index = 0; index < np; index++)
    {
        //UpdateNeighbors(index, pos);
        UpdateNeighborsBruteForce(pos[index], index, pos);
    }
    for (int index = 0; index < np; index++)
    {
        CalculateDensityEstimate(pos[index], index, pos);
    }
    for (int index = 0; index < np; index++)
    {
        CalculateLambdaScalingFactor(pos[index], index, pos);
    }
    for (int index = 0; index < np; index++)
    {
        DeltaPosition(pos[index], index, pos);
    }

    for (int index = 0; index < np; index++)
    {
        UpdatePositions(pos[index], index);
    }

    return true;
}

//Smoothing Kernal WPoly6 for density estimation
inline double
PbdConstantDensityConstraint::WPoly6(const Vec3d &pi, const Vec3d &pj)
{
    Vec3d r = pi - pj;
    double rLength = Length(r);
    if (rLength > m_maxDist || rLength == 0)
    {
        return 0;
    }

    return m_wPoly6Coeff * pow((m_maxDist * m_maxDist) - (rLength * rLength), 3);
}

//Smoothing Kernal Spiky for gradient calculation
inline double
PbdConstantDensityConstraint::WSpiky(const Vec3d &pi, const Vec3d &pj)
{
    Vec3d r = pi - pj;
    double rLength = Length(r);
    if (rLength > m_maxDist || rLength == 0)
    {
        return 0;
    }

    return m_wSpikyCoeff * pow(m_maxDist - rLength, 3);
}

inline Vec3d
PbdConstantDensityConstraint::GradSpiky(const Vec3d &pi, const Vec3d &pj)
{
    Vec3d r = pi - pj;
    Vec3d zero;
    zero[0] = 0;
    zero[1] = 0;
    zero[2] = 0;
    double rLength = Length(r);
    if (rLength > m_maxDist || rLength == 0)
    {
        return zero;
    }

    return r * (m_wSpikyCoeff * (-3.0) * (m_maxDist - rLength) * (m_maxDist - rLength));
}

inline double
PbdConstantDensityConstraint::Length(const Vec3d &p1)
{
    return sqrt(p1[0] * p1[0] + p1[1] * p1[1] + p1[2] * p1[2]);
}

inline void
PbdConstantDensityConstraint::ClearNeighbors(const int &np)
{
    m_numNeighbors.clear();
    m_neighbors.clear();
    m_numNeighbors.resize(np);
    m_neighbors.resize(np * m_maxNumNeighbors);
}

inline void
PbdConstantDensityConstraint::PointTable(const Vec3d &pi, const int &index)
{
    m_xPosIndexes[index] = (pi[0] - pi[0] * m_maxDist) / m_maxDist;
    m_yPosIndexes[index] = (pi[1] - pi[1] * m_maxDist) / m_maxDist;
    m_zPosIndexes[index] = (pi[2] - pi[2] * m_maxDist) / m_maxDist;
}

inline void
PbdConstantDensityConstraint::UpdateNeighbors(const int &index, const StdVectorOfVec3d &positions)
{
    int ip = m_xPosIndexes[index];
    int jp = m_yPosIndexes[index];
    int kp = m_zPosIndexes[index];

    int np = m_zPosIndexes.size();
    int neighborCount = 0;

    int ibound = ip - 2;
    if (ibound < 0)
    {
        ibound = 0;
    }
    int jbound = jp - 2;
    if (jbound < 0)
    {
        jbound = 0;
    }
    int kbound = kp - 2;
    if (kbound < 0)
    {
        kbound = 0;
    }
    for (int i = 0; i < np; i++)
    {
        if (neighborCount >= m_maxNumNeighbors || i == index)
        {
            continue;
        }
        if (m_xPosIndexes[i] > ibound && m_xPosIndexes[i] < (ip + 2))
        {
            if (m_yPosIndexes[i] > jbound && m_yPosIndexes[i] < (jp + 2))
            {
                if (m_zPosIndexes[i] > kbound && m_zPosIndexes[i] < (kp + 2))
                {
                    m_neighbors[index * m_maxNumNeighbors + neighborCount] = i;
                    neighborCount++;
                }
            }
        }
    }
    m_numNeighbors[index] = neighborCount;
}

//brute Force
inline
void PbdConstantDensityConstraint::UpdateNeighborsBruteForce(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions)
{
    double neighborRadius = 2 * m_maxDist;
    Vec3d r;
    double rLength;
    int neighborCount = 0;
    //loop over all points
    for (int j = 0; j < positions.size(); j++)
    {
        if (j != index)
        {
            if (neighborCount >= m_maxNumNeighbors)
            {
                continue;
            }
            r = pi - positions[j];
            rLength = Length(r);
            if (rLength < neighborRadius)
            {
                m_neighbors[index * m_maxNumNeighbors + neighborCount] = j;
                neighborCount++;
            }
        }
    }
    m_numNeighbors[index] = neighborCount;
}


inline void
PbdConstantDensityConstraint::CalculateDensityEstimate(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions)
{
    double densitySum = 0.0;
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        densitySum += WPoly6(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]]);
    }

    m_densities[index] = densitySum;
}

inline void
PbdConstantDensityConstraint::CalculateLambdaScalingFactor(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions)
{
    double densityConstraint = (m_densities[index] / m_restDensity) - 1;
    double gradientSum = 0.0;
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        gradientSum += pow(Length(GradSpiky(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]])), 2) / m_restDensity;
    }

    m_lambdas[index] = densityConstraint / (gradientSum + m_relaxationParameter);
}

inline void
PbdConstantDensityConstraint::DeltaPosition(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions)
{
    //Make sure the point is valid
    Vec3d gradientLambdaSum;
    gradientLambdaSum[0] = 0;
    gradientLambdaSum[1] = 0;
    gradientLambdaSum[2] = 0;
    for (int j = 0; j < m_numNeighbors[index]; j++)
    {
        int neighborInd = m_neighbors[index * m_maxNumNeighbors + j];
        double lambdasDiff = (m_lambdas[index] + m_lambdas[m_neighbors[index * m_maxNumNeighbors + j]]);
        Vec3d gradKernal = GradSpiky(pi, positions[m_neighbors[index * m_maxNumNeighbors + j]]);
        gradientLambdaSum += (gradKernal * lambdasDiff);
    }

    m_deltaPositions[index] = gradientLambdaSum / m_restDensity;
}

inline void
PbdConstantDensityConstraint::UpdatePositions(Vec3d &pi, const int &index)
{
    //Make sure the point is valid
    pi[0] += m_deltaPositions[index][0];
    pi[1] += m_deltaPositions[index][1];
    pi[2] += m_deltaPositions[index][2];
}
} // imstk