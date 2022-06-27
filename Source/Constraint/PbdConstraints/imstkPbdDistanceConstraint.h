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
/// \class PbdDistanceConstraint
///
/// \brief Distance constraints between two nodal points
///
class PbdDistanceConstraint : public PbdConstraint
{
public:
    PbdDistanceConstraint() : PbdConstraint()
    {
        m_vertexIds.resize(2);
        m_dcdx.resize(2);
    }

    ///
    /// \brief Initializes the distance constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx0,
                        const size_t& pIdx1,
                        const double k = 1e5);

    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

public:
    double m_restLength = 0.0; ///< Rest length between the nodes
};
} // namespace imstk