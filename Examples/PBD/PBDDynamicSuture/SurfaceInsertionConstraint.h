/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

using namespace imstk;

///
/// \class SurfaceInsertionConstraint
///
/// \brief Constrains a barycentric point on a surface mesh to a rigid body arc needle
///
class SurfaceInsertionConstraint : public PbdCollisionConstraint
{
private:

    Vec3d m_insertionPoint;
    Vec3d m_barycentricPt;
    Vec3d m_contactPt;

public:

    ///
    /// \param the Rigid body needle
    /// \param
    ///
    SurfaceInsertionConstraint() :  PbdCollisionConstraint(1, 3)
    {
    }

    ~SurfaceInsertionConstraint() override = default;

public:

    void initConstraint(
        Vec3d          insertionPoint,
        VertexMassPair ptB1,
        VertexMassPair ptB2,
        VertexMassPair ptB3,
        Vec3d          contactPt,
        Vec3d          barycentricPt,
        double         stiffnessA,
        double         stiffnessB);

    bool computeValueAndGradient(
        double&             c,
        std::vector<Vec3d>& dcdxA,
        std::vector<Vec3d>& dcdxB) const override;
};