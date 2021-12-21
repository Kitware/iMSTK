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

#include "imstkPbdPointPointConstraint.h"

namespace imstk
{
void
PbdPointPointConstraint::initConstraint(
    VertexMassPair ptA, VertexMassPair ptB,
    double stiffnessA, double stiffnessB)
{
    m_bodiesFirst[0]  = ptA;
    m_bodiesSecond[0] = ptB;

    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;
}

bool
PbdPointPointConstraint::computeValueAndGradient(double&             c,
                                                 std::vector<Vec3d>& dcdxA,
                                                 std::vector<Vec3d>& dcdxB) const
{
    // Current position during solve
    const Vec3d& x_a = *m_bodiesFirst[0].vertex;
    const Vec3d& x_b = *m_bodiesSecond[0].vertex;

    const Vec3d diff = x_b - x_a;
    c = diff.norm();

    if (c == 0.0)
    {
        return false;
    }

    const Vec3d n = diff / c;

    dcdxA[0] = n;
    dcdxB[0] = -n;

    return true;
}
} // imstk