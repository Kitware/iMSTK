/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdContactConstraint.h"

namespace imstk
{
void
PbdContactConstraint::projectConstraint(PbdState& bodies,
                                        const double dt, const SolverType& solverType)
{
    if (dt == 0.0)
    {
        return;
    }

    // dcdx is normalized
    double c      = 0.0;
    bool   update = this->computeValueAndGradient(bodies, c,
        m_dcdx);
    if (!update)
    {
        return;
    }

    // Compute generalized inverse mass sum
    double w = 0.0;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        w += computeGeneralizedInvMass(bodies, i, m_r[i]) * m_dcdx[i].squaredNorm();
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
            // Positional impulse
            const Vec3d p  = dlambda * m_dcdx[i];
            const Vec3d dx = p * invMass;
            bodies.getPosition(m_particles[i]) += dx;

            if (bodies.getBodyType(m_particles[i]) == PbdBody::Type::RIGID)
            {
                Quatd&       q = bodies.getOrientation(m_particles[i]);
                const Mat3d& invInteria = bodies.getInvInertia(m_particles[i]);

                Vec3d rot = m_r[i].cross(p);
                // Might be able to avoid a division in the inverse under assuming its unit
                rot = q.inverse()._transformVector(rot);
                rot = invInteria * rot;
                rot = q._transformVector(rot);

                // Apply a max rotation limit, quaternions can only represent
                // rotations up to 180deg
                double       scale = 1.0;
                const double phi   = rot.norm();
                if (phi > 0.5) // Max rot
                {
                    scale = 0.5 / phi;
                }

                const Quatd dq = Quatd(0.0,
                    rot[0] * scale,
                    rot[1] * scale,
                    rot[2] * scale) * q;
                q.coeffs() += dq.coeffs() * 0.5;
                q.normalize();
            }
        }
    }
}

void
PbdContactConstraint::correctVelocity(PbdState& bodies, const double dt)
{
    if (!m_correctVelocity)
    {
        return;
    }

    // Assumed equal and opposite normals/constraint gradients
    const Vec3d contactNormal = m_dcdx[0].normalized();

    // We can also assume equal and opposite normals/constraint gradients
    const Vec3d  relativeVelocity = computeRelativeVelocity(bodies);
    const double vNMag = contactNormal.dot(relativeVelocity);
    const Vec3d  vN    = vNMag * contactNormal;
    const Vec3d  vT    = relativeVelocity - vN;
    const double vTMag = vT.norm();

    if (vTMag == 0.0)
    {
        return;
    }
    // Avoid jitter with threshold here
    //const double restitution = (std::abs(vNMag) < 1.0e-10) ? 0.0 : m_restitution;

    // Velocity correction for friction & restitution
    const Vec3d dV = (vT / vTMag) * std::min(m_friction * getForce(dt) * dt, vTMag) +
                     contactNormal * (-vNMag + std::min(-m_restitution * vNMag, 0.0));

    // Compute generalized inverse mass sum
    double w = 0.0;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        w += computeGeneralizedInvMass(bodies, i, m_r[i]) /** m_dcdx[i].squaredNorm()*/;
    }
    if (w == 0.0)
    {
        return;
    }

    const Vec3d p = dV / w;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        if (invMass > 0.0)
        {
            bodies.getVelocity(m_particles[i]) += p * invMass * m_weights[i];

            if (bodies.getBodyType(m_particles[i]) == PbdBody::Type::RIGID)
            {
                const Mat3d& invInteria  = bodies.getInvInertia(m_particles[i]);
                const Quatd& orientation = bodies.getOrientation(m_particles[i]);

                // Apply inertia from rest pose
                Vec3d rot = m_r[i].cross(p);
                rot = orientation.inverse()._transformVector(rot);
                rot = invInteria * rot;
                rot = orientation._transformVector(rot);
                bodies.getAngularVelocity(m_particles[i]) += rot;
            }
        }
    }
}

