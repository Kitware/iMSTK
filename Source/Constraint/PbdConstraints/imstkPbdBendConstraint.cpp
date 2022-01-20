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
PbdBendConstraint::initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                                  const size_t& pIdx0,
                                  const size_t& pIdx1,
                                  const size_t& pIdx2,
                                  const double k)
{
    m_vertexIds[0] = pIdx0;
    m_vertexIds[1] = pIdx1;
    m_vertexIds[2] = pIdx2;

    m_stiffness  = k;
    m_compliance = 1.0 / k;

    const Vec3d& p0 = initVertexPositions[pIdx0];
    const Vec3d& p1 = initVertexPositions[pIdx1];
    const Vec3d& p2 = initVertexPositions[pIdx2];

    // Instead of using the angle between the segments we can use the distance
    // from the center of the triangle
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    m_restLength = (p1 - center).norm();
}

bool
PbdBendConstraint::computeValueAndGradient(
    const VecDataArray<double, 3>& currVertexPositions,
    double& c,
    std::vector<Vec3d>& dcdx) const
{
    const size_t i0 = m_vertexIds[0];
    const size_t i1 = m_vertexIds[1];
    const size_t i2 = m_vertexIds[2];

    const Vec3d& p0 = currVertexPositions[i0];
    const Vec3d& p1 = currVertexPositions[i1];
    const Vec3d& p2 = currVertexPositions[i2];

    // Move towards triangle center
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    const Vec3d& diff   = p1 - center;
    const double dist   = diff.norm();

    if (dist < m_epsilon)
    {
        return false;
    }

    c = dist - m_restLength;

    dcdx[0] = (-2.0 / dist) * diff;
    dcdx[1] = -2.0 * dcdx[0];
    dcdx[2] = dcdx[0];

    return true;
}
} // namespace imstk