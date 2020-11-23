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
/// \class PbdEdgeEdgeConstraint
///
/// \brief The PbdEdgeEdgeConstraint class for edge-edge collision response
///
class PbdEdgeEdgeConstraint : public PbdCollisionConstraint
{
public:
    PbdEdgeEdgeConstraint() : PbdCollisionConstraint(2, 2) {}

    ///
    /// \brief Get the type of pbd constraint
    ///
    Type getType() const { return Type::EdgeEdge; }

    ///
    /// \brief initialize constraint
    /// \param pIdx1 first point of the edge from object1
    /// \param pIdx2 second point of the edge from object1
    /// \param pIdx3 first point of the edge from object2
    /// \param pIdx4 second point of the edge from object2
    /// \return  true if succeeded
    ///
    void initConstraint(
        const size_t& pIdxA1, const size_t& pIdxA2,
        const size_t& pIdxB1, const size_t& pIdxB2,
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
    bool computeValueAndGradient(const VecDataArray<double, 3>& currVertexPositionsA,
                                 const VecDataArray<double, 3>& currVertexPositionsB,
                                 double& c,
                                 VecDataArray<double, 3>& dcdxA,
                                 VecDataArray<double, 3>& dcdxB) const override;
};
} // imstk
