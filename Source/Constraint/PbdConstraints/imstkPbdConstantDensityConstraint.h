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

#pragma once

#include "imstkNeighborSearch.h"
#include "imstkPbdConstraint.h"
#include "imstkTypes.h"

namespace imstk
{
///
/// \class PbdConstantDensityConstraint
///
/// \brief Implements the constant density constraint to simulate fluids.
/// This constraint is global and applied to all vertices passed in during projection
///
class PbdConstantDensityConstraint : public PbdConstraint
{
public:
    PbdConstantDensityConstraint() : PbdConstraint()
    {
        setMaxNeighborDistance(m_maxDist);
    }

    ///
    /// \brief Constant Density Constraint Initialization
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions, const double k);

    ///
    /// \brief Returns PBD constraint of type ConstantDensity
    ///
    std::string getType() const override { return "ConstantDensity"; }

    ///
    /// \brief Solves the constant density constraint
    ///
    void projectConstraint(const DataArray<double>& currInvMasses,
                           const double dt,
                           const PbdConstraint::SolverType& type,
                           VecDataArray<double, 3>& currVertexPositions) override;

    bool computeValueAndGradient(
        const VecDataArray<double, 3>& imstkNotUsed(currVertexPositions),
        double& imstkNotUsed(c),
        std::vector<Vec3d>& imstkNotUsed(dcdx)) const override
    {
        return true;
    }

private:
    ///
    /// \brief Smoothing kernel WPoly6 for density estimation
    ///
    inline double wPoly6(const Vec3d& pi, const Vec3d& pj) const
    {
        const double rLengthSqr = (Vec3d(pi - pj)).squaredNorm();
        if (rLengthSqr > m_maxDistSqr || rLengthSqr < 1e-20)
        {
            return 0.0;
        }
        else
        {
            const double maxDiff = m_maxDistSqr - rLengthSqr;
            return m_wPoly6Coeff * maxDiff * maxDiff * maxDiff;
        }
    }

    ///
    /// \brief Gradient of density kernel
    ///
    inline Vec3d gradSpiky(const Vec3d& pi, const Vec3d& pj) const
    {
        const Vec3d  r = pi - pj;
        const double rLengthSqr = r.squaredNorm();

        if (rLengthSqr > m_maxDistSqr || rLengthSqr < 1e-20)
        {
            return Vec3d::Zero();
        }

        const double rLength = std::sqrt(rLengthSqr);
        return r * (m_wSpikyCoeff * (m_maxDist - rLength) * (m_maxDist - rLength));
    }

    ///
    /// \brief
    ///
    void computeDensity(const Vec3d& pi, const size_t index, const VecDataArray<double, 3>& positions);

    ///
    /// \brief
    ///
    void computeLambdaScalingFactor(const Vec3d& pi, const size_t index, const VecDataArray<double, 3>& positions);

    ///
    /// \brief
    ///
    void updatePositions(const Vec3d& pi, const size_t index, VecDataArray<double, 3>& positions);

    ///
    /// \brief Set/Get rest density
    ///
    void setDensity(const double density) { m_restDensity = density; }
    double getDensity() const { return m_restDensity; }

    ///
    /// \brief Set/Get max. neighbor distance
    ///
    void setMaxNeighborDistance(const double dist);
    double getMaxNeighborDistance() const { return m_maxDist; }

    ///
    /// \brief Set/Get neighbor search method
    ///
    void setNeighborSearchMethod(NeighborSearch::Method method) { m_NeighborSearchMethod = method; }
    NeighborSearch::Method getNeighborSearchMethod() const { return m_NeighborSearchMethod; }

private:
    double m_wPoly6Coeff;
    double m_wSpikyCoeff;

    double m_maxDist    = 0.2;                       ///> Max. neighbor distance
    double m_maxDistSqr = 0.04;                      ///> Max. neighbor squared distance
    double m_relaxationParameter = 600.0;            ///> Relaxation parameter
    double m_restDensity = 6378.0;                   ///> Fluid density

    std::vector<double> m_lambdas;                   ///> lambdas
    std::vector<double> m_densities;                 ///> densities
    std::vector<Vec3d>  m_deltaPositions;            ///> delta positions
    std::vector<std::vector<size_t>> m_neighborList; ///> indices of neighbor particles

    NeighborSearch::Method m_NeighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
    std::shared_ptr<NeighborSearch> m_NeighborSearcher;  ///> neighbor searcher, must be initialized during model initialization
};
}
