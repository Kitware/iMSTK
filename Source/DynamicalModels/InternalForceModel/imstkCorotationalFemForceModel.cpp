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

#include "imstkCorotationalFemForceModel.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4458 )
#endif
#include <corotationalLinearFEM.h>
#ifdef WIN32
#pragma warning( pop )
#endif

namespace imstk
{
CorotationalFemForceModel::CorotationalFemForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, const int warp) : InternalForceModel(), m_warp(warp)
{
    auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);
    m_corotationalLinearFem = std::make_shared<vega::CorotationalLinearFEM>(tetMesh.get());
}

void
CorotationalFemForceModel::getInternalForce(const Vectord& u, Vectord& internalForce)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFem->ComputeEnergyAndForceAndStiffnessMatrix(data, nullptr, internalForce.data(), nullptr, m_warp);
}

void
CorotationalFemForceModel::getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFem->ComputeEnergyAndForceAndStiffnessMatrix(data, nullptr, nullptr, m_vegaTangentStiffnessMatrix.get(), m_warp);
    InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
}

void
CorotationalFemForceModel::getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
{
    m_corotationalLinearFem->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
}

void
CorotationalFemForceModel::getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
{
    double* data = const_cast<double*>(u.data());
    m_corotationalLinearFem->ComputeEnergyAndForceAndStiffnessMatrix(data, nullptr, internalForce.data(), m_vegaTangentStiffnessMatrix.get(), m_warp);
    InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
}

void
CorotationalFemForceModel::setWarp(const int warp)
{
    m_warp = warp;
}

void
CorotationalFemForceModel::setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K)
{
    m_vegaTangentStiffnessMatrix = K;
}
} // namespace imstk
