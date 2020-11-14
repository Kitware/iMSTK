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
#include "imstkCollisionData.h"

namespace imstk
{
struct MeshToAnalyticalCollisionDataElement;
////
/// \class PbdAnalyticalCollisionConstraint
///
/// \brief Area constraint for triangular face
///
class PbdAnalyticalCollisionConstraint : public PbdCollisionConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdAnalyticalCollisionConstraint() : PbdCollisionConstraint(1, 0)
    {}

    ///
    /// \brief Returns the type of the pbd collision constraint
    ///
    Type getType() const
    {
        return Type::Analytical;
    }

    ///
    /// \brief initialize constraint
    /// \param pIdxA1 index of the point from object1
    /// \return
    ///
    void initConstraint(std::shared_ptr<PbdCollisionConstraintConfig> configA, const MeshToAnalyticalCollisionDataElement& MAColData);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[in] currVertexPositionsA current positions from object A
    /// \param[in] currVertexPositionsA current positions from object B
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(const StdVectorOfVec3d& currVertexPositionsA,
                                 const StdVectorOfVec3d& currVertexPositionsB,
                                 double&                 c,
                                 StdVectorOfVec3d&       dcdxA,
                                 StdVectorOfVec3d&       dcdxB) const override;

public:
    Vec3d m_penetrationVector = Vec3d::Zero();
};
} // imstk