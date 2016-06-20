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

class LinearFEMForceModel : virtual public InternalForceModel
{
public:
    LinearFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, bool withGravity = true, double gravity = -9.81) : InternalForceModel()
    {
        m_stVKInternalForces = std::make_shared<vega::StVKInternalForces>(mesh.get(), 0, withGravity, gravity);

        auto stVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());
        stVKStiffnessMatrix->GetStiffnessMatrixTopology(&m_stiffnessMatrix.get());
        double * zero = (double*)calloc(m_stiffnessMatrix->GetNumRows(), sizeof(double));
        stVKStiffnessMatrix->ComputeStiffnessMatrix(zero, m_stiffnessMatrix.get());
        free(zero);
        delete(stVKStiffnessMatrix);
    };

    virtual ~LinearFEMForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_stiffnessMatrix->MultiplyVector(u.data(), internalForce.data());
    }

    void getTangentStiffnessMatrix(Vectord& u, SparseMatrixd tangentStiffnessMatrix)
    {
        InternalForceModel::updateValuesFromMatrix(m_stiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

protected:
    std::shared_ptr<vega::SparseMatrix> m_stiffnessMatrix;
    std::shared_ptr<vega::StVKInternalForces> m_stVKInternalForces;
};

} // imstk

#endif // imstkLinearFEMForceModel_h
