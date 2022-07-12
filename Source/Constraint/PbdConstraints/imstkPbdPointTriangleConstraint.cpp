/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdPointTriangleConstraint.h"

namespace imstk
{
void
PbdPointTriangleConstraint::initConstraint(const PbdParticleId& ptA,
                                           const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3,
                                           double stiffnessA, double stiffnessB)
{
    m_particles[0] = ptA;
    m_particles[1] = ptB1;
    m_particles[2] = ptB2;
    m_particles[3] = ptB3;

    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
PbdPointTriangleConstraint::computeValueAndGradient(
    PbdState& bodies,
    double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& x0 = bodies.getPosition(m_particles[0]);
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& x2 = bodies.getPosition(m_particles[2]);
    const Vec3d& x3 = bodies.getPosition(m_particles[3]);

    // Compute barycentric coordinates u,v,w
    const Vec3d  v0    = x2 - x1;
    const Vec3d  v1    = x3 - x1;
    const Vec3d  v2    = x0 - x1;
    const double d00   = v0.dot(v0);
    const double d01   = v0.dot(v1);
    const double d11   = v1.dot(v1);
    const double d20   = v2.dot(v0);
    const double d21   = v2.dot(v1);
    const double denom = d00 * d11 - d01 * d01;
    if (fabs(denom) < 1e-12)
    {
        c = 0.0;
        return false;
    }
    const double v = (d11 * d20 - d01 * d21) / denom;
    const double w = (d00 * d21 - d01 * d20) / denom;
    const double u = 1.0 - v - w;

    // This constraint becomes invalid if moved out of the triangle
    if (u < 0.0 || v < 0.0 || w < 0.0)
    {
        c = 0.0;
        return false;
    }

    // If contacting point near a boundary ignore constraint
    if (!m_enableBoundaryCollisions)
    {
        int maxId = 0;
        Vec3d(u, v, w).maxCoeff(&maxId);
        if (bodies.getInvMass(m_particles[maxId + 1]) == 0.0)
        {
            c = 0.0;
            return false;
        }
    }

    // Triangle normal (pointing up on standard counter clockwise triangle)
    const Vec3d n = v0.cross(v1).normalized();
    // Point could be on either side of triangle, we want to resolve to the triangles plane
    const double l = v2.dot(n);

    // A
    dcdx[0] = -n;
    // B
    dcdx[1] = u * n;
    dcdx[2] = v * n;
    dcdx[3] = w * n;

    c = l;

    return true;
}
} // namespace imstk