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
                                    const size_t& pIdx0, const size_t& pIdx1,
                                    const size_t& pIdx2, const size_t& pIdx3,
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

    m_restVolume = (1.0 / 6.0) * ((p1 - p0).cross(p2 - p0)).dot(p3 - p0);
}

bool
PbdVolumeConstraint::computeValueAndGradient(const StdVectorOfVec3d& currVertexPositions,
                                             double&                 c,
                                             StdVectorOfVec3d&       dcdx) const
{
    const auto i0 = m_vertexIds[0];
    const auto i1 = m_vertexIds[1];
    const auto i2 = m_vertexIds[2];
    const auto i3 = m_vertexIds[3];

    const Vec3d& x0 = currVertexPositions[i0];
    const Vec3d& x1 = currVertexPositions[i1];
    const Vec3d& x2 = currVertexPositions[i2];
    const Vec3d& x3 = currVertexPositions[i3];

    const double onesixth = 1.0 / 6.0;

    dcdx[0] = onesixth * (x1 - x2).cross(x3 - x1);
    dcdx[1] = onesixth * (x2 - x0).cross(x3 - x0);
    dcdx[2] = onesixth * (x3 - x0).cross(x1 - x0);
    dcdx[3] = onesixth * (x1 - x0).cross(x2 - x0);

    c = dcdx[3].dot(x3 - x0);
    return true;
}
} // imstk
