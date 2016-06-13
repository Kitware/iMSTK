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

#ifndef imstkCorotationalFEMForceModel_h
#define imstkCorotationalFEMForceModel_h

#include <memory>

#include "g3log/g3log.hpp"

#include "imstkInternalForceModel.h"

//vega
#include "corotationalLinearFEM.h"

namespace imstk
{

class CorotationalFEMForceModel : virtual public InternalForceModel
{
public:
    CorotationalFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, int warp = 1)
    {
        m_corotationalLinearFEM = std::make_shared<vega::CorotationalLinearFEM>(mesh.get());
        m_warp = warp;
    }

    virtual ~CorotationalFEMForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(u.data(), internalForce.data(), nullptr, m_warp);
    }

    void getTangentStiffnessMatrix(Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
    {
        m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(u.data(), nullptr, m_vegaTangentStiffnessMatrix.get(), m_warp);
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    void GetForceAndMatrix(Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
    {
        m_corotationalLinearFEM->ComputeForceAndStiffnessMatrix(u.data(), internalForce.data(), m_vegaTangentStiffnessMatrix.get(), m_warp);
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    void SetWarp(const int warp)
    {
        this->m_warp = warp;
    }

protected:
    std::shared_ptr<vega::CorotationalLinearFEM> m_corotationalLinearFEM;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
    int m_warp;
};

} // imstk

#endif // imstkCorotationalFEMForceModel_h