void
PbdTriangleToBodyConstraint::initConstraint(
    const PbdState& state,
    const PbdParticleId& bodyId,
    const Vec3d contactPtOnBody,
    const PbdParticleId& x0, const PbdParticleId& x1, const PbdParticleId& x2,
    const double compliance)
{
    m_particles[0] = bodyId;
    // Compute local position on body
    m_r[0] = contactPtOnBody - state.getPosition(bodyId);
    m_particles[1] = x0;
    m_particles[2] = x1;
    m_particles[3] = x2;

    setCompliance(compliance);
}

bool
PbdTriangleToBodyConstraint::computeInterpolantsAndContact(const PbdState& bodies,
                                                           std::vector<double>& weights, Vec3d& contactNormal, double& depth) const
{
    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);
    const Vec3d& x1      = bodies.getPosition(m_particles[1]);
    const Vec3d& x2      = bodies.getPosition(m_particles[2]);
    const Vec3d& x3      = bodies.getPosition(m_particles[3]);

    // Global position
    const Vec3d p = bodyPos + m_r[0];

    // Compute barycentric coordinates u,v,w
    const Vec3d  v0    = x2 - x1;
    const Vec3d  v1    = x3 - x1;
    const Vec3d  v2    = p - x1;
    const double d00   = v0.dot(v0);
    const double d01   = v0.dot(v1);
    const double d11   = v1.dot(v1);
    const double d20   = v2.dot(v0);
    const double d21   = v2.dot(v1);
    const double denom = d00 * d11 - d01 * d01;
    if (fabs(denom) < 1e-12)
    {
        return false;
    }
    // Point
    weights[0] = 1.0;
    // Triangle
    weights[3] = (d11 * d20 - d01 * d21) / denom;
    weights[2] = (d00 * d21 - d01 * d20) / denom;
    weights[1] = 1.0 - weights[2] - weights[3];

    // This constraint becomes invalid if moved out of the triangle
    if (weights[1] < 0.0 || weights[2] < 0.0 || weights[3] < 0.0)
    {
        return false;
    }

    // Triangle normal (pointing up on standard counter clockwise triangle)
    contactNormal = v0.cross(v1).normalized();
    // Point could be on either side of triangle, we want to resolve to the triangles plane
    depth = v2.dot(contactNormal);

    return true;
}

bool
PbdTriangleToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                     double&             c,
                                                     std::vector<Vec3d>& n)
{
    Vec3d  normal = Vec3d::Zero();
    double depth  = 0.0;
    if (!computeInterpolantsAndContact(bodies, m_weights, normal, depth))
    {
        c = 0.0;
        return false;
    }

    // A
    n[0] = normal;
    // B
    n[1] = -m_weights[1] * normal;
    n[2] = -m_weights[2] * normal;
    n[3] = -m_weights[3] * normal;

    c = depth;

    return true;
}

Vec3d
PbdTriangleToBodyConstraint::computeRelativeVelocity(PbdState& bodies)
{
    Vec3d  normal = Vec3d::Zero();
    double depth  = 0.0;
    if (!computeInterpolantsAndContact(bodies, m_weights, normal, depth))
    {
        return Vec3d::Zero();
    }
    m_weights[0] = -m_weights[0];

    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);
    // Global position
    const Vec3d contactPt = bodyPos + m_r[0];
    const Vec3d v0 = getVelocityOnRigidBody(bodies, m_particles[0].first, contactPt);

    //const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& v1 = bodies.getPosition(m_particles[1]);
    //const Vec3d& x2 = bodies.getPosition(m_particles[2]);
    const Vec3d& v2 = bodies.getPosition(m_particles[2]);
    //const Vec3d& x3 = bodies.getPosition(m_particles[3]);
    const Vec3d& v3   = bodies.getPosition(m_particles[3]);
    const Vec3d  v123 = v1 * m_weights[1] + v2 * m_weights[2] + v3 * m_weights[3];

    // We can also assume equal and opposite normals/constraint gradients
    return v0 - v123;
}

