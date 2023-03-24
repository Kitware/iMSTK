/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    PbdConstantDensityConstraint();

    IMSTK_TYPE_NAME(PbdConstantDensityConstraint)

    ///
    /// \brief Constant Density Constraint Initialization
    /// \param Bodies containing all the particles
    /// \param the body index to simulate as a fluid
    /// \param stiffness
    ///
    void initConstraint(const int    numParticles,
                        const int    bodyHandle,
                        const double particleRadius,
                        const double density);

    ///
    /// \brief Solves the constant density constraint
    ///
    void projectConstraint(PbdState& bodies, const double dt,
                           const SolverType& type) override;

    bool computeValueAndGradient(
        PbdState&           imstkNotUsed(bodies),
        double&             imstkNotUsed(c),
        std::vector<Vec3d>& imstkNotUsed(dcdx)) override
    {
        return true;
    }

    ///
    /// \brief Set/Get rest density
    ///@{
    void setDensity(const double density) { m_restDensity = density; }
    double getDensity() const { return m_restDensity; }
///@}

private:
    ///
    /// \brief Smoothing kernel WPoly6 for density estimation
    ///
    inline double wPoly6(const Vec3d& pi, const Vec3d& pj) const
    {
        const double rLengthSqr = (Vec3d(pi - pj)).squaredNorm();
        if (rLengthSqr > m_particleRadiusSqr || rLengthSqr == 0.0)
        {
            return 0.0;
        }
        else
        {
            const double maxDiff = m_particleRadiusSqr - rLengthSqr;
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

        if (rLengthSqr > m_particleRadiusSqr || rLengthSqr < 1e-20)
        {
            return Vec3d::Zero();
        }

        const double rLength = std::sqrt(rLengthSqr);
        return r * (m_wSpikyCoeff * (m_particleRadius - rLength) * (m_particleRadius - rLength));
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
    /// \brief Set/Get neighbor search method
    ///
    void setNeighborSearchMethod(NeighborSearch::Method method) { m_NeighborSearchMethod = method; }
    NeighborSearch::Method getNeighborSearchMethod() const { return m_NeighborSearchMethod; }

    ///
    /// \brief Return the rest configuration for the constraint
    ///
    double getRestValue() const { return m_restDensity; }

private:
    int    m_bodyHandle  = -1;
    double m_wPoly6Coeff = 0.0;
    double m_wSpikyCoeff = 0.0;

    double m_particleRadius      = 0.2;              ///< Max. neighbor distance
    double m_particleRadiusSqr   = 0.04;             ///< Max. neighbor squared distance
    double m_relaxationParameter = 600.0;            ///< Relaxation parameter
    double m_restDensity = 6378.0;                   ///< Fluid density

    std::vector<double> m_lambdas;                   ///< lambdas
    std::vector<double> m_densities;                 ///< densities
    std::vector<Vec3d>  m_deltaPositions;            ///< delta positions
    std::vector<std::vector<size_t>> m_neighborList; ///< indices of neighbor particles

    NeighborSearch::Method m_NeighborSearchMethod = NeighborSearch::Method::UniformGridBasedSearch;
    std::shared_ptr<NeighborSearch> m_NeighborSearcher;  ///< neighbor searcher, must be initialized during model initialization
};
} // namespace imstk
