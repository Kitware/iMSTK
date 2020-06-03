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

#include "imstkPbdDistanceConstraint.h"

namespace  imstk
{
void
PbdDistanceConstraint::initConstraint(
    const StdVectorOfVec3d& initVertexPositions,
    const size_t& pIdx1, const size_t& pIdx2, const double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_stiffness    = k;

    const Vec3d& p1 = initVertexPositions[pIdx1];
    const Vec3d& p2 = initVertexPositions[pIdx2];

    m_restLength = (p1 - p2).norm();
}

bool
PbdDistanceConstraint::solvePositionConstraint(
    StdVectorOfVec3d&      currVertexPositions,
    const StdVectorOfReal& currInvMasses)
{
    const size_t i1 = m_vertexIds[0];
    const size_t i2 = m_vertexIds[1];

    Vec3d& p0 = currVertexPositions[i1];
    Vec3d& p1 = currVertexPositions[i2];

    const Real im1 = currInvMasses[i1];
    const Real im2 = currInvMasses[i2];

    const auto wsum = im1 + im2;

    if (wsum == 0.0)
    {
        return false;
    }

    Vec3d      n   = p1 - p0;
    const auto len = n.norm();
    n /= len;

    const Vec3d gradC = n * m_stiffness * (len - m_restLength) / wsum;

    if (im1 > 0)
    {
        p0 += im1 * gradC;
    }

    if (im2 > 0)
    {
        p1 += -im2 * gradC;
    }
    return true;
}
} // imstk