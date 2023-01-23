/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

using namespace imstk;

///
/// \class ThreadInsertionConstraint
///
/// \brief Constrains an point on a line mesh to a PBD surface mesh
///
class ThreadInsertionConstraint : public PbdCollisionConstraint
{
public:
    ThreadInsertionConstraint() :  PbdCollisionConstraint(2, 3) { }
    ~ThreadInsertionConstraint() override = default;

    void initConstraint(
        const PbdState&      bodies,
        const PbdParticleId& ptA1,
        const PbdParticleId& ptA2,
        const Vec2d&         threadBaryPoint,
        const PbdParticleId& ptB1,
        const PbdParticleId& ptB2,
        const PbdParticleId& ptB3,
        const Vec3d&         triBaryPoint,
        double               stiffnessA,
        double               stiffnessB);

    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    Vec2d m_threadBaryPt;
    Vec3d m_triangleBaryPt;
    Vec3d m_triInsertionPoint;
    Vec3d m_threadInsertionPoint;
};