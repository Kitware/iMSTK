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
AreaConstraint::initConstraint(PositionBasedDynamicsModel &model, const unsigned int &pIdx1,
                               const unsigned int &pIdx2, const unsigned int &pIdx3,
                               const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;

    m_stiffness = k;

    auto state = model.getState();

    const Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    const Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    const Vec3d &p2 = state->getInitialVertexPosition(pIdx3);

    m_restArea = 0.5*(p1 - p0).cross(p2 - p0).norm();
}

bool
AreaConstraint::solvePositionConstraint(PositionBasedDynamicsModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);

    const double im0 = model.getInvMass(i1);
    const double im1 = model.getInvMass(i2);
    const double im2 = model.getInvMass(i3);

    const Vec3d e1 = p0 - p1;
    const Vec3d e2 = p1 - p2;
    const Vec3d e3 = p2 - p0;

    Vec3d n = e1.cross(e2);
    const double A = 0.5*n.norm();

    if (A < EPS)
    {
        return false;
    }

    n /= 2 * A;

    const Vec3d grad0 = e2.cross(n);
    const Vec3d grad1 = e3.cross(n);
    const Vec3d grad2 = e1.cross(n);

    double lambda = im0*grad0.squaredNorm() + im1*grad1.squaredNorm() + im2*grad2.squaredNorm();

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