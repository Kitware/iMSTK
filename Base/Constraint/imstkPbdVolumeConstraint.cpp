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

#include "imstkPbdVolumeConstraint.h"
#include "imstkPbdModel.h"

namespace  imstk
{

void
VolumeConstraint::initConstraint(PositionBasedDynamicsModel &model, const unsigned int &pIdx1,
                                 const unsigned int &pIdx2, const unsigned int &pIdx3,
                                 const unsigned int &pIdx4, const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;
    m_bodies[3] = pIdx4;

    m_stiffness = k;

    auto state = model.getState();

    const Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    const Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    const Vec3d &p2 = state->getInitialVertexPosition(pIdx3);
    const Vec3d &p3 = state->getInitialVertexPosition(pIdx4);

    m_restVolume = (1.0 / 6.0)*((p1 - p0).cross(p2 - p0)).dot(p3 - p0);
}

bool
VolumeConstraint::solvePositionConstraint(PositionBasedDynamicsModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];
    const unsigned int i4 = m_bodies[3];

    auto state = model.getState();

    Vec3d &x1 = state->getVertexPosition(i1);
    Vec3d &x2 = state->getVertexPosition(i2);
    Vec3d &x3 = state->getVertexPosition(i3);
    Vec3d &x4 = state->getVertexPosition(i4);

    const double im1 = state->getInvMass(i1);
    const double im2 = state->getInvMass(i2);
    const double im3 = state->getInvMass(i3);
    const double im4 = state->getInvMass(i4);

    const double onesixth = 1.0 / 6.0;

    const Vec3d grad1 = onesixth*(x2 - x3).cross(x4 - x2);
    const Vec3d grad2 = onesixth*(x3 - x1).cross(x4 - x1);
    const Vec3d grad3 = onesixth*(x4 - x1).cross(x2 - x1);
    const Vec3d grad4 = onesixth*(x2 - x1).cross(x3 - x1);

    const double V = grad4.dot(x4 - x1);

    double lambda = im1*grad1.squaredNorm() +
        im2*grad2.squaredNorm() +
        im3*grad3.squaredNorm() +
        im4*grad4.squaredNorm();

    lambda = (V - m_restVolume) / lambda * m_stiffness;

    if (im1 > 0)
    {
        x1 += -im1*lambda*grad1;
    }

    if (im1 > 0)
    {
        x2 += -im2*lambda*grad2;
    }

    if (im3 > 0)
    {
        x3 += -im3*lambda*grad3;
    }

    if (im4 > 0)
    {
        x4 += -im4*lambda*grad4;
    }

    return true;
}

} // imstk