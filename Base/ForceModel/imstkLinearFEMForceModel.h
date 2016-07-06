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

#ifndef imstkLinearFEMForceModel_h
#define imstkLinearFEMForceModel_h

#include <memory>

#include "g3log/g3log.hpp"

#include "imstkInternalForceModel.h"

//vega
#include "StVKInternalForces.h"
#include "StVKStiffnessMatrix.h"

namespace imstk
{

class LinearFEMForceModel : public InternalForceModel
{
public:
    LinearFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, bool withGravity = true, double gravity = -9.81) : InternalForceModel()
    {
        auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);

        m_stVKInternalForces = std::make_shared<vega::StVKInternalForces>(tetMesh.get(), nullptr, withGravity, gravity);

        auto stVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());
        auto s = m_stiffnessMatrix.get();
        stVKStiffnessMatrix->GetStiffnessMatrixTopology(&s);
        double * zero = (double*)calloc(m_stiffnessMatrix->GetNumRows(), sizeof(double));
        stVKStiffnessMatrix->ComputeStiffnessMatrix(zero, m_stiffnessMatrix.get());
        free(zero);
        //delete(stVKStiffnessMatrix);
    };

    virtual ~LinearFEMForceModel(){};

    void getInternalForce(const Vectord& u, Vectord& internalForce)
    {
        double *data = const_cast<double*>(u.data());
        m_stiffnessMatrix->MultiplyVector(data, internalForce.data());
    }

    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
    {
        InternalForceModel::updateValuesFromMatrix(m_stiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
    {
        *tangentStiffnessMatrix = new vega::SparseMatrix(*m_stiffnessMatrix.get());
    }

    void GetForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
    {
        getInternalForce(u, internalForce);
        getTangentStiffnessMatrix(u, tangentStiffnessMatrix);
    }
protected:
    std::shared_ptr<vega::SparseMatrix> m_stiffnessMatrix;
    std::shared_ptr<vega::StVKInternalForces> m_stVKInternalForces;
};

} // imstk

#endif // imstkLinearFEMForceModel_h
