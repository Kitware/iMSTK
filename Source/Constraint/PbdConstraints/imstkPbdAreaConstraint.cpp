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

#include "imstkPbdAreaConstraint.h"
#include "imstkPbdModel.h"

namespace  imstk
{
void
PbdAreaConstraint::initConstraint(PbdModel &model, const size_t& pIdx1,
                                  const size_t& pIdx2, const size_t& pIdx3,
                                  const double k)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_vertexIds[2] = pIdx3;

    m_stiffness = k;

    auto state = model.getInitialState();

    const Vec3d &p0 = state->getVertexPosition(pIdx1);
    const Vec3d &p1 = state->getVertexPosition(pIdx2);
    const Vec3d &p2 = state->getVertexPosition(pIdx3);

    m_restArea = 0.5*(p1 - p0).cross(p2 - p0).norm();
}

bool
PbdAreaConstraint::solvePositionConstraint(PbdModel& model)
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];
    const auto i3 = m_vertexIds[2];

    auto state = model.getCurrentState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);

    const auto im0 = model.getInvMass(i1);
    const auto im1 = model.getInvMass(i2);
    const auto im2 = model.getInvMass(i3);

    const auto e1 = p0 - p1;
    const auto e2 = p1 - p2;
    const auto e3 = p2 - p0;

    auto n = e1.cross(e2);
    const auto A = 0.5*n.norm();

    if (A < m_epsilon)
    {
        return false;
    }

    n /= 2 * A;

    const auto grad0 = e2.cross(n);
    const auto grad1 = e3.cross(n);
    const auto grad2 = e1.cross(n);

    auto lambda = im0*grad0.squaredNorm() + im1*grad1.squaredNorm() + im2*grad2.squaredNorm();

    lambda = (A - m_restArea) / lambda * m_stiffness;

    if (im0 > 0)
    {
        p0 += -im0*lambda*grad0;
    }

    if (im1 > 0)
    {
        p1 += -im1*lambda*grad1;
    }

    if (im2 > 0)
    {
        p2 += -im2*lambda*grad2;
    }

    return true;
}
} // imstk