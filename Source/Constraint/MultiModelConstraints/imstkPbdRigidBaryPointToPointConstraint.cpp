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

#include "imstkPbdRigidBaryPointToPointConstraint.h"

namespace imstk
{
PbdRigidBaryPointToPointConstraint::PbdRigidBaryPointToPointConstraint(std::shared_ptr<RigidBody> obj1) :
    PbdBaryPointToPointConstraint(),
    RbdConstraint(
        obj1,
        nullptr,
        RbdConstraint::Side::A)
{
}

///
/// \brief compute value and gradient of constraint function and weight it
/// by half to force the motion to the half way point between two bodies
///
/// \param[inout] c constraint value
/// \param[inout] dcdxA constraint gradient for A
/// \param[inout] dcdxB constraint gradient for B
bool
PbdRigidBaryPointToPointConstraint::computeValueAndGradient(
    double&             c,
    std::vector<Vec3d>& dcdxA,
    std::vector<Vec3d>& dcdxB) const
{
    // Compute the middle position between the point on the rigid body and the PBD object
    Vec3d diff = 0.5 * computeInterpolantDifference();

    c = diff.norm();

    if (c < IMSTK_DOUBLE_EPS)
    {
        diff = Vec3d::Zero();
        return false;
    }
    diff /= c;

    for (size_t i = 0; i < dcdxA.size(); i++)
    {
        dcdxA[i] = diff * m_weightsA[i];
    }
    for (size_t i = 0; i < dcdxB.size(); i++)
    {
        dcdxB[i] = -diff * m_weightsB[i];
    }

    return true;
}

void
PbdRigidBaryPointToPointConstraint::compute(double dt)
{
    J = Eigen::Matrix<double, 3, 4>::Zero();

    // Compute the middle position between the point on the rigid body and the PBD object
    Vec3d diff = 0.5 * computeInterpolantDifference();

    J(0, 0) = -diff[0]; J(0, 1) = 0.0;
    J(1, 0) = -diff[1]; J(1, 1) = 0.0;
    J(2, 0) = -diff[2]; J(2, 1) = 0.0;

    // B stabilization term
    vu = diff.norm() * m_beta / dt;
}
} // namespace imstk