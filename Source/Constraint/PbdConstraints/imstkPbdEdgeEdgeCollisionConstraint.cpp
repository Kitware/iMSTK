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
#include "imstkLogger.h"

namespace imstk
{
void
PbdEdgeEdgeConstraint::initConstraint(
    const size_t& pIdxA1, const size_t& pIdxA2,
    const size_t& pIdxB1, const size_t& pIdxB2,
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
PbdEdgeEdgeConstraint::solvePositionConstraint(
    StdVectorOfVec3d&      currVertexPositionsA,
    StdVectorOfVec3d&      currVertexPositionsB,
    const StdVectorOfReal& currInvMassesA,
    const StdVectorOfReal& currInvMassesB)
{
    const auto i0 = m_bodiesFirst[0];
    const auto i1 = m_bodiesFirst[1];
    const auto i2 = m_bodiesSecond[0];
    const auto i3 = m_bodiesSecond[1];

    Vec3d& x0 = currVertexPositionsA[i0];
    Vec3d& x1 = currVertexPositionsA[i1];
    Vec3d& x2 = currVertexPositionsB[i2];
    Vec3d& x3 = currVertexPositionsB[i3];

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
        return false;
    }

    Vec3d grad0 = -(1 - t) * n;
    Vec3d grad1 = -(t) * n;
    Vec3d grad2 = (1 - s) * n;
    Vec3d grad3 = (s) * n;

    const auto im0 = currInvMassesA[i0];
    const auto im1 = currInvMassesA[i1];
    const auto im2 = currInvMassesB[i2];
    const auto im3 = currInvMassesB[i3];

    auto lambda = im0 * grad0.squaredNorm() +
                  im1 * grad1.squaredNorm() +
                  im2 * grad2.squaredNorm() +
                  im3 * grad3.squaredNorm();

    lambda = (l - dist) / lambda;

//    LOG(INFO) << "Lambda:" << lambda <<" Normal:" << n[0] <<" " << n[1] <<" "<<n[2];

    if (im0 > 0)
    {
        x0 += -im0 * lambda * grad0 * m_configA->m_stiffness;
    }

    if (im1 > 0)
    {
        x1 += -im1 * lambda * grad1 * m_configA->m_stiffness;
    }

    if (im2 > 0)
    {
        x2 += -im2 * lambda * grad2 * m_configB->m_stiffness;
    }

    if (im3 > 0)
    {
        x3 += -im3 * lambda * grad3 * m_configB->m_stiffness;
    }

    return true;
}
} // imstk
