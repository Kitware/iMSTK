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

void
PbdAngularHingeConstraint::initConstraint(
    const PbdParticleId& pIdx0,
    const Vec3d&         hingeAxes,
    const double         compliance)
{
    m_particles[0] = pIdx0;
    m_hingeAxes    = hingeAxes;
    setCompliance(compliance);
}

bool
PbdAngularHingeConstraint::computeValueAndGradient(PbdState& bodies,
                                                   double& c, std::vector<Vec3d>& dcdx)
{
    // Is this the fastest way to get basis vectors?
    const Vec3d localY = bodies.getOrientation(m_particles[0]).toRotationMatrix().col(1);

    // Gives rotation
    Vec3d dir = m_hingeAxes.cross(localY);
    dcdx[0] = dir.normalized();
    c       = dir.norm();

    return true;
}

void
PbdAngularDistanceConstraint::initConstraint(
    const PbdParticleId& p0,
    const PbdParticleId& p1,
    const double         compliance)
{
    m_particles[0] = p0;
    m_particles[1] = p1;
    setCompliance(compliance);
}

void
PbdAngularDistanceConstraint::initConstraintOffset(
    const PbdState&      bodies,
    const PbdParticleId& p0,
    const PbdParticleId& p1,
    const double         compliance)
{
    m_particles[0] = p0;
    m_particles[1] = p1;

    const Quatd& q0 = bodies.getOrientation(m_particles[0]);
    const Quatd& q1 = bodies.getOrientation(m_particles[1]);

    m_offset = q0.inverse() * q1;

    setCompliance(compliance);
}

void
PbdAngularDistanceConstraint::initConstraintOffset(
    const PbdParticleId& p0,
    const PbdParticleId& p1,
    const Quatd          rotationalOffset,
    const double         compliance)
{
    initConstraint(p0, p1, compliance);
    m_offset = rotationalOffset;
}

bool
PbdAngularDistanceConstraint::computeValueAndGradient(PbdState& bodies,
                                                      double& c, std::vector<Vec3d>& dcdx)
{
    const Quatd& q0 = bodies.getOrientation(m_particles[0]);
    const Quatd& q1 = bodies.getOrientation(m_particles[1]);

    // Gives rotation from q0->q1 (ie: q1 = dq * q0)
    const Quatd       dq = q1 * (q0 * m_offset).inverse();
    Eigen::AngleAxisd angleAxes(dq);
    dcdx[0] = angleAxes.axis();
    dcdx[1] = -dcdx[0];
    c       = -angleAxes.angle();

    return true;
}
} // namespace imstk