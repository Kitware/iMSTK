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

#include "imstkPbdPointNormalCollisionConstraint.h"

namespace imstk
{
void
PbdPointNormalCollisionConstraint::initConstraint(std::shared_ptr<PbdCollisionConstraintConfig> configA,
                                                  const Vec3d& contactPt, const Vec3d& penetrationVector, const int nodeId)
{
    m_contactPt = contactPt;
    m_penetrationDepth = penetrationVector.norm();
    m_normal = penetrationVector.normalized();
    m_bodiesFirst[0] = nodeId;
    m_configA = configA;
}

bool
PbdPointNormalCollisionConstraint::computeValueAndGradient(const VecDataArray<double, 3>& currVertexPositionsA,
                                                           const VecDataArray<double, 3>& imstkNotUsed(currVertexPositionsB),
                                                           double& c,
                                                           VecDataArray<double, 3>& dcdxA,
                                                           VecDataArray<double, 3>& dcdxB) const
{
    // Current position during solve
    const Vec3d& x = currVertexPositionsA[m_bodiesFirst[0]];

    // Project diff onto normal
    const Vec3d diff = x - m_contactPt;
    // Actual penetration depth (thus far in solve)
    c = std::max(std::min(diff.dot(-m_normal), m_penetrationDepth), 0.0);

    dcdxA.resize(1);
    dcdxB.resize(0);
    dcdxA[0] = -m_normal;

    return true;
}
} // imstk