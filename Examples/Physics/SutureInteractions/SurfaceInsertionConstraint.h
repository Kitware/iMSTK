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
public:
    SurfaceInsertionConstraint() :  PbdCollisionConstraint(0, 3) { }
    ~SurfaceInsertionConstraint() override = default;

    void initConstraint(
        const Vec3d&         insertionPoint,
        const PbdParticleId& ptB1,
        const PbdParticleId& ptB2,
        const PbdParticleId& ptB3,
        const Vec3d&         contactPt,
        const Vec3d&         barycentricPt,
        double               stiffnessA,
        double               stiffnessB);

    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

private:
    Vec3d m_insertionPoint;
    Vec3d m_barycentricPt;
    Vec3d m_contactPt;
};