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
PbdBaryPointToPointConstraint::computePtA() const
{
    Vec3d p1 = Vec3d::Zero();
    for (size_t i = 0; i < m_bodiesFirst.size(); i++)
    {
        p1 += *m_bodiesFirst[i].vertex * m_weightsA[i];
    }
    return p1;
}

Vec3d
PbdBaryPointToPointConstraint::computePtB() const
{
    Vec3d p2 = Vec3d::Zero();
    for (size_t i = 0; i < m_bodiesSecond.size(); i++)
    {
        p2 += *m_bodiesSecond[i].vertex * m_weightsB[i];
    }
    return p2;
}

void
PbdBaryPointToPointConstraint::initConstraint(
    const std::vector<VertexMassPair>& ptsA,
    const std::vector<double>& weightsA,
    const std::vector<VertexMassPair>& ptsB,
    const std::vector<double>& weightsB,
    const double stiffnessA, const double stiffnessB)
{
    m_dcdxA.resize(ptsA.size());
    m_bodiesFirst = ptsA;
    m_weightsA    = weightsA;

    m_dcdxB.resize(ptsB.size());
    m_bodiesSecond = ptsB;
    m_weightsB     = weightsB;

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