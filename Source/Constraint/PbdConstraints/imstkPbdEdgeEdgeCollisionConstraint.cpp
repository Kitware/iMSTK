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

#include "imstkPbdEdgeEdgeCollisionConstraint.h"

namespace imstk
{
void
PbdEdgeEdgeConstraint::initConstraint(const size_t&                                 pIdxA1,
                                      const size_t&                                 pIdxA2,
                                      const size_t&                                 pIdxB1,
                                      const size_t&                                 pIdxB2,
                                      std::shared_ptr<PbdCollisionConstraintConfig> configA,
                                      std::shared_ptr<PbdCollisionConstraintConfig> configB)
{
    m_configA = configA;
    m_configB = configB;
    m_bodiesFirst[0]  = pIdxA1;
    m_bodiesFirst[1]  = pIdxA2;
    m_bodiesSecond[0] = pIdxB1;
    m_bodiesSecond[1] = pIdxB2;
}

bool
PbdEdgeEdgeConstraint::computeValueAndGradient(const VecDataArray<double, 3>& currVertexPositionsA,
                                               const VecDataArray<double, 3>& currVertexPositionsB,
                                               double& cc,
                                               VecDataArray<double, 3>& dcdxA,
                                               VecDataArray<double, 3>& dcdxB) const
{
    const auto i0 = m_bodiesFirst[0];
    const auto i1 = m_bodiesFirst[1];
    const auto i2 = m_bodiesSecond[0];
    const auto i3 = m_bodiesSecond[1];

    const Vec3d& x0 = currVertexPositionsA[i0];
    const Vec3d& x1 = currVertexPositionsA[i1];
    const Vec3d& x2 = currVertexPositionsB[i2];
    const Vec3d& x3 = currVertexPositionsB[i3];

    auto a = (x3 - x2).dot(x1 - x0);
    auto b = (x1 - x0).dot(x1 - x0);
    auto c = (x0 - x2).dot(x1 - x0);
    auto d = (x3 - x2).dot(x3 - x2);
    auto e = a;
    auto f = (x0 - x2).dot(x3 - x2);

    auto   det = a * e - d * b;
    double s   = 0.5;
    double t   = 0.5;
    if (fabs(det) > 1e-12)
    {
        s = (c * e - b * f) / det;
        t = (c * d - a * f) / det;
        if (s < 0 || s > 1.0 || t < 0 || t > 1.0)
        {
            c = 0.0;
            return false;
        }
    }
    else
    {
        //LOG(WARNING) << "det is null";
    }

    Vec3d P = x0 + t * (x1 - x0);
    Vec3d Q = x2 + s * (x3 - x2);

    Vec3d n = Q - P;
    auto  l = n.norm();
    n /= l;

    const auto dist = m_configA->m_proximity + m_configB->m_proximity;

    if (l > dist)
    {
        c = 0.0;
        return false;
    }

    dcdxA.resize(2);
    dcdxB.resize(2);
    dcdxA[0] = -(1 - t) * n;
    dcdxA[1] = -(t) * n;
    dcdxB[0] = (1 - s) * n;
    dcdxB[1] = (s) * n;

    cc = l - dist;

    return true;
}
} // imstk
