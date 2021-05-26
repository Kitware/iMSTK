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

namespace imstk
{
///
/// \class RbdContactConstraint
///
/// \brief A hard rigid body constraint to prevent intersection
///
class RbdContactConstraint : public RbdConstraint
{
public:
    RbdContactConstraint(
        std::shared_ptr<RigidBody> obj1,
        std::shared_ptr<RigidBody> obj2,
        const Vec3d&               contactN,
        const Vec3d&               contactPt,
        const double               contactDepth,
        const double               beta = 0.05,
        const Side                 side = Side::AB) : RbdConstraint(obj1, obj2, side),
        m_contactPt(contactPt), m_contactN(contactN),
        m_contactDepth(contactDepth), m_beta(beta)
    {
    }

    ~RbdContactConstraint() override = default;

public:
    void compute(double dt) override;

private:
    Vec3d  m_contactPt;
    Vec3d  m_contactN;
    double m_contactDepth;
    double m_beta = 0.05;
};
}