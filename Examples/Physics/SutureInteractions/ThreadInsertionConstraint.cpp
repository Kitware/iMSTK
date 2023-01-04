/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "ThreadInsertionConstraint.h"

using namespace imstk;

void
ThreadInsertionConstraint::initConstraint(
    const PbdState&      bodies,
    const PbdParticleId& ptA1,
    const PbdParticleId& ptA2,
    const Vec2d&         threadBaryPoint,
    const PbdParticleId& ptB1,
    const PbdParticleId& ptB2,
    const PbdParticleId& ptB3,
    const Vec3d&         triBaryPoint,
    double               stiffnessA,
    double               stiffnessB)
{
    // Vertex mass pairs for thread
    m_particles[0] = ptA1;
    m_particles[1] = ptA2;

    // Barycentric coordinate on thread of intersection point
    m_threadBaryPt = threadBaryPoint;

    // Computing world coordinates of intersecting point along thread
    m_threadInsertionPoint = m_threadBaryPt[0] * bodies.getPosition(m_particles[0])
                             + m_threadBaryPt[1] * bodies.getPosition(m_particles[1]);

    // Vertex mass pairs for triangle
    m_particles[2] = ptB1;
    m_particles[3] = ptB2;
    m_particles[4] = ptB3;

    // Barycentric coordinate of puncture point on triangle
    m_triangleBaryPt = triBaryPoint;

    // Computing world coordinates of puncture point
    m_triInsertionPoint = m_triangleBaryPt[0] * bodies.getPosition(m_particles[2])
                          + m_triangleBaryPt[1] * bodies.getPosition(m_particles[3])
                          + m_triangleBaryPt[2] * bodies.getPosition(m_particles[4]);

    // Saving stiffness
    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
ThreadInsertionConstraint::computeValueAndGradient(PbdState& bodies,
                                                   double& c, std::vector<Vec3d>& dcdx)
{
    // \todo: No reprojection is done here so smooth multi iteration solve is not possible
    // two-way solve not possible

    // Move thread such that the thread stays intersected with the
    // puncture point on the triangle

    Vec3d diff = m_triInsertionPoint - m_threadInsertionPoint;  // gradient dcdx
    c = diff.norm();

    // If sufficiently close, do not solve constraint
    if (c < 1E-8)
    {
        c = 0.0;
        return false;
    }
    diff.normalize();

    // Move thread to follow insertion point
    dcdx[0] = diff * m_threadBaryPt[0];
    dcdx[1] = diff * m_threadBaryPt[1];

    // Move triangle to follow thread point (WARNING: Currently inactive)
    dcdx[2] = -diff * m_triangleBaryPt[0];
    dcdx[3] = -diff * m_triangleBaryPt[1];
    dcdx[4] = -diff * m_triangleBaryPt[2];

    return true;
}