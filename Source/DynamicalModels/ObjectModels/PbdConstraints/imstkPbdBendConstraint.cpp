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

#include "imstkPbdBendConstraint.h"
#include "imstkPbdModel.h"

namespace imstk
{
void
PbdBendConstraint::initConstraint(PbdModel& model,
                                  const size_t& pIdx1, const size_t& pIdx2,
                                  const size_t& pIdx3, const double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_vertexIds[2] = pIdx3;

    m_stiffness = k;
    auto state = model.getInitialState();

    const Vec3d& p0 = state->getVertexPosition(pIdx1);
    const Vec3d& p1 = state->getVertexPosition(pIdx2);
    const Vec3d& p2 = state->getVertexPosition(pIdx3);

    // Instead of using the angle between the segments we can use the distance
    // from the center of the triangle
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    m_restLength = (p1 - center).norm();
}

bool
PbdBendConstraint::solvePositionConstraint(PbdModel& model)
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];
    const auto i3 = m_vertexIds[2];

    auto state = model.getCurrentState();

    Vec3d& p0 = state->getVertexPosition(i1);
    Vec3d& p1 = state->getVertexPosition(i2);
    Vec3d& p2 = state->getVertexPosition(i3);

    const auto im0 = model.getInvMass(i1);
    const auto im1 = model.getInvMass(i2);
    const auto im2 = model.getInvMass(i3);

    // Move towards triangle center
    const Vec3d& center = (p0 + p1 + p2) / 3.0;
    const Vec3d& diff = p1 - center;
    const double dist = diff.norm();
    if (dist < m_epsilon)
    {
        return false;
    }
    const Vec3d& dir = diff / dist;
    const double c = (dist - m_restLength) * m_stiffness;

    // Now weight the applied movements by masses
    double w = (1.0 / im0) + (2.0 / im1) + (1.0 / im2);

    if (im0 > 0)
    {
        double ws = (2.0 / im0) / w;
        p0 += c * dir * ws;
    }

    if (im1 > 0)
    {
        double ws = (4.0 / im1) / w;
        p1 -= c * dir * ws;
    }

    if (im2 > 0)
    {
        double ws = (2.0 / im2) / w;
        p2 += c * dir * ws;
    }

    return true;
}
} // imstk