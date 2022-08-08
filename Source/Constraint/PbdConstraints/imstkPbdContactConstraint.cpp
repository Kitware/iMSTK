/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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
        //w += computeGeneralizedInvMass(bodies, i, m_r[i]);
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
    // Either both could be rigid, or ones rigid and the others deformable
    Vec3d contactVel1  = Vec3d::Zero();
    bool  leftIsRigid  = bodies.getBodyType(m_particles[0]) == PbdBody::Type::RIGID;
    Vec3d contactVel2  = Vec3d::Zero();
    bool  rightIsRigid = bodies.getBodyType(m_particles.back()) == PbdBody::Type::RIGID;

    if (leftIsRigid)
    {
        const Vec3d& v = bodies.getVelocity(m_particles[0]);
        const Vec3d& w = bodies.getAngularVelocity(m_particles[0]);
        contactVel1 = v + w.cross(m_r[0]);
    }
    if (rightIsRigid)
    {
        const int    lastIndex = m_particles.size() - 1;
        const Vec3d& v = bodies.getVelocity(m_particles[lastIndex]);
        const Vec3d& w = bodies.getAngularVelocity(m_particles[lastIndex]);
        contactVel1 = v + w.cross(m_r[lastIndex]);
    }

    // We can also assume equal and opposite normals/constraint gradients
    const Vec3d  n = m_dcdx[0].normalized();
    const Vec3d  relativeVelocity = contactVel1 - contactVel2;
    const double vNMag = n.dot(relativeVelocity);
    const Vec3d  vN    = vNMag * n;
    const Vec3d  vT    = relativeVelocity - vN;
    const double vTMag = vT.norm();

    if (vTMag == 0.0)
    {
        return;
    }
    // To avoid jitter threshold restitution by normal velocity
    const double restitution = (std::abs(vNMag) < 1.0e-10) ? 0.0 : m_restitution;
    // Correction for friction & restitution
    Vec3d dV = (vT / vTMag) * std::min(m_friction * getForce(dt) * dt, vTMag) +
               n * (-vNMag + std::min(-m_restitution * vNMag, 0.0));

    // Compute generalized inverse mass sum
    double w = 0.0;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        w += computeGeneralizedInvMass(bodies, i, m_r[i]);
    }
    if (w == 0.0)
    {
        return;
    }

    // How to apply dV to deformable particles
    // Do I compute relative velocity per particle?
    // Do I compute relative velocity between contact point? (seems right)
    //  if I do this how do I apply the correction back to the deformable

    const Vec3d p = dV / w;
    for (size_t i = 0; i < m_particles.size(); i++)
    {
        const double invMass = bodies.getInvMass(m_particles[i]);
        if (invMass > 0.0)
        {
            Vec3d p1 = p;
            /*if (bodies.getBodyType(m_particles[i]) != PbdBody::Type::RIGID)
            {
                p1 *= -1.0;
            }*/

            // \todo: Need to apply negative/opposite to other "side"
            bodies.getVelocity(m_particles[i]) += p1 * invMass;

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

bool
PbdTriangleToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                     double&             c,
                                                     std::vector<Vec3d>& n)
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
        c = 0.0;
        return false;
    }
    const double v = (d11 * d20 - d01 * d21) / denom;
    const double w = (d00 * d21 - d01 * d20) / denom;
    const double u = 1.0 - v - w;

    // This constraint becomes invalid if moved out of the triangle
    if (u < 0.0 || v < 0.0 || w < 0.0)
    {
        c = 0.0;
        return false;
    }

    // Triangle normal (pointing up on standard counter clockwise triangle)
    const Vec3d normal = v0.cross(v1).normalized();
    // Point could be on either side of triangle, we want to resolve to the triangles plane
    const double l = v2.dot(normal);

    // A
    n[0] = normal;
    // B
    n[1] = -u * normal;
    n[2] = -v * normal;
    n[3] = -w * normal;

    c = l;

    return true;
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

bool
PbdEdgeToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                 double&             c,
                                                 std::vector<Vec3d>& n)
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
        // There is no distance between the edge, can't do anything
        c = 0.0;
        return false;
    }
    const Vec3d dir1 = ab / length;

    // Project onto the line
    const Vec3d  diff = p - x1;
    const double p1   = dir1.dot(diff);
    if (p1 < 0.0 || p1 > length)
    {
        c = 0.0;
        return false;
    }
    // Remove tangent component to get normal
    const Vec3d  diff1 = diff - p1 * dir1;
    const double l     = diff1.norm();
    if (l == 0.0)
    {
        // The point is on the line
        c = 0.0;
        return false;
    }
    const Vec3d  normal = diff1 / l;
    const double u      = p1 / length;

    // A
    n[0] = normal;
    // B
    n[1] = -(1.0 - u) * normal;
    n[2] = -u * normal;

    c = l;

    return true;
}

bool
PbdBodyToBodyConstraint::computeValueAndGradient(PbdState&           bodies,
                                                 double&             c,
                                                 std::vector<Vec3d>& n)
{
    const Vec3d& bodyPos0 = bodies.getPosition(m_particles[0]);
    const Vec3d  p0       = bodyPos0 + m_r[0];

    const Vec3d& bodyPos1 = bodies.getPosition(m_particles[1]);
    const Vec3d  p1       = bodyPos1 + m_r[1];

    const Vec3d diff = p1 - p0;

    c = diff.dot(m_contactNormal);

    // A
    n[0] = -m_contactNormal;
    // B
    n[1] = m_contactNormal;

    return true;
}

bool
PbdBodyToBodyDistanceConstraint::computeValueAndGradient(PbdState&           bodies,
                                                         double&             c,
                                                         std::vector<Vec3d>& n)
{
    // Transform local position to acquire transformed global (for constraint reprojection)
    const Vec3d& bodyPos0 = bodies.getPosition(m_particles[0]);
    const Vec3d  p0       = bodyPos0 + bodies.getOrientation(m_particles[0])._transformVector(m_rest_r[0]);

    const Vec3d& bodyPos1 = bodies.getPosition(m_particles[1]);
    const Vec3d  p1       = bodyPos1 + bodies.getOrientation(m_particles[1])._transformVector(m_rest_r[1]);

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
} // namespace imstk