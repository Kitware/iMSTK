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

#include "imstkPbdPointTriCollisionConstraint.h"

#include "imstkLogger.h"

namespace imstk
{
void
PbdPointTriangleConstraint::initConstraint(const size_t&                                 pIdxA1,
                                           const size_t&                                 pIdxB1,
                                           const size_t&                                 pIdxB2,
                                           const size_t&                                 pIdxB3,
                                           std::shared_ptr<PbdCollisionConstraintConfig> configA,
                                           std::shared_ptr<PbdCollisionConstraintConfig> configB)
{
    m_bodiesFirst[0]  = pIdxA1;
    m_bodiesSecond[0] = pIdxB1;
    m_bodiesSecond[1] = pIdxB2;
    m_bodiesSecond[2] = pIdxB3;
    m_configA = configA;
    m_configB = configB;
}

bool
PbdPointTriangleConstraint::computeValueAndGradient(const StdVectorOfVec3d& currVertexPositionsA,
                                                    const StdVectorOfVec3d& currVertexPositionsB,
                                                    double&                 c,
                                                    StdVectorOfVec3d&       dcdxA,
                                                    StdVectorOfVec3d&       dcdxB) const
{
    const size_t i0 = m_bodiesFirst[0];
    const size_t i1 = m_bodiesSecond[0];
    const size_t i2 = m_bodiesSecond[1];
    const size_t i3 = m_bodiesSecond[2];

    const Vec3d& x0 = currVertexPositionsA[i0];
    const Vec3d& x1 = currVertexPositionsB[i1];
    const Vec3d& x2 = currVertexPositionsB[i2];
    const Vec3d& x3 = currVertexPositionsB[i3];

    const Vec3d x12 = x2 - x1;
    const Vec3d x13 = x3 - x1;
    Vec3d       n   = x12.cross(x13);
    const Vec3d x01 = x0 - x1;

    double alpha = n.dot(x12.cross(x01)) / (n.dot(n));
    double beta  = n.dot(x01.cross(x13)) / (n.dot(n));

    if (alpha < 0 || beta < 0 || alpha + beta > 1)
    {
        //LOG(WARNING) << "Projection point not inside the triangle";
        c = 0.0;
        return false;
    }

    const double dist = m_configA->m_proximity + m_configB->m_proximity;

    n.normalize();

    double l = x01.dot(n);

    if (l > dist)
    {
        c = 0.0;
        return false;
    }

    double gamma = 1.0 - alpha - beta;
    dcdxA.resize(1);
    dcdxB.resize(3);
    dcdxA[0] = n;
    dcdxB[0] = -alpha * n;
    dcdxB[1] = -beta * n;
    dcdxB[2] = -gamma * n;

    c = l - dist;
    return true;
}
} // imstk
