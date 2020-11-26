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

#pragma once

#include "imstkPbdAnalyticalCollisionConstraint.h"

namespace imstk
{
void
PbdAnalyticalCollisionConstraint::initConstraint(std::shared_ptr<PbdCollisionConstraintConfig> configA,
                                                 const MeshToAnalyticalCollisionDataElement&   MAColData)
{
    m_penetrationVector = MAColData.penetrationVector;
    m_bodiesFirst[0]    = MAColData.nodeIdx;
    m_configA = configA;
}

bool
PbdAnalyticalCollisionConstraint::computeValueAndGradient(const VecDataArray<double, 3>& currVertexPositionsA,
                                                          const VecDataArray<double, 3>& currVertexPositionsB,
                                                          double& c,
                                                          VecDataArray<double, 3>& dcdxA,
                                                          VecDataArray<double, 3>& dcdxB) const
{
    c = m_penetrationVector.norm();
    dcdxA.resize(1);
    dcdxB.resize(0);
    dcdxA[0] = m_penetrationVector / c;
    return true;
}
} // imstk