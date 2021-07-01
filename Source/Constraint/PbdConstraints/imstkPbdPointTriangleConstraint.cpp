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

#include "imstkPbdPointTriangleConstraint.h"

namespace imstk
{
void
PbdPointTriangleConstraint::initConstraint(VertexMassPair ptA,
                                           VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
                                           double stiffnessA, double stiffnessB)
{
    m_bodiesFirst[0] = ptA;

    m_bodiesSecond[0] = ptB1;
    m_bodiesSecond[1] = ptB2;
    m_bodiesSecond[2] = ptB3;

    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;
}

bool
PbdPointTriangleConstraint::computeValueAndGradient(double&             c,
                                                    std::vector<Vec3d>& dcdxA,
                                                    std::vector<Vec3d>& dcdxB) const
{
    const Vec3d& x0 = *m_bodiesFirst[0].vertex;
    const Vec3d& x1 = *m_bodiesSecond[0].vertex;
    const Vec3d& x2 = *m_bodiesSecond[1].vertex;
    const Vec3d& x3 = *m_bodiesSecond[2].vertex;

    // Compute barycentric coordinates u,v,w
    const Vec3d  v0    = x2 - x1;
    const Vec3d  v1    = x3 - x1;
    const Vec3d  v2    = x0 - x1;
    const double d00   = v0.dot(v0);
    const double d01   = v0.dot(v1);
    const double d11   = v1.dot(v1);
    const double d20   = v2.dot(v0);
    const double d21   = v2.dot(v1);
    const double denom = d00 * d11 - d01 * d01;
    const double v     = (d11 * d20 - d01 * d21) / denom;
    const double w     = (d00 * d21 - d01 * d20) / denom;
    const double u     = 1.0 - v - w;

    // This constraint becomes invalid if moved out of the triangle
    if (u < 0.0 || v < 0.0 || w < 0.0)
    {
        c = 0.0;
        return false;
    }

    // Triangle normal (pointing up on standard counter clockwise triangle)
    const Vec3d n = v0.cross(v1).normalized();
    // Point could be on either side of triangle, we want to resolve to the triangles plane
    const double l = v2.dot(n);

    dcdxA[0] = -n;
    dcdxB[0] = u * n;
    dcdxB[1] = v * n;
    dcdxB[2] = w * n;

    c = l;

    return true;
}
} // imstk
