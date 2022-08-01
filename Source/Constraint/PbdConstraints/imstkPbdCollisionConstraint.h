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
/// \struct VertexMassPair
///
/// \brief Represents a point in a PBD mesh with a mass & velocity
///
struct VertexMassPair
{
    Vec3d* vertex   = nullptr;
    double invMass  = 0.0;
    Vec3d* velocity = nullptr;
};

///
/// \class PbdCollisionConstraint
///
/// \brief Abstract base class for collision constraints. Collision
/// constraints are non penetration constraints solved with PBD.
/// They are formulated with a gradient dcdx and scalar c which must
/// be returned in computeValueAndGradient.
/// These constraints differ from PbdConstraints in that they do not
/// support XPBD (as they are meant to be solved completely) & they
/// are meant to be generated during runtime quickly upon contacts.
///
class PbdCollisionConstraint
{
public:
    ///
    /// \brief Construct with the number of vertices from A and B for each side
    ///
    PbdCollisionConstraint(const unsigned int numVertsA, const unsigned int numVertsB);
    virtual ~PbdCollisionConstraint() = default;

public:
    ///
    /// \brief Get vertex, mass, velocities of constrained objects
    ///@{
    const std::vector<VertexMassPair>& getVertexIdsFirst() const { return m_bodiesFirst; }
    const std::vector<VertexMassPair>& getVertexIdsSecond() const { return m_bodiesSecond; }
    ///@}

    ///
    /// \brief Get stiffness
    ///@{
    const double getStiffnessA() const { return m_stiffnessA; }
    const double getStiffnessB() const { return m_stiffnessB; }
    ///@}

    ///
    /// \brief Get enableBoundaryCollision
    ///@{
    void setEnableBoundaryCollisions(const double enableBoundaryCollisions) { m_enableBoundaryCollisions = enableBoundaryCollisions; }
    const double getEnableBoundaryCollisions() const { return m_enableBoundaryCollisions; }
    ///@}

    ///
    /// \brief compute value and gradient of constraint function
    /// \param Constraint value
    /// \param Normalized constraint gradients of A (per vertex)
    /// \param Normalized constraint gradients of B (per vertex)
    ///
    virtual bool computeValueAndGradient(double&             c,
                                         std::vector<Vec3d>& dcdxA,
                                         std::vector<Vec3d>& dcdxB) const = 0;

    ///
    /// \brief Solve the positions given to the constraint
    ///
    virtual void solvePosition();

    ///
    /// \brief Solve the velocities given to the constraint
    ///
    virtual void correctVelocity(const double friction, const double restitution);

protected:
    std::vector<VertexMassPair> m_bodiesFirst;  ///< index of points for the first object
    std::vector<VertexMassPair> m_bodiesSecond; ///< index of points for the second object

    double m_stiffnessA = 1.0;
    double m_stiffnessB = 1.0;

    std::vector<Vec3d> m_dcdxA;              ///< Normalized constraint gradients (per vertex)
    std::vector<Vec3d> m_dcdxB;              ///< Normalized constraint gradients (per vertex)

    bool m_enableBoundaryCollisions = false; ///< enables boundary collisions, only use with 2 way coupling
};
} // namespace imstk
