/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSurfaceInsertionConstraint.h"

using namespace imstk;

void
SurfaceInsertionConstraint::initConstraint(
    const Vec3d&         insertionPoint,
    const PbdParticleId& ptN,
    const PbdParticleId& ptB1,
    const PbdParticleId& ptB2,
    const PbdParticleId& ptB3,
    const Vec3d&         contactPt,
    const Vec3d&         barycentricPt,
    double               stiffnessA,
    double               stiffnessB)
{
    m_insertionPoint = insertionPoint;
    m_contactPt      = contactPt;
    m_barycentricPt  = barycentricPt;

    m_particles[0] = ptN;
    m_particles[1] = ptB1;
    m_particles[2] = ptB2;
    m_particles[3] = ptB3;

    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
SurfaceInsertionConstraint::computeValueAndGradient(PbdState&,
                                                    double& c, std::vector<Vec3d>& dcdx)
{
    // Get current position of puncture point
    // Move triangle to match motion of needle

    Vec3d diff = m_contactPt - m_insertionPoint;

    c = diff.norm();

    // If sufficiently close, do not solve constraint
    if (c < 1E-8)
    {
        return false;
    }

    diff.normalize();// gradient dcdx

    // Dont adjust position of needle, force mesh to follow needle
    dcdx[0] = -1.0 * diff;// Vec3d::Zero(); //  Vec3d::Zero(); // Not two-way

    // Weight by berycentric coordinates
    dcdx[1] = diff * m_barycentricPt[0];
    dcdx[2] = diff * m_barycentricPt[1];
    dcdx[3] = diff * m_barycentricPt[2];

    return true;
}
