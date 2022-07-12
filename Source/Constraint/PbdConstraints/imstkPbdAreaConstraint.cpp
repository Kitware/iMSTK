/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdAreaConstraint.h"

namespace imstk
{
void
PbdAreaConstraint::initConstraint(
    const Vec3d& p0, const Vec3d& p1, const Vec3d& p2,
    const PbdParticleId& pIdx0, const PbdParticleId& pIdx1, const PbdParticleId& pIdx2,
    const double k)
{
    m_particles[0] = pIdx0;
    m_particles[1] = pIdx1;
    m_particles[2] = pIdx2;
    setStiffness(k);

    m_restArea = 0.5 * (p1 - p0).cross(p2 - p0).norm();
}

bool
PbdAreaConstraint::computeValueAndGradient(PbdState& bodies,
                                           double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& p0 = bodies.getPosition(m_particles[0]);
    const Vec3d& p1 = bodies.getPosition(m_particles[1]);
    const Vec3d& p2 = bodies.getPosition(m_particles[2]);

    const Vec3d e0 = p0 - p1;
    const Vec3d e1 = p1 - p2;
    const Vec3d e2 = p2 - p0;

    Vec3d n = e0.cross(e1);
    c = 0.5 * n.norm();

    if (c == 0.0)
    {
        return false;
    }

    n /= 2.0 * c;
    c -= m_restArea;

    dcdx[0] = e1.cross(n);
    dcdx[1] = e2.cross(n);
    dcdx[2] = e0.cross(n);

    return true;
}
} // namespace imstk
