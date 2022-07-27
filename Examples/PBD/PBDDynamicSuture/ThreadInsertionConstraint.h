/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"
#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class ThreadInsertionConstraint
///
/// \brief Constrains an point on a line mesh to a PBD surface mesh
///
class ThreadInsertionConstraint : public PbdCollisionConstraint
{
private:

    Vec2d m_threadBaryPt;
    Vec3d m_triangleBaryPt;
    Vec3d m_triInsertionPoint;
    Vec3d m_threadInsertionPoint;

public:

    ThreadInsertionConstraint() :  PbdCollisionConstraint(2, 3) // (num thread verts, num triangle verts)
    {
    }

    ~ThreadInsertionConstraint() override = default;

public:

    void initConstraint(
        VertexMassPair ptA1,
        VertexMassPair ptA2,
        Vec2d          threadBaryPoint,
        VertexMassPair ptB1,
        VertexMassPair ptB2,
        VertexMassPair ptB3,
        Vec3d          triBaryPoint,
        double         stiffnessA,
        double         stiffnessB);

    bool computeValueAndGradient(
        double&             c,
        std::vector<Vec3d>& dcdxA,
        std::vector<Vec3d>& dcdxB) const override;
};