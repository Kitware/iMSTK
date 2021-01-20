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

#include "imstkRbdFrictionConstraint.h"
#include "imstkMacros.h"

namespace imstk
{
namespace expiremental
{
RbdFrictionConstraint::RbdFrictionConstraint(
    std::shared_ptr<RigidBody> obj1,
    std::shared_ptr<RigidBody> obj2,
    const Vec3d&               contactNormal,
    const Vec3d&               contactPt,
    const double               contactDepth,
    const double               frictionCoefficient,
    const Side                 side) : RbdConstraint(obj1, obj2, side),
    m_contactPt(contactPt), m_contactN(contactNormal),
    m_contactDepth(contactDepth), m_frictionCoefficient(frictionCoefficient)
{
}

void
RbdFrictionConstraint::compute(double imstkNotUsed(dt))
{
    // Displacement from center of mass
    //const Vec3d r1 = m_contactPt - m_obj1->getPosition();
    //const Vec3d r2 = m_contactPt - m_obj2->getPosition();

    // Jacobian of contact
    J = Eigen::Matrix<double, 3, 4>::Zero();
    if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
    {
        const double vN   = m_contactN.dot(m_obj1->getVelocity());
        const Vec3d  vTan = m_obj1->getVelocity() - vN * m_contactN;
        const Vec3d  tan  = vTan.normalized();

        // No angular friction
        J(0, 0) = -tan[0]; J(0, 1) = 0.0;
        J(1, 0) = -tan[1]; J(1, 1) = 0.0;
        J(2, 0) = -tan[2]; J(2, 1) = 0.0;

        const double fNMag = std::max(0.0, m_obj1->getForce().dot(-m_contactN));
        const double fu    = m_frictionCoefficient * fNMag;
        range[0] = -fu;
        range[1] = fu;
    }
    if ((m_side == Side::AB || m_side == Side::B) && !m_obj2->m_isStatic)
    {
        const double vN   = m_contactN.dot(m_obj2->getVelocity());
        const Vec3d  vTan = m_obj2->getVelocity() - vN * -m_contactN;
        const Vec3d  tan  = vTan.normalized();

        // No angular friction
        J(0, 0) = tan[0]; J(0, 1) = 0.0;
        J(1, 0) = tan[1]; J(1, 1) = 0.0;
        J(2, 0) = tan[2]; J(2, 1) = 0.0;

        // Get normal force
        const double fNMag = std::max(0.0, m_obj2->getForce().dot(m_contactN));
        const double fu    = m_frictionCoefficient * fNMag;
        range[0] = -fu;
        range[1] = fu;
    }

    /*if both? Kinda need two ranges
    {
        J(0, 0) = -tan[0]; J(0, 1) = -tan[1]; J(0, 2) = -tan[2];
        J(1, 0) = 0.0;     J(1, 1) = 0.0;     J(1, 2) = 0.0;
        J(2, 0) = tan[0];  J(2, 1) = tan[1];  J(2, 2) = tan[2];
        J(3, 0) = 0.0;     J(3, 1) = 0.0;     J(3, 2) = 0.0;

        const Vec3d  netForce = m_obj1->getForce() + m_obj2->getForce();
        const double fNMag    = std::max(0.0, netForce.dot(m_contactN));
        const double fu       = m_frictionCoefficient * fNMag;
        range[0] = -fu;
        range[1] = fu;
    }*/
}
}
};