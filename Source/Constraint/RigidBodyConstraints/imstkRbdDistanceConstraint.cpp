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

#include "imstkRbdDistanceConstraint.h"
#include "imstkMacros.h"

namespace imstk
{
namespace expiremental
{
RbdDistanceConstraint::RbdDistanceConstraint(
    std::shared_ptr<RigidBody> obj1,
    std::shared_ptr<RigidBody> obj2,
    const Vec3d& p1, const Vec3d& p2,
    double dist,
    const Side side) : RbdConstraint(obj1, obj2, side),
    m_p1(p1), m_p2(p2), m_dist(dist)
{
}

void
RbdDistanceConstraint::compute(double imstkNotUsed(dt))
{
    J = Eigen::Matrix<double, 3, 4>::Zero();
    if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
    {
        // Displacement from center of mass
        const Vec3d r1   = m_p1 - m_obj1->getPosition();
        const Vec3d diff = m_p2 - m_p1;
        const Vec3d c    = r1.cross(diff);
        J(0, 0) = -diff[0]; J(0, 1) = -c[0];
        J(1, 0) = -diff[1]; J(1, 1) = -c[1];
        J(2, 0) = -diff[2]; J(2, 1) = -c[2];
    }
    if ((m_side == Side::AB || m_side == Side::B) && !m_obj2->m_isStatic)
    {
        const Vec3d r2   = m_p2 - m_obj2->getPosition();
        const Vec3d diff = m_p2 - m_p1;
        const Vec3d c    = r2.cross(diff);
        J(0, 0) = diff[0]; J(0, 1) = c[0];
        J(1, 0) = diff[1]; J(1, 1) = c[1];
        J(2, 0) = diff[2]; J(2, 1) = c[2];
    }
}
}
}