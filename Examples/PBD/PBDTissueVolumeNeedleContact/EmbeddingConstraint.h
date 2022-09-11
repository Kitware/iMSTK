/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdContactConstraint.h"

namespace imstk
{
///
/// \brief Constraints a triangle and a rigid body needle,
/// via a point on the triangle described as a barycentric coordinate.
///
/// As the constraint is solved its looking to tell if its been unpunctured
/// or not. Movement along the triangle plane is fine but against the normal
/// if projected above/below the triangle it may exit.
///
class EmbeddingConstraint : public PbdContactConstraint
{
public:
    EmbeddingConstraint() : PbdContactConstraint(4) { }
    ~EmbeddingConstraint() override = default;

    ///
    /// \brief Initializes both PBD and RBD constraint
    /// \param bodies
    /// \param rigid body particle
    /// \param triangle particle b1
    /// \param triangle particle b2
    /// \param triangle particle b3
    /// \param p/start of line
    /// \param q/end of line
    ///
    void initConstraint(
        PbdState& bodies,
        const PbdParticleId& ptA1,                                                       // rigid body id
        const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3, // Triangle
        Vec3d* p, Vec3d* q,
        const double compliance = 0.0);                                                  // Needle line vertices

    ///
    /// \brief Given two interpolants on the two elements, compute the difference
    /// between them and use for resolution
    ///
    Vec3d computeInterpolantDifference(const PbdState& bodies) const;

    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

// Vec3d computeRelativeVelocity(PbdState& bodies) override;

    const Vec3d& getIntersectionPoint() const { return m_iPt; }
    Vec3d* getP() const { return m_p; }
    Vec3d* getQ() const { return m_q; }

protected:
    // Intersection point via interpolants on triangle
    Vec3d m_uvw = Vec3d::Zero();
    // Intersection point via interpolants on line
    Vec2d  m_uv = Vec2d::Zero();
    double t    = 0.0;

    // The actual line
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;

protected:
    Vec3d m_iPt;

protected:
    // If 0.0, completely removes pbd reaction in line axes direction, the pbd triangle will completely let
    // the tool slide in that direction
    // If 1.0, completely resist normal movement
    double m_normalFriction = 0.0;
};
} // namespace imstk