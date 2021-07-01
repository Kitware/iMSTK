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

#include "imstkRbdContactConstraint.h"

namespace imstk
{
void
RbdContactConstraint::compute(double dt)
{
    // Jacobian of contact (defines linear and angular constraint axes)
    J = Eigen::Matrix<double, 3, 4>::Zero();
    if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
    {
        // Displacement from center of mass
        const Vec3d r1 = m_contactPt - m_obj1->getPosition();
        const Vec3d c  = r1.cross(m_contactN);
        J(0, 0) = m_contactN[0]; J(0, 1) = c[0];
        J(1, 0) = m_contactN[1]; J(1, 1) = c[1];
        J(2, 0) = m_contactN[2]; J(2, 1) = c[2];
    }
    if ((m_side == Side::AB || m_side == Side::B) && !m_obj2->m_isStatic)
    {
        // Displacement from center of mass
        const Vec3d r2 = m_contactPt - m_obj2->getPosition();
        const Vec3d c  = r2.cross(-m_contactN);
        J(0, 2) = -m_contactN[0]; J(0, 3) = c[0];
        J(1, 2) = -m_contactN[1]; J(1, 3) = c[1];
        J(2, 2) = -m_contactN[2]; J(2, 3) = c[2];
    }

    vu = m_contactDepth * m_beta / dt;
}
}