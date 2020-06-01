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

#include "imstkPbdBendConstraint.h"

namespace imstk
{
void
PbdBendConstraint::initConstraint(
    const StdVectorOfVec3d& initVertexPositions,
    const size_t& pIdx1, const size_t& pIdx2,
    const size_t& pIdx3, const double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_vertexIds[2] = pIdx3;

    m_stiffness = k;

    const Vec3d& p0 = initVertexPositions[pIdx1];
    const Vec3d& p1 = initVertexPositions[pIdx2];
    const Vec3d& p2 = initVertexPositions[pIdx3];

    // Instead of using the angle between the segments we can use the distance
    // from the center of the triangle
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    m_restLength = (p1 - center).norm();
}

bool
PbdBendConstraint::solvePositionConstraint(
    StdVectorOfVec3d&      currVertexPositions,
    const StdVectorOfReal& currInvMasses)
{
    const size_t i1 = m_vertexIds[0];
    const size_t i2 = m_vertexIds[1];
    const size_t i3 = m_vertexIds[2];

    Vec3d& p0 = currVertexPositions[i1];
    Vec3d& p1 = currVertexPositions[i2];
    Vec3d& p2 = currVertexPositions[i3];

    const Real im0 = currInvMasses[i1];
    const Real im1 = currInvMasses[i2];
    const Real im2 = currInvMasses[i3];

    const Real m0 = (im0 > 0.0) ? 1.0 / im0 : 0.0;
    const Real m1 = (im1 > 0.0) ? 1.0 / im1 : 0.0;
    const Real m2 = (im2 > 0.0) ? 1.0 / im2 : 0.0;

    // Move towards triangle center
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    const Vec3d& diff   = p1 - center;
    const double dist   = diff.norm();
    if (dist < m_epsilon)
    {
        return false;
    }
    const Vec3d& dir = diff / dist;
    const double c   = (dist - m_restLength) * m_stiffness;

    // Now Apply movement weighted by masses
    if (im0 > 0.0)
    {
        p0 += c * dir * 2.0 * m0 / (m0 + m1 + m2);
    }

    if (im1 > 0.0)
    {
        p1 -= c * dir * 4.0 * m1 / (m0 + m1 + m2);
    }

    if (im2 > 0.0)
    {
        p2 += c * dir * 2.0 * m2 / (m0 + m1 + m2);
    }

    return true;
}
} // imstk