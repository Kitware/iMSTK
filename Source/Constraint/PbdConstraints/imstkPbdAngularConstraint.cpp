/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdAngularConstraint.h"

namespace imstk
{
void
PbdAngularConstraint::projectConstraint(PbdState& bodies,
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

    // Compute the generalized inverse mass w
    double w = 0.0;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const Quatd& q = bodies.getOrientation(m_particles[i]);
        const Mat3d& invInteria = bodies.getInvInertia(m_particles[i]);
        const Vec3d  l = q.inverse()._transformVector(m_dcdx[i]);
        w += l[0] * l[0] * invInteria(0, 0) +
             l[1] * l[1] * invInteria(1, 1) +
             l[2] * l[2] * invInteria(2, 2);
    }

    if (w < IMSTK_DOUBLE_EPS)
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
        Quatd&       q = bodies.getOrientation(m_particles[i]);
        const Mat3d& invInteria = bodies.getInvInertia(m_particles[i]);

        // Transform to rest pose and apply inertia then back
        const Vec3d p   = dlambda * m_dcdx[i];
        Vec3d       rot = p;
        rot = q.inverse()._transformVector(rot);
        rot = invInteria * rot;
        rot = q._transformVector(rot);

        double       scale = 1.0;
        const double phi   = rot.norm();
        if (phi > 0.5) // Max rot
        {
            scale = 0.5 / phi;
        }

        const Quatd dq_quat = Quatd(0.0,
            rot[0] * scale,
            rot[1] * scale,
            rot[2] * scale) * q;
        q.coeffs() += dq_quat.coeffs() * 0.5;
        q.normalize();
    }
}
} // namespace imstk
