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
#include "imstkPbdModel.h"

namespace  imstk
{

void
DistanceConstraint::initConstraint(PositionBasedDynamicsModel& model, const size_t& pIdx1,
                                   const size_t& pIdx2, const double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_stiffness = k;

    auto state = model.getInitialState();
    const Vec3d &p1 = state->getVertexPosition(pIdx1);
    const Vec3d &p2 = state->getVertexPosition(pIdx2);

    m_restLength = (p1 - p2).norm();
}

bool
DistanceConstraint::solvePositionConstraint(PositionBasedDynamicsModel &model)
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];

    auto state = model.getCurrentState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);

    const auto im1 = model.getInvMass(i1);
    const auto im2 = model.getInvMass(i2);

    const auto wsum = im1 + im2;

    if (wsum == 0.0)
    {
        return false;
    }

    Vec3d n = p1 - p0;
    const auto len = n.norm();
    n /= len;

    auto gradC = n*m_stiffness*(len - m_restLength) / wsum;

    if (im1 > 0)
    {
        p0 += im1*gradC;
    }

    if (im2 > 0)
    {
        p1 += -im2*gradC;
    }
    return true;
}

} // imstk