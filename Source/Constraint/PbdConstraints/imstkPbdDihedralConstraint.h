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

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdDihedralConstraint
///
/// \brief Angular constraint between two triangular faces
///
class PbdDihedralConstraint : public PbdConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdDihedralConstraint() : PbdConstraint()
    {
        m_vertexIds.resize(4);
        m_dcdx.resize(4);
    }

    ///
    /// \brief Returns PBD constraint of type Type::Dihedral
    ///
    inline Type getType() const override { return Type::Dihedral; }

    /**
      \brief initConstraint
             p3
            / | \
           /  |  \
          p0  |  p1
           \  |  /
            \ | /
              p2
      \param model
      \param pIdx1 index of p0
      \param pIdx2 index of p1
      \param pIdx3 index of p2
      \param pIdx4 index of p3
      \param k stiffness
    */
    void initConstraint(
        const VecDataArray<double, 3>& initVertexPositions,
        const size_t& pIdx0, const size_t& pIdx1,
        const size_t& pIdx2, const size_t& pIdx3,
        const double k);

    ///
    /// \brief Compute value and gradient of the constraint
    ///
    /// \param[in] currVertexPositions vector of current positions
    /// \param[inout] c constraint value
    ///
    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

public:
    double m_restAngle = 0.0; ///> Rest angle
};
} //imstk
