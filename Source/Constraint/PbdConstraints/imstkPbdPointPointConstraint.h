/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
////
/// \class PbdPointPointConstraint
///
/// \brief This constraint resolves two vertices to each other
///
class PbdPointPointConstraint : public PbdCollisionConstraint
{
public:
    PbdPointPointConstraint() : PbdCollisionConstraint(1, 1) { }
    ~PbdPointPointConstraint() override = default;

public:
    ///
    /// \brief initialize constraint
    ///
    void initConstraint(
        VertexMassPair ptA, VertexMassPair ptB,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdxA constraint gradient for A
    /// \param[inout] dcdxB constraint gradient for B
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override;
};
} // imstk