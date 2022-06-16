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

#include "imstkEdgeEdgeCCDState.h"
#include "imstkPbdEdgeEdgeCCDConstraint.h"
#include "imstkLineMeshToLineMeshCCD.h"

namespace imstk
{
void
PbdEdgeEdgeCCDConstraint::initConstraint(
    VertexMassPair prev_ptA1, VertexMassPair prev_ptA2, VertexMassPair prev_ptB1, VertexMassPair prev_ptB2,
    VertexMassPair ptA1, VertexMassPair ptA2, VertexMassPair ptB1, VertexMassPair ptB2,
    double stiffnessA, double stiffnessB)
{
    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;

    m_bodiesFirst[0] = prev_ptA1;
    m_bodiesFirst[1] = prev_ptA2;
    m_bodiesFirst[2] = ptA1;
    m_bodiesFirst[3] = ptA2;

    m_bodiesSecond[0] = prev_ptB1;
    m_bodiesSecond[1] = prev_ptB2;
    m_bodiesSecond[2] = ptB1;
    m_bodiesSecond[3] = ptB2;
}

bool
PbdEdgeEdgeCCDConstraint::computeValueAndGradient(double&             c,
                                                  std::vector<Vec3d>& dcdxA,
                                                  std::vector<Vec3d>& dcdxB) const
{
    EdgeEdgeCCDState prevState(
        *m_bodiesFirst[0].vertex, *m_bodiesFirst[1].vertex,
        *m_bodiesSecond[0].vertex, *m_bodiesSecond[1].vertex);

    EdgeEdgeCCDState currState(
        *m_bodiesFirst[2].vertex, *m_bodiesFirst[3].vertex,
        *m_bodiesSecond[2].vertex, *m_bodiesSecond[3].vertex);

    double timeOfImpact  = 0.0;
    int    collisionType = EdgeEdgeCCDState::testCollision(prevState, currState, timeOfImpact);
    if (collisionType == 0)
    {
        c = 0.0;
        return false;
    }

    double s  = currState.si();
    double t  = currState.sj();
    Vec3d  n0 = prevState.pi() - prevState.pj();
    Vec3d  n1 = currState.pi() - currState.pj();

    Vec3d n = n1;
    // invert the normal if lines are crossing:
    bool crossing = false;
    if (n0.dot(n1) < 0)
    {
        n *= -1.0;
        crossing = true;
    }

    const double d = n.norm();
    if (d <= 0.0)
    {
        c = 0.0;
        return false;
    }
    n /= d;

    // keep the prev values static by assigning zero vector as solution gradient.
    // This can also be done by assigning invMass as zero for prev timestep vertices.
    dcdxA[0] = Vec3d::Zero();
    dcdxA[1] = Vec3d::Zero();
    dcdxB[0] = Vec3d::Zero();
    dcdxB[1] = Vec3d::Zero();

    dcdxA[2] = (1 - s) * n;
    dcdxA[3] = s * n;

    dcdxB[2] = -(1 - t) * n;
    dcdxB[3] = -t * n;

    if (crossing)
    {
        c = d + currState.thickness();
    }
    else
    {
        c = std::abs(d - currState.thickness());
    }

    return true;
}
} // namespace imstk