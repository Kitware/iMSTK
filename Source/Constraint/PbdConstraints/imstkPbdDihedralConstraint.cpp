/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdDihedralConstraint.h"

namespace  imstk
{
void
PbdDihedralConstraint::initConstraint(
    const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
    const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
    const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
    const double k)
{
    m_particles[0] = pIdx0;
    m_particles[1] = pIdx1;
    m_particles[2] = pIdx2;
    m_particles[3] = pIdx3;

    setStiffness(k);

    const Vec3d n1 = (p2 - p0).cross(p3 - p0).normalized();
    const Vec3d n2 = (p3 - p1).cross(p2 - p1).normalized();

    m_restAngle = atan2(n1.cross(n2).dot(p3 - p2), (p3 - p2).norm() * n1.dot(n2));
}

bool
PbdDihedralConstraint::computeValueAndGradient(PbdState& bodies,
                                               double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& p0 = bodies.getPosition(m_particles[0]);
    const Vec3d& p1 = bodies.getPosition(m_particles[1]);
    const Vec3d& p2 = bodies.getPosition(m_particles[2]);
    const Vec3d& p3 = bodies.getPosition(m_particles[3]);

    const Vec3d e  = p3 - p2;
    const Vec3d e1 = p3 - p0;
    const Vec3d e2 = p0 - p2;
    const Vec3d e3 = p3 - p1;
    const Vec3d e4 = p1 - p2;
    // To accelerate, all normal (area) vectors and edge length should be precomputed in parallel
    Vec3d        n1 = e1.cross(e);
    Vec3d        n2 = e.cross(e3);
    const double A1 = n1.norm();
    const double A2 = n2.norm();
    n1 /= A1;
    n2 /= A2;

    const double l = e.norm();
    if (l == 0.0)
    {
        return false;
    }

    dcdx[0] = -(l / A1) * n1;
    dcdx[1] = -(l / A2) * n2;
    dcdx[2] = (e.dot(e1) / (A1 * l)) * n1 + (e.dot(e3) / (A2 * l)) * n2;
    dcdx[3] = (e.dot(e2) / (A1 * l)) * n1 + (e.dot(e4) / (A2 * l)) * n2;

    c = atan2(n1.cross(n2).dot(e), l * n1.dot(n2)) - m_restAngle;

    return true;
}
} // namespace imstk