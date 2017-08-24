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

namespace imstk
{
///
/// \class PbdConstantDensityConstraint
///
/// \brief Implements the constant density constraint to simulate fluids
///
class PbdConstantDensityConstraint : public PbdConstraint
{
public:
    ///
    /// \brief constructor
    ///
    PbdConstantDensityConstraint() : PbdConstraint() {}

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
    ///
    /// \brief Smoothing kernel WPoly6 for density estimation
    ///
    double wPoly6(const Vec3d &pi, const Vec3d &pj);

    ///
    /// \brief Smoothing kernel Spiky for gradient calculation
    ///
    double wSpiky(const Vec3d &pi, const Vec3d &pj);

    ///
    /// \brief
    ///
    Vec3d gradSpiky(const Vec3d &pi, const Vec3d &pj);

    ///
    /// \brief Update the neighbors of each node using burte force search O(n*n)
    ///
    void updateNeighborsBruteForce(const Vec3d &pi, const size_t index, const StdVectorOfVec3d &positions);

    ///
    /// \brief Clear the list of neighbors
    ///
    void clearNeighbors(const size_t np);

    ///
    /// \brief
    ///
    void calculateDensityEstimate(const Vec3d &pi, const size_t index, const StdVectorOfVec3d &positions);

    ///
    /// \brief
    ///
    void calculateLambdaScalingFactor(const Vec3d &pi, const size_t index, const StdVectorOfVec3d &positions);

    ///
    /// \brief
    ///
    void updatePositions(const Vec3d &pi, const size_t index, StdVectorOfVec3d &positions);

    ///
    /// \brief Set/Get rest density
    ///
    void setDensity(const double density) { m_restDensity = density; }
    double getDensity() { return m_restDensity; }

    ///
    /// \brief Set/Get max. neighbor distance
    ///
    void setMaxNeighborDistance(const double dist) { m_maxDist = dist; }
    double getMaxNeighborDistance() { return m_restDensity; }

private:
    double m_wPoly6Coeff;
    double m_wSpikyCoeff;

    double m_maxDist = 0.2;                 ///> Max. neighbor distance
    double m_relaxationParameter = 600.0;   ///> Relaxation parameter
    double m_restDensity = 6378.0;          ///> Fluid density
    int m_maxNumNeighbors = 50;             ///> Max. number of neighbors

    std::vector<double> m_lambdas;          ///> lambdas
    std::vector<double> m_densities;        ///> densities
    std::vector<Vec3d> m_deltaPositions;    ///> delta positions
    std::vector<int> m_neighbors;           ///> index of neighbors
    std::vector<int> m_numNeighbors;        ///> number of neighbors
};
}

#endif // imstkPbdConstantDensityConstraint_h
