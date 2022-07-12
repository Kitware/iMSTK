/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdBody.h"

namespace imstk
{
using PbdParticleId = std::pair<int, int>;

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
    /// \param PbdState provides all the bodies
    /// \param Constraint value
    /// \param Normalized constraint gradients (per vertex)
    ///
    virtual bool computeValueAndGradient(PbdState& bodies,
                                         double& c, std::vector<Vec3d>& dcdx) = 0;

    ///
    /// \brief Get the vertex indices of the constraint
    ///
    std::vector<PbdParticleId>& getParticles() { return m_particles; }

    ///
    /// \brief Get/Set resitution
    ///@{
    double getRestitution() const { return m_restitution; }
    void setRestitution(const double restitution) { m_restitution = restitution; }
    ///@}

    ///
    /// \brief Get/Set friction
    ///@{
    double getFriction() const { return m_friction; }
    void setFriction(const double friction) { m_friction = friction; }
    ///@}

    ///
    /// \brief Get/Set the stiffness
    /// @{
    double getStiffness() const { return m_stiffness; }
    void setStiffness(const double stiffness)
    {
        m_stiffness = stiffness;
        CHECK(m_stiffness != 0.0) << "0 stiffness is invalid";
        // This is a bit ambigiuous
        m_compliance = 1.0 / stiffness;
    }

    /// @}

    ///
    /// \brief Get/Set the compliance
    /// This function is also provided in case users need 0 compliance
    /// @{
    double getCompliance() const { return m_compliance; }
    void setCompliance(const double compliance)
    {
        m_compliance = compliance;
        // 0 compliance implies infinite stiffness, instead set stiffness to 1.0
        // which is a convienent value for collision/unilateral constraints solved
        // under PBD
        m_stiffness = (m_compliance == 0.0) ? 1.0 : 1.0 / m_compliance;
    }

    /// @}

    ///
    /// \brief Get the force magnitude, valid after solving lambda
    /// Only valid with xpbd
    ///
    double getForce(const double dt) const { return m_lambda / (dt * dt); }

    ///
    /// \brief Zero's out the lagrange multplier before integration
    /// only used for xpbd, must be called before solving
    ///
    void zeroOutLambda() { m_lambda = 0.0; }

    ///
    /// \brief Update positions by projecting constraints.
    ///
    virtual void projectConstraint(PbdState& bodies, const double dt, const SolverType& type);

    ///
    /// \brief Correct velocities according to friction and restitution
    /// Corrects according to the gradient direction
    ///
    virtual void correctVelocity(PbdState& bodies, const double dt);

    ///
    /// \brief Compute generalized inverse mass of the particle
    /// \param bodies of the system
    /// \param Index of the particle in the constraint to compute from
    /// \param Optional local support point for which to cross when particle
    /// is oriented
    ///
    double computeGeneralizedInvMass(PbdState& bodies,
                                     const size_t particleIndex, const Vec3d& r = Vec3d::Zero()) const;

protected:
    PbdConstraint(const size_t numParticles)
    {
        m_particles.resize(numParticles);
        m_dcdx.resize(numParticles);
    }

    std::vector<PbdParticleId> m_particles; ///< body, particle index

    double m_stiffness  = 1.0;              ///< used in PBD, [0, 1]
    double m_compliance = 1e-7;             ///< used in xPBD, inverse of Young's Modulus
    double m_lambda     = 0.0;              ///< Lagrange multiplier

    std::vector<Vec3d> m_dcdx;              ///< Normalized constraint gradients (per particle)

    double m_friction    = 0.0;
    double m_restitution = 0.0;
};
} // namespace imstk