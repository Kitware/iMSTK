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

#include <functional>

using namespace imstk;

///
/// \class RbdLinearNeedleLockingConstraint
///
/// \brief Constrains the body to specified orientation
/// and only allows linear movement along the inital axes
///
class RbdLinearNeedleLockingConstraint : public RbdConstraint
{
public:
    RbdLinearNeedleLockingConstraint(
        std::shared_ptr<RigidBody> obj,
        const Vec3d&               initNeedleAxesPt,
        const Vec3d&               initNeedleAxes,
        //const Quatd& initNeedleOrientation,
        const double               beta = 0.05) : RbdConstraint(obj, nullptr, Side::A),
        m_initNeedleAxesPt(initNeedleAxesPt),
        m_initNeedleAxes(initNeedleAxes),
        //m_initNeedleOrientation(initNeedleOrientation),
        m_beta(beta)
    {
    }

    ~RbdLinearNeedleLockingConstraint() override = default;

public:
    void compute(double dt) override
    {
        // Jacobian of contact (defines linear and angular constraint axes)
        J = Eigen::Matrix<double, 3, 4>::Zero();
        if ((m_side == Side::AB || m_side == Side::A) && !m_obj1->m_isStatic)
        {
            // Displacement to needle Axes
            const Vec3d  diff = m_obj1->getPosition() - m_initNeedleAxesPt;
            const Vec3d  perpDisplacement = diff - m_initNeedleAxes.dot(diff) * m_initNeedleAxes;
            const double displacement     = perpDisplacement.norm();
            if (displacement != 0)
            {
                const Vec3d displacementDir = perpDisplacement / displacement;
                vu = displacement * m_beta / dt;

                // Displacement from center of mass
                J(0, 0) = -displacementDir[0]; J(0, 1) = 0.0;
                J(1, 0) = -displacementDir[1]; J(1, 1) = 0.0;
                J(2, 0) = -displacementDir[2]; J(2, 1) = 0.0;
            }
            else
            {
                vu = 0.0;
            }
        }
    }

private:
    Vec3d  m_initNeedleAxesPt; ///> Point on the axes to constrain too
    Vec3d  m_initNeedleAxes;   //> Axes to constrain too
    double m_beta = 0.05;
};