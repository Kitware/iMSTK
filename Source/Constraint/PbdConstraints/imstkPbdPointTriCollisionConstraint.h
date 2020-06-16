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
///
/// \brief The PbdPointTriangleConstraint class for point-triangle collision response
///
class PbdPointTriangleConstraint : public PbdCollisionConstraint
{
public:
    PbdPointTriangleConstraint() : PbdCollisionConstraint(1, 3)
    {}

    ///
    /// \brief Returns the type of the pbd collision constraint
    ///
    Type getType() const
    {
        return Type::PointTriangle;
    }

    ///
    /// \brief initialize constraint
    /// \param pIdxA1 index of the point from object1
    /// \param pIdxB1 first point of the triangle from object2
    /// \param pIdxB2 second point of the triangle from object2
    /// \param pIdxB3 third point of the triangle from object2
    /// \return
    ///
    void initConstraint(const size_t& pIdxA1,
                        const size_t& pIdxB1, const size_t& pIdxB2, const size_t& pIdxB3,
                        std::shared_ptr<PbdCollisionConstraintConfig> configA,
                        std::shared_ptr<PbdCollisionConstraintConfig> configB);

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
};
} // imstk
