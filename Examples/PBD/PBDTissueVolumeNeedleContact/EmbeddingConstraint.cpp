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
    const PbdParticleId& ptB1, const PbdParticleId& ptB2, const PbdParticleId& ptB3,
    Vec3d* p, Vec3d* q)
{
    m_state = &bodies;

    // Set the triangle
    m_particles[0] = ptB1;
    m_particles[1] = ptB2;
    m_particles[2] = ptB3;
    const Vec3d& x1 = bodies.getPosition(m_particles[0]);
    const Vec3d& x2 = bodies.getPosition(m_particles[1]);
    const Vec3d& x3 = bodies.getPosition(m_particles[2]);

    // Compute intersection point & interpolant on triangle
    CollisionUtils::testSegmentTriangle(*p, *q, x1, x2, x3, m_uvw);
    m_iPt    = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];
    m_iPtVel = Vec3d::Zero();

    // Completely rigid for PBD
    m_stiffness[0] = m_stiffness[1] = 1.0;

    // Compute the interpolant on the line
    {
        m_p = p;
        m_q = q;
        const Vec3d pq = (*p - *q).normalized();
        const Vec3d d  = m_iPt - *q;
        m_t = pq.dot(d);
    }
}

Vec3d
EmbeddingConstraint::computeInterpolantDifference(const PbdState& bodies) const
{
    //const Vec3d& x0 = *m_bodiesFirst[0].vertex;
    const Vec3d& x1 = bodies.getPosition(m_particles[0]);
    const Vec3d& x2 = bodies.getPosition(m_particles[1]);
    const Vec3d& x3 = bodies.getPosition(m_particles[2]);

    Vec3d*      p    = m_p;
    Vec3d*      q    = m_q;
    const Vec3d pq   = (*p - *q);
    const Vec3d pq_n = pq.normalized();

    // Compute the location of the intersection point on both elements
    const Vec3d triPos  = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];
    const Vec3d linePos = (*q) + pq_n * m_t;

    // Compute the transform to align the triangle to the line
    return triPos - linePos;
}

bool
EmbeddingConstraint::computeValueAndGradient(PbdState&           bodies,
                                             double&             c,
                                             std::vector<Vec3d>& dcdx)
{
    // Triangle
    /*const Vec3d& x0 = bodies.getPosition(m_particles[0]);
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& x2 = bodies.getPosition(m_particles[2]);*/

    // Compute the normal/axes of the line
    const Vec3d pq   = *m_p - *m_q;
    const Vec3d pq_n = pq.normalized();

    // Compute the difference between the two interpolated points on the elements
    Vec3d diff = computeInterpolantDifference(bodies);

    // Remove any normal movement (remove only fraction for sort of friction)
    // Frees normal movement
    diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_normalFriction);
    const Vec3d n = diff.normalized();

    dcdx[0] = n;
    dcdx[1] = n;
    dcdx[2] = n;

    c = -diff.norm() * (1.0 - m_compliance0);

    return true;
}

void
EmbeddingConstraint::compute(double dt)
{
    // Jacobian of contact (defines linear and angular constraint axes)
    J = Eigen::Matrix<double, 3, 4>::Zero();
    if (!m_obj1->m_isStatic)
    {
        // Compute the normal/axes of the line
        const Vec3d pq   = *m_p - *m_q;
        const Vec3d pq_n = pq.normalized();

        // Compute the difference between the two interpolated points on the elements
        Vec3d diff = computeInterpolantDifference(*m_state);

        // Remove any normal movement (remove only fraction for sort of friction)
        // Frees normal movement
        diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_normalFriction);
        const Vec3d n = -diff.normalized();

        vu = diff.norm() * m_beta / dt * m_compliance0;

        // Displacement from center of mass
        J(0, 0) = -n[0]; J(0, 1) = 0.0;
        J(1, 0) = -n[1]; J(1, 1) = 0.0;
        J(2, 0) = -n[2]; J(2, 1) = 0.0;
    }
}
} // namespace imstk