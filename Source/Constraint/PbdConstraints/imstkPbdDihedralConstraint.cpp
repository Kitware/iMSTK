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

#include "imstkPbdDihedralConstraint.h"

namespace  imstk
{
void
PbdDihedralConstraint::initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                                      const size_t& pIdx0,
                                      const size_t& pIdx1,
                                      const size_t& pIdx2,
                                      const size_t& pIdx3,
                                      const double k)
{
    m_vertexIds[0] = pIdx0;
    m_vertexIds[1] = pIdx1;
    m_vertexIds[2] = pIdx2;
    m_vertexIds[3] = pIdx3;

    m_stiffness  = k;
    m_compliance = 1.0 / k;

    const Vec3d& p0 = initVertexPositions[pIdx0];
    const Vec3d& p1 = initVertexPositions[pIdx1];
    const Vec3d& p2 = initVertexPositions[pIdx2];
    const Vec3d& p3 = initVertexPositions[pIdx3];

    const Vec3d n1 = (p2 - p0).cross(p3 - p0).normalized();
    const Vec3d n2 = (p3 - p1).cross(p2 - p1).normalized();

    m_restAngle = atan2(n1.cross(n2).dot(p3 - p2), (p3 - p2).norm() * n1.dot(n2));
}

bool
PbdDihedralConstraint::computeValueAndGradient(
    const VecDataArray<double, 3>& currVertexPositions,
    double& c,
    std::vector<Vec3d>& dcdx) const
{
    const auto i0 = m_vertexIds[0];
    const auto i1 = m_vertexIds[1];
    const auto i2 = m_vertexIds[2];
    const auto i3 = m_vertexIds[3];

    const Vec3d& p0 = currVertexPositions[i0];
    const Vec3d& p1 = currVertexPositions[i1];
    const Vec3d& p2 = currVertexPositions[i2];
    const Vec3d& p3 = currVertexPositions[i3];

    const auto e  = p3 - p2;
    const auto e1 = p3 - p0;
    const auto e2 = p0 - p2;
    const auto e3 = p3 - p1;
    const auto e4 = p1 - p2;
    // To accelerate, all normal (area) vectors and edge length should be precomputed in parallel
    auto       n1 = e1.cross(e);
    auto       n2 = e.cross(e3);
    const auto A1 = n1.norm();
    const auto A2 = n2.norm();
    n1 /= A1;
    n2 /= A2;

    const double l = e.norm();
    if (l < m_epsilon)
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
}