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

#include "imstkPbdAngularConstraint.h"

namespace imstk
{
///
/// \class PbdHingeJointConstraint
///
/// \brief Constraint a single oriented particle along an axes of rotation
/// Aligns oriented particles up axes to hinge axes
///
class PbdHingeJointConstraint : public PbdAngularConstraint
{
public:
    PbdHingeJointConstraint() : PbdAngularConstraint(1) { }
    ~PbdHingeJointConstraint() override = default;

    void initConstraint(const PbdParticleId& pIdx0,
                        const Vec3d&         hingeAxes,
                        const double         k);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    Vec3d m_hingeAxes = Vec3d(1.0, 0.0, 0.0); // The axes with which you may rotate around
};
} // namespace imstk