/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "EmbeddingConstraint.h"
#include "imstkCollisionUtils.h"

namespace imstk
{
void
EmbeddingConstraint::initConstraint(
    PbdState& bodies,
    const PbdParticleId& ptA1,
    const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3,
    Vec3d* p, Vec3d* q,
    const double compliance)
{
    // Set the triangle
    m_particles[0] = ptB1;
    m_particles[1] = ptB2;
    m_particles[2] = ptB3;
    const Vec3d& x1 = bodies.getPosition(m_particles[0]);
    const Vec3d& x2 = bodies.getPosition(m_particles[1]);
    const Vec3d& x3 = bodies.getPosition(m_particles[2]);

    // Compute intersection point & interpolant on triangle
    CollisionUtils::testSegmentTriangle(*p, *q, x1, x2, x3, m_uvw);
    m_iPt = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];

    m_particles[3] = ptA1;
    // Compute local untransformed position on needle (the iPt will move with the needle)
    m_r[3] = bodies.getOrientation(ptA1).inverse()._transformVector(m_iPt - bodies.getPosition(ptA1));

    // Compute the interpolant on the line
    {
        m_p = p;
        m_q = q;
        const Vec3d pq = (*p - *q).normalized();
        const Vec3d d  = m_iPt - *q;
        t       = pq.dot(d);
        m_uv[0] = t = pq.dot(d) / pq.norm();
        m_uv[1] = 1.0 - m_uv[0];
    }

    setCompliance(compliance);
}

Vec3d
EmbeddingConstraint::computeInterpolantDifference(const PbdState& bodies) const
{
    const Vec3d& x1 = bodies.getPosition(m_particles[0]);
    const Vec3d& x2 = bodies.getPosition(m_particles[1]);
    const Vec3d& x3 = bodies.getPosition(m_particles[2]);

    Vec3d*      p    = m_p;
    Vec3d*      q    = m_q;
    const Vec3d pq   = (*p - *q);
    const Vec3d pq_n = pq.normalized();

    // Compute the location of the intersection point on both elements
    const Vec3d triPos  = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];
    const Vec3d linePos = (*q) * m_uv[0] + (*p) * m_uv[1];
    //const Vec3d linePos = (*q) + pq_n * t;

    // Compute the transform to align the triangle to the line
    return triPos - linePos;
}

bool
EmbeddingConstraint::computeValueAndGradient(PbdState& bodies, double& c,
                                             std::vector<Vec3d>& dcdx)
{
    // Triangle
    const Vec3d& x0 = bodies.getPosition(m_particles[0]);
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& x2 = bodies.getPosition(m_particles[2]);
    // Body center of mass
    const Vec3d& x3 = bodies.getPosition(m_particles[3]);
    //const Quatd& q3 = bodies.getOrientation(m_particles[3]);

    // Compute the normal/axes of the line
    const Vec3d pq   = *m_p - *m_q;
    const Vec3d pq_n = pq.normalized();

    // Compute the difference between the two interpolated points on the elements
    Vec3d diff = computeInterpolantDifference(bodies);

    // Remove any normal movement (remove only fraction for sort of friction)
    // Frees normal movement
    diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_normalFriction);
    const Vec3d ortho = diff.normalized(); // Constrain only orthogonal movement

    // Puncture point
    const Vec3d triPos = m_iPt = x0 * m_uvw[0] + x1 * m_uvw[1] + x2 * m_uvw[2];

    dcdx[0] = -ortho;
    dcdx[1] = -ortho;
    dcdx[2] = -ortho;
    dcdx[3] = ortho;
    //m_r[3] = triPos - x3;

    c = -diff.norm();

    return true;
}
} // namespace imstk