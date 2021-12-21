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

#include "EmbeddingConstraint.h"
#include "imstkCollisionUtils.h"

namespace imstk
{
// \todo: Try adding torque/rotation to the tool, two interpolant points a and b with line center of mass c
// then rotation a to b around c is given with torques (a-c)x(b-c)

void
EmbeddingConstraint::initConstraint(
    VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
    Vec3d* p, Vec3d* q)
{
    // Set the triangle
    m_bodiesSecond[0] = ptB1;
    m_bodiesSecond[1] = ptB2;
    m_bodiesSecond[2] = ptB3;
    const Vec3d& x1 = *m_bodiesSecond[0].vertex;
    const Vec3d& x2 = *m_bodiesSecond[1].vertex;
    const Vec3d& x3 = *m_bodiesSecond[2].vertex;

    // Compute intersection point & interpolant on triangle
    CollisionUtils::testSegmentTriangle(*p, *q, x1, x2, x3, m_uvw);
    m_iPt    = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];
    m_iPtVel = Vec3d::Zero();

    // Set the point on the line for PBD
    m_bodiesFirst[0] = { &m_iPt, 0.0, &m_iPtVel };
    const Vec3d& x0 = *m_bodiesFirst[0].vertex;

    // Completely rigid for PBD
    m_stiffnessA = m_stiffnessB = 1.0;

    // Compute the interpolant on the line
    {
        m_p = p;
        m_q = q;
        const Vec3d pq = (*p - *q).normalized();
        const Vec3d d  = x0 - *q;
        m_t = pq.dot(d);
    }
}

Vec3d
EmbeddingConstraint::computeInterpolantDifference() const
{
    //const Vec3d& x0 = *m_bodiesFirst[0].vertex;
    const Vec3d& x1 = *m_bodiesSecond[0].vertex;
    const Vec3d& x2 = *m_bodiesSecond[1].vertex;
    const Vec3d& x3 = *m_bodiesSecond[2].vertex;

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
EmbeddingConstraint::computeValueAndGradient(double&             c,
                                             std::vector<Vec3d>& dcdxA,
                                             std::vector<Vec3d>& dcdxB) const
{
    //const Vec3d& x0 = *m_bodiesFirst[0].vertex;
    //const Vec3d& x1 = *m_bodiesSecond[0].vertex;
    //const Vec3d& x2 = *m_bodiesSecond[1].vertex;
    //const Vec3d& x3 = *m_bodiesSecond[2].vertex;

    // Compute the normal/axes of the line
    const Vec3d pq   = *m_p - *m_q;
    const Vec3d pq_n = pq.normalized();

    // Compute the difference between the two interpolated points on the elements
    Vec3d diff = computeInterpolantDifference();

    // Remove any normal movement (remove only fraction for sort of friction)
    // Frees normal movement
    diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_normalFriction);
    const Vec3d n = diff.normalized();

    dcdxA[0] = Vec3d::Zero();
    dcdxB[0] = n;
    dcdxB[1] = n;
    dcdxB[2] = n;

    c = -diff.norm() * (1.0 - m_compliance);

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
        Vec3d diff = computeInterpolantDifference();

        // Remove any normal movement (remove only fraction for sort of friction)
        // Frees normal movement
        diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_normalFriction);
        const Vec3d n = -diff.normalized();

        vu = diff.norm() * m_beta / dt * m_compliance;

        // Displacement from center of mass
        J(0, 0) = -n[0]; J(0, 1) = 0.0;
        J(1, 0) = -n[1]; J(1, 1) = 0.0;
        J(2, 0) = -n[2]; J(2, 1) = 0.0;
    }
}
}