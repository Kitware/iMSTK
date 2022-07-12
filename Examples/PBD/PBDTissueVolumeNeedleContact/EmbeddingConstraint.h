/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"
#include "imstkRbdConstraint.h"

namespace imstk
{
///
/// \brief Given a line and a triangle it computes the intersection between
/// them upon initialation for which it saves the interpolation weights.
///
/// This allows us to deform both shapes whilst maintaining a relative
/// position on that element.
///
/// To then constrain we compute the difference between the two interpolated
/// positions on each element and pull the line back towards the triangle
/// via rbd constraint and pull the triangle back towards the line via pbd
///
/// A compliance term gives the weighting for which to do this. To make more
/// physically accurate we would need to take an approach like XPBD in solving.
///
class EmbeddingConstraint : public PbdCollisionConstraint, public RbdConstraint
{
public:
    EmbeddingConstraint(std::shared_ptr<RigidBody> obj1) :
        PbdCollisionConstraint(3, 0), RbdConstraint(obj1, nullptr, RbdConstraint::Side::A)
    {
    }

    ~EmbeddingConstraint() override = default;

public:
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
        const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3,
        Vec3d* p, Vec3d* q);

    ///
    /// \brief Given two interpolants on the two elements, compute the difference
    /// between them and use for resolution
    ///
    Vec3d computeInterpolantDifference(const PbdState& bodies) const;

    ///
    /// \brief Update the pbd constraint
    ///
    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& dcdx) override;

    ///
    /// \brief Update the rbd constraint
    ///
    void compute(double dt) override;

protected:
    // Intersection point via interpolants on triangle
    Vec3d m_uvw = Vec3d::Zero();
    // Intersection point via interpolants on line
    double m_t = 0.0;

    // The actual line
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;

protected:
    // Step for rbd constraint
    double m_beta = 0.05;

    Vec3d m_iPt;
    Vec3d m_iPtVel;

protected:
    // Ratio between the two models (ie: how much rbd tool is moved vs how much pbd tissue is)
    //
    // If 0.0, rbd tool is completely resolved and PBD tissue does not move
    // If 1.0, pbd tissue completely moves and rbd tool feels no resistance
    double m_compliance0 = 0.5;

    // If 0.0, completely removes pbd reaction in line axes direction, the pbd triangle will completely let
    // the tool slide in that direction
    // If 1.0, completely resist normal movement
    double m_normalFriction = 0.0;

    PbdState* m_state = nullptr; ///< Temporary to handle issue of multi body constraints with bodies
};
} // namespace imstk