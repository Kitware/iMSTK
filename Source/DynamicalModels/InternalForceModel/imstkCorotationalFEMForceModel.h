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

#include "imstkInternalForceModel.h"

namespace vega
{
class VolumetricMesh;
class CorotationalLinearFEM;
class SparseMatrix;
} // namespace vega

namespace imstk
{
///
/// \class CorotationalFemForceModel
///
/// \brief Force model for corotational based finite elements formulation
///
class CorotationalFemForceModel : public InternalForceModel
{
public:
    ///
    /// \brief Constructor using \p mesh
    /// \param warp if use warp
    ///
    CorotationalFemForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, const int warp = 1);
    CorotationalFemForceModel() = delete;
    ~CorotationalFemForceModel() override = default;

    ///
    /// \brief Compute internal force \p internalForce at state \p u
    ///
    void getInternalForce(const Vectord& u, Vectord& internalForce) override;

    ///
    /// \brief Compute stiffness matrix \p tangentStiffnessMatrix at state \u
    ///
    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief Build the sparsity pattern for stiffness matrix
    ///
    void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override;

    ///
    /// \brief Compute internal force \p internalForce and stiffness matrix \p tangentStiffnessMatrix at state \u
    ///
    void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief Turn on/off warp
    ///
    void setWarp(const int warp);

    ///
    /// \brief Specify tangent stiffness matrix
    ///
    void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override;

protected:
    std::shared_ptr<vega::CorotationalLinearFEM> m_corotationalLinearFem;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
    int m_warp;
};
} // namespace imstk