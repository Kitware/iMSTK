/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdPointTriangleConstraint
///
/// \brief The PbdPointTriangleConstraint moves a point to a triangle, and
/// the triangle to the point.
///
class PbdPointTriangleConstraint : public PbdCollisionConstraint
{
public:
    PbdPointTriangleConstraint() : PbdCollisionConstraint(1, 3) { }
    ~PbdPointTriangleConstraint() override = default;

    IMSTK_TYPE_NAME(PbdTriangleConstraint)

public:
    ///
    /// \brief Initialize the constraint
    /// \param ptA Point to resolve to triangle
    /// \param point of triangle B
    /// \param point of triangle B
    /// \param point of triangle B
    /// \param stiffness of A
    /// \param stiffness of B
    ///
    void initConstraint(const PbdParticleId& ptA,
                        const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3,
                        double stiffnessA, double stiffnessB);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;
};
} // namespace imstk