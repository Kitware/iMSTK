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
/// \brief The PbdPointTriangleConstraint moves a point to a triangle, and the triangle
/// to the point. Give zero mass to make one-sided/immovable
///
class PbdPointTriangleConstraint : public PbdCollisionConstraint
{
public:
    PbdPointTriangleConstraint() : PbdCollisionConstraint(1, 3) { }
    ~PbdPointTriangleConstraint() override = default;

public:
    ///
    /// \brief initialize constraint
    /// \param pIdxA1 index of the point from object1
    /// \param pIdxB1 first point of the triangle from object2
    /// \param pIdxB2 second point of the triangle from object2
    /// \param pIdxB3 third point of the triangle from object2
    /// \return
    ///
    void initConstraint(VertexMassPair ptA,
                        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
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