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

#include "imstkPbdDihedralConstraint.h"
#include "imstkPbdModel.h"

namespace  imstk
{

void
DihedralConstraint::initConstraint(PositionBasedDynamicsModel &model,
                                   const unsigned int &pIdx1, const unsigned int &pIdx2,
                                   const unsigned int &pIdx3, const unsigned int &pIdx4,
                                   const double k)
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

    const Vec3d n1 = (p2 - p0).cross(p3 - p0).normalized();
    const Vec3d n2 = (p3 - p1).cross(p2 - p1).normalized();

    m_restAngle = atan2(n1.cross(n2).dot(p3 - p2), (p3 - p2).norm()*n1.dot(n2));
}

bool
DihedralConstraint::solvePositionConstraint(PositionBasedDynamicsModel& model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];
    const unsigned int i4 = m_bodies[3];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);
    Vec3d &p3 = state->getVertexPosition(i4);

    const double im0 = state->getInvMass(i1);
    const double im1 = state->getInvMass(i2);
    const double im2 = state->getInvMass(i3);
    const double im3 = state->getInvMass(i4);

    if (im0 == 0.0 && im1 == 0.0)
    {
        return false;
    }

    const Vec3d e = p3 - p2;
    const Vec3d e1 = p3 - p0;
    const Vec3d e2 = p0 - p2;
    const Vec3d e3 = p3 - p1;
    const Vec3d e4 = p1 - p2;
    // To accelerate, all normal (area) vectors and edge length should be precomputed in parallel
    Vec3d n1 = e1.cross(e);
    Vec3d n2 = e.cross(e3);
    const double A1 = n1.norm();
    const double A2 = n2.norm();
    n1 /= A1;
    n2 /= A2;

    const double l = e.norm();
    if (l < EPS)
    {
        return false;
    }

    const Vec3d grad0 = -(l / A1)*n1;
    const Vec3d grad1 = -(l / A2)*n2;
    const Vec3d grad2 = (e.dot(e1) / (A1*l))*n1 + (e.dot(e3) / (A2*l))*n2;
    const Vec3d grad3 = (e.dot(e2) / (A1*l))*n1 + (e.dot(e4) / (A2*l))*n2;

    double lambda = im0*grad0.squaredNorm() +
        im1*grad1.squaredNorm() +
        im2*grad2.squaredNorm() +
        im3*grad3.squaredNorm();

    // huge difference if use acos instead of atan2
    lambda = (atan2(n1.cross(n2).dot(e), l*n1.dot(n2)) - m_restAngle) / lambda * m_stiffness;

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

    if (im3 > 0)
    {
        p3 += -im3*lambda*grad3;
    }

    return true;
}

} // imstk