void
PbdVertexToBodyConstraint::initConstraint(
    const PbdState&      state,
    const PbdParticleId& bodyId,
    const Vec3d          contactPtOnBody,
    const PbdParticleId& x0,
    const double         compliance)
{
    m_particles[0] = bodyId;
    // Compute local position on body
    m_r[0] = contactPtOnBody - state.getPosition(bodyId);
    m_particles[1] = x0;

    // Infinite stiffness/completely rigid
    setCompliance(compliance);
}

bool
PbdVertexToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                   double&             c,
                                                   std::vector<Vec3d>& n)
{
    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);

    // Global position
    const Vec3d p = bodyPos + m_r[0];

    // Current position during solve
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);

    const Vec3d diff = x1 - p;
    c = diff.norm();

    if (c == 0.0)
    {
        return false;
    }

    const Vec3d normal = diff / c;

    // A (direction to move body)
    n[0] = -normal;
    // B (direction to move vertex)
    n[1] = normal;

    return true;
}

Vec3d
PbdVertexToBodyConstraint::computeRelativeVelocity(PbdState& bodies)
{
    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);
    // Global position
    const Vec3d contactPt = bodyPos + m_r[0];
    const Vec3d v0 = getVelocityOnRigidBody(bodies, m_particles[0].first, contactPt);

    m_weights[0] = 1.0;
    m_weights[1] = -1.0;

    //const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& v1 = bodies.getVelocity(m_particles[1]);
    return v0 - v1;
}

void
PbdEdgeToBodyConstraint::initConstraint(
    const PbdState& state,
    const PbdParticleId& bodyId,
    const Vec3d contactPtOnBody,
    const PbdParticleId& x0, const PbdParticleId& x1,
    const double compliance)
{
    m_particles[0] = bodyId;
    // Compute local position on body
    m_r[0] = contactPtOnBody - state.getPosition(bodyId);
    m_particles[1] = x0;
    m_particles[2] = x1;

    setCompliance(compliance);
}

bool
PbdEdgeToBodyConstraint::computeInterpolantsAndContact(const PbdState& bodies,
                                                       std::vector<double>& weights, Vec3d& normal, double& depth) const
{
    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);

    // Global position
    const Vec3d p = bodyPos + m_r[0];

    // Just project p onto x3-x2. Get the normal component for distance to line
    const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& x2 = bodies.getPosition(m_particles[2]);

    const Vec3d  ab     = x2 - x1;
    const double length = ab.norm();
    if (length == 0.0)
    {
        return false;
    }
    const Vec3d dir1 = ab / length;

    // Project onto the line
    const Vec3d  diff = p - x1;
    const double p1   = dir1.dot(diff);
    if (p1 < 0.0 || p1 > length)
    {
        return false;
    }
    // Remove tangent component to get normal
    const Vec3d  diff1 = diff - p1 * dir1;
    const double l     = diff1.norm();
    if (l == 0.0)
    {
        return false;
    }
    normal = diff1 / l;
    const double u = p1 / length;

    // Point
    weights[0] = 1.0;
    // Edge
    weights[1] = (1.0 - u);
    weights[2] = u;

    depth = l;
    return true;
}

bool
PbdEdgeToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                 double&             c,
                                                 std::vector<Vec3d>& n)
{
    Vec3d  normal = Vec3d::Zero();
    double depth  = 0.0;
    if (!computeInterpolantsAndContact(bodies, m_weights, normal, depth))
    {
        c = 0.0;
        return false;
    }

    // A
    n[0] = normal;
    // B
    n[1] = -m_weights[1] * normal;
    n[2] = -m_weights[2] * normal;

    c = depth;

    return true;
}

