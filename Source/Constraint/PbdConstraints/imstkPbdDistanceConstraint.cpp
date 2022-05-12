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
PbdDistanceConstraint::initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                                      const size_t& pIdx0,
                                      const size_t& pIdx1,
                                      const double k)
{
    m_vertexIds[0] = pIdx0;
    m_vertexIds[1] = pIdx1;
    m_stiffness    = k;
    m_compliance   = 1.0 / k;

    const Vec3d& p0 = initVertexPositions[pIdx0];
    const Vec3d& p1 = initVertexPositions[pIdx1];

    m_restLength = (p0 - p1).norm();
}

bool
PbdDistanceConstraint::computeValueAndGradient(
    const VecDataArray<double, 3>& currVertexPositions,
    double& c,
    std::vector<Vec3d>& dcdx) const
{
    const Vec3d& p0 = currVertexPositions[m_vertexIds[0]];
    const Vec3d& p1 = currVertexPositions[m_vertexIds[1]];

    dcdx[0] = p0 - p1;
    const double len = dcdx[0].norm();
    if (len == 0.0)
    {
        return false;
    }
    dcdx[0] /= len;
    dcdx[1]  = -dcdx[0];
    c        = len - m_restLength;

    return true;
}
} // namespace imstk
