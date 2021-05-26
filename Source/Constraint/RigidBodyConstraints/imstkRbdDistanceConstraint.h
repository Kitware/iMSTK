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

namespace imstk
{
///
/// \class RbdDistanceConstraint
///
/// \brief A rigid body constraint to keep objects at a specified distance
/// from each other given two local points on the bodies
///
class RbdDistanceConstraint : public RbdConstraint
{
public:
    RbdDistanceConstraint(
        std::shared_ptr<RigidBody> obj1,
        std::shared_ptr<RigidBody> obj2,
        const Vec3d& p1, const Vec3d& p2,
        double dist,
        const Side side = Side::AB);
    ~RbdDistanceConstraint() override = default;

public:
    void compute(double dt) override;

private:
    Vec3d  m_p1   = Vec3d(0.0, 0.0, 0.0);
    Vec3d  m_p2   = Vec3d(0.0, 0.0, 0.0);
    double m_dist = 1.0;
};
}