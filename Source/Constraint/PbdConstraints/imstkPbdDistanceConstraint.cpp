/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdDistanceConstraint.h"

namespace  imstk
{
void
PbdDistanceConstraint::initConstraint(
    const double restLength,
    const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
    const double k)
{
    m_particles[0] = pIdx0;
    m_particles[1] = pIdx1;
    setStiffness(k);

    m_restLength = restLength;
}

bool
PbdDistanceConstraint::computeValueAndGradient(PbdState& bodies,
                                               double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& p0 = bodies.getPosition(m_particles[0]);
    const Vec3d& p1 = bodies.getPosition(m_particles[1]);

    dcdx[0] = p0 - p1;
    const double len = dcdx[0].norm();
    if (len < 1.0e-16)
    {
        return false;
    }
    dcdx[0] /= len;
    dcdx[1]  = -dcdx[0];
    c        = len - m_restLength;

    return true;
}
} // namespace imstk