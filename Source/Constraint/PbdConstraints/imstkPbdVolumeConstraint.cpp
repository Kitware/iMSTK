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

#include "imstkPbdVolumeConstraint.h"

namespace  imstk
{
void
PbdVolumeConstraint::initConstraint(const StdVectorOfVec3d& initVertexPositions,
                                    const size_t& pIdx1, const size_t& pIdx2,
                                    const size_t& pIdx3, const size_t& pIdx4,
                                    double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_vertexIds[2] = pIdx3;
    m_vertexIds[3] = pIdx4;

    m_stiffness = k;

    const Vec3d& p0 = initVertexPositions[pIdx1];
    const Vec3d& p1 = initVertexPositions[pIdx2];
    const Vec3d& p2 = initVertexPositions[pIdx3];
    const Vec3d& p3 = initVertexPositions[pIdx4];

    m_restVolume = (1.0 / 6.0) * ((p1 - p0).cross(p2 - p0)).dot(p3 - p0);
}

bool
PbdVolumeConstraint::solvePositionConstraint(
    StdVectorOfVec3d&      currVertexPositions,
    const StdVectorOfReal& currInvMasses)
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];
    const auto i3 = m_vertexIds[2];
    const auto i4 = m_vertexIds[3];

    Vec3d& x1 = currVertexPositions[i1];
    Vec3d& x2 = currVertexPositions[i2];
    Vec3d& x3 = currVertexPositions[i3];
    Vec3d& x4 = currVertexPositions[i4];

    const auto im1 = currInvMasses[i1];
    const auto im2 = currInvMasses[i2];
    const auto im3 = currInvMasses[i3];
    const auto im4 = currInvMasses[i4];

    const double onesixth = 1.0 / 6.0;

    const Vec3d grad1 = onesixth * (x2 - x3).cross(x4 - x2);
    const Vec3d grad2 = onesixth * (x3 - x1).cross(x4 - x1);
    const Vec3d grad3 = onesixth * (x4 - x1).cross(x2 - x1);
    const Vec3d grad4 = onesixth * (x2 - x1).cross(x3 - x1);

    const auto V = grad4.dot(x4 - x1);

    double lambda = im1 * grad1.squaredNorm() +
                    im2 * grad2.squaredNorm() +
                    im3 * grad3.squaredNorm() +
                    im4 * grad4.squaredNorm();

    lambda = (V - m_restVolume) / lambda * m_stiffness;

    if (im1 > 0)
    {
        x1 += -im1 * lambda * grad1;
    }

    if (im2 > 0)
    {
        x2 += -im2 * lambda * grad2;
    }

    if (im3 > 0)
    {
        x3 += -im3 * lambda * grad3;
    }

    if (im4 > 0)
    {
        x4 += -im4 * lambda * grad4;
    }

    return true;
}
} // imstk