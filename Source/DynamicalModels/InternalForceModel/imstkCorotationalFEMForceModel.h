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

#include <memory>

#include "g3log/g3log.hpp"

//imstk
#include "imstkInternalForceModel.h"

//vega
#include "corotationalLinearFEM.h"

namespace imstk
{
///
/// \class CorotationalFEMForceModel
///
/// \brief Force model for corotational based finite elements formulation
///
class CorotationalFEMForceModel : public InternalForceModel
{
public:
    ///
    /// \brief
    ///
    CorotationalFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, const int warp = 1);
    CorotationalFEMForceModel() = delete;

    ///
    /// \brief
    ///
    virtual ~CorotationalFEMForceModel() = default;

    ///
    /// \brief
    ///
    void getInternalForce(const Vectord& u, Vectord& internalForce) override;

    ///
    /// \brief
    ///
    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief
    ///
    void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override;

    ///
    /// \brief
    ///
    void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override;

    ///
    /// \brief
    ///
    void setWarp(const int warp);

    ///
    /// \brief
    ///
    void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override;

protected:
    std::shared_ptr<vega::CorotationalLinearFEM> m_corotationalLinearFEM;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
    int m_warp;
};
} // imstk

