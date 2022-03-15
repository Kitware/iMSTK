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

#include "imstkPbdBaryPointToPointConstraint.h"

namespace imstk
{
Vec3d
PbdBaryPointToPointConstraint::computeInterpolantDifference() const
{
    Vec3d p1 = Vec3d::Zero();
    for (size_t i = 0; i < m_bodiesFirst.size(); i++)
    {
        p1 += *m_bodiesFirst[i].vertex * m_weightsA[i];
    }

    Vec3d p2 = Vec3d::Zero();
    for (size_t i = 0; i < m_bodiesSecond.size(); i++)
    {
        p2 += *m_bodiesSecond[i].vertex * m_weightsB[i];
    }

    return p2 - p1;
}

void
PbdBaryPointToPointConstraint::initConstraint(
    std::vector<VertexMassPair> ptsA,
    std::vector<double> weightsA,
    std::vector<VertexMassPair> ptsB,
    std::vector<double> weightsB,
    double stiffnessA, double stiffnessB)
{
    m_bodiesFirst.resize(ptsA.size());
    m_dcdxA.resize(ptsA.size());
    for (int i = 0; i < ptsA.size(); i++)
    {
        m_bodiesFirst[i] = ptsA[i];
    }
    m_weightsA = weightsA;

    m_bodiesSecond.resize(ptsB.size());
    m_dcdxB.resize(ptsB.size());
    for (int i = 0; i < ptsB.size(); i++)
    {
        m_bodiesSecond[i] = ptsB[i];
    }
    m_weightsB = weightsB;

    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;
}

bool
PbdBaryPointToPointConstraint::computeValueAndGradient(double&             c,
                                                       std::vector<Vec3d>& dcdxA,
                                                       std::vector<Vec3d>& dcdxB) const
{
    // Compute the difference between the interpolant points (points in the two cells)
    Vec3d diff = computeInterpolantDifference();

    c = diff.norm();

    if (c < IMSTK_DOUBLE_EPS)
    {
        diff = Vec3d::Zero();
        return false;
    }
    diff /= c;

    for (size_t i = 0; i < dcdxA.size(); i++)
    {
        dcdxA[i] = diff * m_weightsA[i];
    }
    for (size_t i = 0; i < dcdxB.size(); i++)
    {
        dcdxB[i] = -diff * m_weightsB[i];
    }

    return true;
}
} // namespace imstk