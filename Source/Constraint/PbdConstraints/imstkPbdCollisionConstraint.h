/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdCollisionConstraint
///
/// \brief The PbdCollisionConstraint implements two sided collision.
/// This allows the usage of differing stiffness for each side which can
/// be useful during solve. Due to differences in definition, collisions
/// do not use XPBD. Only PBD. They are assumed perfectly rigid even though
/// stiffness is modifiable. Given enough iterations in the solve, it will
/// converge to perfectly rigid.
///
/// Collision constraint also provides a correctVelocity function.
/// This may be overriden but by default it will correct velocity along
/// the gradient tangents and normal according to frictional and restitution
/// coefficients.
///
class PbdCollisionConstraint : public PbdConstraint
{
public:
    ~PbdCollisionConstraint() override = default;

public:
    ///
    /// \brief Get/Set stiffness A or B
    ///@{
    double getStiffnessA() const { return m_stiffness[0]; }
    void setStiffnessA(const double stiffnessA) { m_stiffness[0] = stiffnessA; }
    double getStiffnessB() const { return m_stiffness[1]; }
    void setStiffnessB(const double stiffnessB) { m_stiffness[1] = stiffnessB; }
    ///@}

    ///
    /// \brief Get enableBoundaryCollision
    ///@{
    void setEnableBoundaryCollisions(const double enableBoundaryCollisions) { m_enableBoundaryCollisions = enableBoundaryCollisions; }
    const double getEnableBoundaryCollisions() const { return m_enableBoundaryCollisions; }
    ///@}

    ///
    /// \brief Performs the actual positional solve
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;

protected:
    PbdCollisionConstraint(const int numParticlesA, const int numParticlesB);

    std::vector<bool>     m_bodiesSides; ///< Stores 0 or 1 to indicate side of particle
    std::array<double, 2> m_stiffness = { 1.0, 1.0 };
    /// Enables boundary collisions, turned off by default due to the edge cases present
    /// when a point is fixed/infinite mass that can cause instabilities
    bool m_enableBoundaryCollisions = false;
};
} // namespace imstk
