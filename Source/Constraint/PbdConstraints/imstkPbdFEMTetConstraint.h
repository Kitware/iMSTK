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

#include "imstkPbdFemConstraint.h"

namespace imstk
{
///
/// \class PbdFemTetConstraint
///
/// \brief The FEMTetConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
///
class PbdFemTetConstraint : public PbdFemConstraint
{
public:
    PbdFemTetConstraint(MaterialType mType = MaterialType::StVK) :
        PbdFemConstraint(4, mType) {}

    ///
    /// \brief Get the type of Fem constraint
    ///
    std::string getType() const override { return "FemTet"; }

    ///
    /// \brief Initialize the tetrahedral FEM constraint
    ///
    bool initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx1, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4,
                        std::shared_ptr<PbdFemConstraintConfig> config);

    ///
    /// \brief Compute the value and gradient of constraint
    ///
    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPosition,
        double& c,
        std::vector<Vec3d>& dcdx) const override;
};
}