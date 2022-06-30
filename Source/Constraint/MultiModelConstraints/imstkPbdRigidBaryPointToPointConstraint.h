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

#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkRbdConstraint.h"

namespace imstk
{
// Define constraint  Note: can be given to either solver
// Single point, build multiple of them for each entity in contact
class PbdRigidBaryPointToPointConstraint : public PbdBaryPointToPointConstraint, public RbdConstraint
{
private:
    double m_beta = 0.0001;

public:
    PbdRigidBaryPointToPointConstraint(std::shared_ptr<RigidBody> obj1);
    ~PbdRigidBaryPointToPointConstraint() override = default;

public:
    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdxA constraint gradient for A
    /// \param[inout] dcdxB constraint gradient for B
    /// Call for RBD, push point on mesh to the fixed point halfway
    /// between the rigid body and the PBD object
    bool computeValueAndGradient(
        double&             c,
        std::vector<Vec3d>& dcdxA,
        std::vector<Vec3d>& dcdxB) const override;

public:
    // Constraint call for RBD
    // Give change in impulse in direction of desired deformation
    void compute(double dt) override;
};
} // namespace imstk