/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraint.h"

namespace imstk
{
void
PbdConstraint::projectConstraint(PbdState& bodies,
                                 const double dt, const SolverType& solverType)
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

    // Compute generalized inverse mass sum
    double w = 0.0;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        // Multiplication with dcdx here is important for non normalized
        // constraint gradients
        w += computeGeneralizedInvMass(bodies, i) * m_dcdx[i].squaredNorm();
    }
    if (w == 0.0)
    {
        return;
    }

    double dlambda = 0.0;
    double alpha   = 0.0;
    switch (solverType)
    {
    case (SolverType::PBD):
        dlambda = -c * m_stiffness / w;
        break;
    case (SolverType::xPBD):
    default:
        alpha     = m_compliance / (dt * dt);
        dlambda   = -(c + alpha * m_lambda) / (w + alpha);
        m_lambda += dlambda;
        break;
    }

    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        if (invMass > 0.0)
        {
            bodies.getPosition(m_particles[i]) += invMass * dlambda * m_dcdx[i];
        }
    }
}

void
PbdConstraint::correctVelocity(PbdState& bodies, const double)
{
    const double fricFrac = 1.0 - m_friction;

    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        if (invMass > 0.0)
        {
            const Vec3d n = m_dcdx[i].normalized();
            Vec3d&      v = bodies.getVelocity(m_particles[i]);

            // Separate velocity into normal and tangent components
            const Vec3d vN = n.dot(v) * n;
            const Vec3d vT = v - vN;

            // Put back together fractionally based on defined restitution and frictional coefficients
            v = vN * m_restitution + vT * fricFrac;
        }
    }
}

double
PbdConstraint::computeGeneralizedInvMass(PbdState& bodies,
                                         const size_t particleIndex, const Vec3d& r) const
{
    const PbdParticleId& pid = m_particles[particleIndex];

    // Compute generalized inverse mass sum
    double       w       = 0.0;
    const double invMass = bodies.getInvMass(pid);
    if (bodies.m_bodies[pid.first]->getOriented())
    {
        const Quatd  invOrientation = bodies.getOrientation(pid).inverse();
        const Mat3d& invInteria     = bodies.getInvInertia(pid);
        Vec3d        l = r.cross(m_dcdx[particleIndex]);
        l = invOrientation._transformVector(l);
        // Assumes inertia is diagonal, always in unrotated state
        w += l[0] * l[0] * invInteria(0, 0) +
             l[1] * l[1] * invInteria(1, 1) +
             l[2] * l[2] * invInteria(2, 2);
    }
    w += invMass;
    return w;
}
} // namespace imstk
