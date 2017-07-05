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

#ifndef imstkPbdConstantDensityConstraint_h
#define imstkPbdConstantDensityConstraint_h

#include "imstkPbdConstraint.h"
#include "../Collision/CollisionDetection/DataStructures/imstkSpatialHashTableSeparateChaining.h"

namespace imstk
{
class PbdConstantDensityConstraint : public PbdConstraint
{
public:
    ///
    /// \brief constructor
    ///
    PbdConstantDensityConstraint() : PbdConstraint() { m_vertexIds.resize(1); }
    ///
    /// \Constant Density Constraint Initialization
    ///
    void initConstraint(PbdModel& model, const double k);

    ///
    /// \brief Returns PBD constraint of type Type::ConstantDensity
    ///
    Type getType() const { return Type::ConstantDensity; }

    ///
    /// \brief Solves the constant density constraint
    ///
    bool solvePositionConstraint(PbdModel &model);

private:
    double WPoly6(const Vec3d &pi, const Vec3d &pj);
    double WSpiky(const Vec3d &pi, const Vec3d &pj);
    Vec3d GradSpiky(const Vec3d &pi, const Vec3d &pj);
    double Length(const Vec3d &);

    void PointTable(const Vec3d &pi, const int &index);
    void UpdateNeighbors(const int &index, const StdVectorOfVec3d &positions);
    void UpdateNeighborsBruteForce(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions);
    void ClearNeighbors(const int &np);
    void CalculateDensityEstimate(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions);
    void CalculateLambdaScalingFactor(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions);
    void DeltaPosition(const Vec3d &pi, const int &index, const StdVectorOfVec3d &positions);
    void UpdatePositions(Vec3d &pi, const int &index);

private:
    double m_wPoly6Coeff;
    double m_wSpikyCoeff;
    double m_maxDist;
    double m_relaxationParameter;
    double m_restDensity;
    int m_maxNumNeighbors;

    std::vector<double> m_lambdas;             ///> lambdas
    std::vector<double> m_densities;           ///> densities
    std::vector<Vec3d> m_deltaPositions;       ///> delta positions
    std::vector<int> m_neighbors;              ///> index of neighbors
    std::vector<int> m_numNeighbors;           ///> number of neighbors
    std::vector<int> m_xPosIndexes;
    std::vector<int> m_yPosIndexes;
    std::vector<int> m_zPosIndexes;
};
}

#endif // imstkPbdConstantDensityConstraint_h
