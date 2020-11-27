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

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
////
/// \class PbdPointNormalCollisionConstraint
///
/// \brief This constraint allows us only to move only along a normal (penetrationVector) direction to try to converge on a contact/target point
///
class PbdPointNormalCollisionConstraint : public PbdCollisionConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdPointNormalCollisionConstraint() : PbdCollisionConstraint(1, 0) { }

    ///
    /// \brief Destructor
    /// 
    ~PbdPointNormalCollisionConstraint() override = default;

public:
    ///
    /// \brief Returns the type of the pbd collision constraint
    ///
    Type getType() const { return Type::Analytical; }

    ///
    /// \brief initialize constraint
    /// \param contactPt, the point to resolve too (target point)
    /// \param penetrationVector, the vector that gets us from current position x to contactPt
    /// \param nodeId index of the point from object1 that we want to move
    /// \return
    ///
    void initConstraint(std::shared_ptr<PbdCollisionConstraintConfig> configA, const Vec3d& contactPt, const Vec3d& penetrationVector, const int nodeId);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[in] currVertexPositionsA current positions from object A
    /// \param[in] currVertexPositionsA current positions from object B
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(const VecDataArray<double, 3>& currVertexPositionsA,
                                 const VecDataArray<double, 3>& currVertexPositionsB,
                                 double& c,
                                 VecDataArray<double, 3>& dcdxA,
                                 VecDataArray<double, 3>& dcdxB) const override;

public:
    //double m_penetrationDepth = 0.0;
    Vec3d m_normal = Vec3d::Zero();
    Vec3d m_contactPt = Vec3d::Zero();
    double m_penetrationDepth = 0.0;
};
} // imstk