Vec3d
PbdEdgeToBodyConstraint::computeRelativeVelocity(PbdState& bodies)
{
    Vec3d  normal = Vec3d::Zero();
    double depth  = 0.0;
    if (!computeInterpolantsAndContact(bodies, m_weights, normal, depth))
    {
        return Vec3d::Zero();
    }
    m_weights[0] = -m_weights[0];

    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);
    // Global position
    const Vec3d contactPt = bodyPos + m_r[0];
    const Vec3d v0 = getVelocityOnRigidBody(bodies, m_particles[0].first, contactPt);

    //const Vec3d& x1 = bodies.getPosition(m_particles[1]);
    const Vec3d& v1 = bodies.getPosition(m_particles[1]);
    //const Vec3d& x2 = bodies.getPosition(m_particles[2]);
    const Vec3d& v2  = bodies.getPosition(m_particles[2]);
    const Vec3d  v12 = v1 * m_weights[1] + v2 * m_weights[2];
    return v0 - v12;
}

bool
PbdBodyToBodyDistanceConstraint::computeValueAndGradient(PbdState&           bodies,
                                                         double&             c,
                                                         std::vector<Vec3d>& n)
{
    // Transform local position to acquire transformed global (for constraint reprojection)
    const Vec3d& bodyPos0 = bodies.getPosition(m_particles[0]);
    Vec3d        p0       = bodyPos0;
    if (bodies.getBodyType(m_particles[0]) != PbdBody::Type::DEFORMABLE)
    {
        m_r[0] = bodies.getOrientation(m_particles[0])._transformVector(m_rest_r[0]);
        p0    += m_r[0];
    }

    const Vec3d& bodyPos1 = bodies.getPosition(m_particles[1]);
    Vec3d        p1       = bodyPos1;
    if (bodies.getBodyType(m_particles[1]) != PbdBody::Type::DEFORMABLE)
    {
        m_r[1] = bodies.getOrientation(m_particles[1])._transformVector(m_rest_r[1]);
        p1    += m_r[1];
    }

    // Move according to the difference
    Vec3d        diff   = p1 - p0;
    const double length = diff.norm();
    if (length == 0.0)
    {
        return false;
    }
    diff /= length;

    // A
    n[0] = diff.normalized();
    // B
    n[1] = -n[0];

    c = -length;

    return true;
}

bool
PbdBodyToBodyNormalConstraint::computeValueAndGradient(PbdState&           bodies,
                                                       double&             c,
                                                       std::vector<Vec3d>& n)
{
    const Vec3d& bodyPos0 = bodies.getPosition(m_particles[0]);
    m_r[0] = bodies.getOrientation(m_particles[0])._transformVector(m_rest_r[0]);
    const Vec3d p0 = bodyPos0 + m_r[0];

    const Vec3d& bodyPos1 = bodies.getPosition(m_particles[1]);
    m_r[1] = bodies.getOrientation(m_particles[1])._transformVector(m_rest_r[1]);
    const Vec3d p1 = bodyPos1 + m_r[1];

    const Vec3d diff = p1 - p0;

    c = diff.dot(m_contactNormal);

    // A
    n[0] = -m_contactNormal;
    // B
    n[1] = m_contactNormal;

    return true;
}

bool
PbdRigidLineToPointConstraint::computeValueAndGradient(PbdState& bodies,
                                                       double& c, std::vector<Vec3d>& n)
{
    const Vec3d& bodyPos = bodies.getPosition(m_particles[0]);
    const Quatd& bodyOrientation = bodies.getOrientation(m_particles[0]);
    const Vec3d  p = bodyPos + bodyOrientation._transformVector(m_p_rest);
    const Vec3d  q = bodyPos + bodyOrientation._transformVector(m_q_rest);

    // Compute distance to pq
    const Vec3d dir = (q - p).normalized();

    const Vec3d& pt = bodies.getPosition(m_particles[1]);

    const Vec3d  diff = pt - q;
    const double l    = diff.dot(dir);
    const Vec3d  dist = diff - l * dir;

    n[1] = dist.normalized();
    n[0] = -n[1];
    // Pt on line - line body center
    m_r[0] = (pt - dist) - bodyPos;

    c = dist.norm();

    return true;
}
} // namespace imstk