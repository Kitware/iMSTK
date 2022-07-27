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
/// \class PbdEdgeEdgeConstraint
///
/// \brief Pushes an edge "outside" the other edge
///
class PbdEdgeEdgeConstraint : public PbdCollisionConstraint
{
public:
    PbdEdgeEdgeConstraint() : PbdCollisionConstraint(2, 2) { }
    ~PbdEdgeEdgeConstraint() override = default;

public:
    ///
    /// \brief initialize constraint
    /// \return  true if succeeded
    ///
    void initConstraint(
        VertexMassPair ptA1, VertexMassPair ptA2,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[in] currVertexPositionsA current positions from object A
    /// \param[in] currVertexPositionsA current positions from object B
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override;
};
} // namespace imstk