/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdPointPointConstraint.h"

namespace imstk
{
void
PbdPointPointConstraint::initConstraint(
    const PbdParticleId& ptA, const PbdParticleId& ptB,
    double stiffnessA, double stiffnessB)
{
    m_particles[0] = ptA;
    m_particles[1] = ptB;

    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
PbdPointPointConstraint::computeValueAndGradient(PbdState& bodies,
                                                 double& c, std::vector<Vec3d>& dcdx)
{
    // Current position during solve
    const Vec3d& x0 = bodies.getPosition(m_particles[0]);
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);

    const Vec3d diff = x1 - x0;
    c = diff.norm();

    if (c == 0.0)
    {
        return false;
    }

    const Vec3d n = diff / c;

    // A
    dcdx[0] = n;
    // B
    dcdx[1] = -n;

    return true;
}
} // namespace imstk