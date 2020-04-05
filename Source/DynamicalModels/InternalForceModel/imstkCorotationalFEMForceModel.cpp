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

#include "imstkCorotationalFEMForceModel.h"

#pragma warning( push )
#pragma warning( disable : 4458 )
//vega
#include "corotationalLinearFEM.h"
#pragma warning( pop )

namespace imstk
{
CorotationalFEMForceModel::CorotationalFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, const int warp) : InternalForceModel(), m_warp(warp)
{
    auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);
    m_corotationalLinearFEM = std::make_shared<vega::CorotationalLinearFEM>(tetMesh.get());
}

void
CorotationalFEMForceModel::getInternalForce(const Vectord& u, Vectord& internalForce)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(data, internalForce.data(), nullptr, m_warp);
}

void
CorotationalFEMForceModel::getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(data, nullptr, m_vegaTangentStiffnessMatrix.get(), m_warp);
    InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
}

void
CorotationalFEMForceModel::getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
{
    m_corotationalLinearFEM->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
}

void
CorotationalFEMForceModel::getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(data, internalForce.data(), m_vegaTangentStiffnessMatrix.get(), m_warp);
    InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
}

void
CorotationalFEMForceModel::setWarp(const int warp)
{
    m_warp = warp;
}

void
CorotationalFEMForceModel::setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K)
{
    m_vegaTangentStiffnessMatrix = K;
}
}