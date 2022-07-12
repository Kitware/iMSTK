/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
PbdCollisionConstraint::PbdCollisionConstraint(const int numParticlesA, const int numParticlesB) :
    PbdConstraint(numParticlesA + numParticlesB)
{
    m_bodiesSides.resize(numParticlesA + numParticlesB);
    for (int i = 0; i < m_bodiesSides.size(); i++)
    {
        m_bodiesSides[i] = (i >= numParticlesA); // false/0 for A, true/1 for B
    }
}

void
PbdCollisionConstraint::projectConstraint(PbdState& bodies, const double dt, const SolverType&)
{
    if (dt == 0.0)
    {
        return;
    }

    double c      = 0.0;
    bool   update = this->computeValueAndGradient(bodies, c, m_dcdx);
    if (!update)
    {
        return;
    }

    double lambda = 0.0;

    // Sum the mass (so we can weight displacements)
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        lambda += invMass * m_dcdx[i].squaredNorm();
    }

    if (lambda == 0.0)
    {
        return;
    }

    lambda = c / lambda;

    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        if (invMass > 0.0)
        {
            const Vec3d dx = invMass * lambda *
                             m_dcdx[i] * m_stiffness[m_bodiesSides[i]];
            bodies.getPosition(m_particles[i]) += dx;
        }
    }
}
} // namespace imstk