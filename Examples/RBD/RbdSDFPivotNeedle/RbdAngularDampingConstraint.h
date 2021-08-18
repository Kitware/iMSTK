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

#include "imstkRbdConstraint.h"

using namespace imstk;

///
/// \class RbdAngularDampingConstraint
///
/// \brief Damping constraint that scales with depth value provided
///
class RbdAngularDampingConstraint : public RbdConstraint
{
public:
    RbdAngularDampingConstraint(
        std::shared_ptr<RigidBody> obj,
        const double               s,
        const double               beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_scale(s),
        m_beta(beta)
    {
    }

    ~RbdAngularDampingConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            const Vec3d& currAngularVelocity = m_obj1->getAngularVelocity();
            const Vec3d  angularVelDir       = currAngularVelocity.normalized();

            vu      = currAngularVelocity.norm() * m_scale * m_beta / dt;
            J(0, 0) = 0.0; J(0, 1) = -angularVelDir[0];
            J(1, 0) = 0.0; J(1, 1) = -angularVelDir[1];
            J(2, 0) = 0.0; J(2, 1) = -angularVelDir[2];
        }
    }

private:
    double m_beta  = 0.05;
    double m_scale = 1.0;
};