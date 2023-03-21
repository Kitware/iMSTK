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
    const Vec3d bary = baryCentric(x0, x1, x2, x3);

    // This constraint becomes invalid if moved out of the triangle
    if (bary[0] < 0.0 || bary[1] < 0.0 || bary[2] < 0.0)
    {
        c = 0.0;
        return false;
    }

    // If contacting point near a boundary ignore constraint
    if (!m_enableBoundaryCollisions)
    {
        int maxId = 0;
        bary.maxCoeff(&maxId);
        // +1 as first particle is from other body (vertex)
        if (bodies.getInvMass(m_particles[maxId + 1]) == 0.0)
        {
            c = 0.0;
            return false;
        }
    }

    // Triangle normal (pointing up on counter clockwise triangle)
    const Vec3d n = (x2 - x1).cross(x3 - x1).normalized();
    // Point could be on either side of triangle, we want to resolve to the triangles plane
    const double l = (x0 - x1).dot(n);

    // A
    dcdx[0] = -n;
    // B
    dcdx[1] = bary[0] * n;
    dcdx[2] = bary[1] * n;
    dcdx[3] = bary[2] * n;

    c = l;

    return true;
}
} // namespace imstk