/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkVecDataArray.h"

namespace imstk
{
///
/// \class PbdConstraint
///
/// \brief Base Constraint class for Position based dynamics constraints
///
class PbdConstraint
{
public:
    ///
    /// \brief Type of solvers
    ///
    enum class SolverType
    {
        xPBD = 0,
        PBD
    };

    PbdConstraint() = default;

    virtual ~PbdConstraint() = default;

    ///
    /// \brief Compute value and gradient of the constraint
    /// \param Vertex positions of particles
    /// \param Constraint value
    /// \param Normalized constraint gradients (per vertex)
    ///
    virtual bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const = 0;

    ///
    /// \brief Get the vertex indices of the constraint
    ///
    std::vector<size_t>& getVertexIds() { return m_vertexIds; }

    ///
    /// \brief Set the tolerance used for pbd constraints
    ///
    void setTolerance(const double eps) { m_epsilon = eps; }

    ///
    /// \brief Get the tolerance used for pbd constraints
    ///
    double getTolerance() const { return m_epsilon; }

    ///
    /// \brief  Set the stiffness
    ///
    void setStiffness(const double stiffness)
    {
        m_stiffness  = stiffness;
        m_compliance = 1.0 / stiffness;
    }

    ///
    /// \brief  Get the stiffness
    ///
    double getStiffness() const { return m_stiffness; }

    ///
    /// \brief Use PBD
    ///
    void zeroOutLambda() { m_lambda = 0.0; }

    ///
    /// \brief Update positions by projecting constraints.
    ///
    virtual void projectConstraint(const DataArray<double>& currInvMasses, const double dt, const SolverType& type, VecDataArray<double, 3>& pos);

protected:
    std::vector<size_t> m_vertexIds;   ///< index of points for the constraint
    double m_epsilon        = 1.0e-16; ///< Tolerance used for the costraints
    double m_stiffness      = 1.0;     ///< used in PBD, [0, 1]
    double m_compliance     = 1e-7;    ///< used in xPBD, inverse of Young's Modulus
    mutable double m_lambda = 0.0;     ///< Lagrange multiplier

    std::vector<Vec3d> m_dcdx;         ///< Normalized constraint gradients (per vertex)
};
} // namespace imstk