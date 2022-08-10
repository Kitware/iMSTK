/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdEdgeEdgeConstraint.h"

namespace imstk
{
void
PbdEdgeEdgeConstraint::initConstraint(
    const PbdParticleId& ptA1, const PbdParticleId& ptA2,
    const PbdParticleId& ptB1, const PbdParticleId& ptB2,
    double stiffnessA, double stiffnessB)
{
    m_particles[0] = ptA1;
    m_particles[1] = ptA2;
    m_particles[2] = ptB1;
    m_particles[3] = ptB2;

    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
}

bool
PbdEdgeEdgeConstraint::computeValueAndGradient(PbdState& bodies,
                                               double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& x0 = bodies.getPosition(m_particles[0]);
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& x2 = bodies.getPosition(m_particles[2]);
    const Vec3d& x3 = bodies.getPosition(m_particles[3]);

    const double a  = (x3 - x2).dot(x1 - x0);
    const double b  = (x1 - x0).dot(x1 - x0);
    const double cc = (x0 - x2).dot(x1 - x0);
    const double d  = (x3 - x2).dot(x3 - x2);
    const double e  = a;
    const double f  = (x0 - x2).dot(x3 - x2);

    const double det = a * e - d * b;
    double       s   = 0.5;
    double       t   = 0.5;
    if (fabs(det) > 1e-12)
    {
        s = (cc * e - b * f) / det;
        t = (cc * d - a * f) / det;
        if (s < 0 || s > 1.0 || t < 0 || t > 1.0)
        {
            c = 0.0;
            return false;
        }
    }

    // Two closest points on the line segments
    const Vec3d P = x0 + t * (x1 - x0);
    const Vec3d Q = x2 + s * (x3 - x2);

    // Distance between nearest points on line segments
    Vec3d        n = (Q - P);
    const double l = n.norm();
    n /= l;

    if (l <= 0.0)
    {
        c = 0.0;
        return false;
    }

    // A
    dcdx[0] = (1 - t) * n;
    dcdx[1] = t * n;
    // B
    dcdx[2] = -(1 - s) * n;
    dcdx[3] = -s * n;

    c = l;

    return true;
}
} // namespace imstk