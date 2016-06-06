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

#include "g3log/g3log.hpp"

#include "imstkInternalForceModel.h"

//vega
#include "StVKInternalForces.h"

namespace imstk
{

class LinearFEMForceModel : virtual public InternalForceModel
{
public:
    LinearFEMForceModel(std::shared_ptr<vega::StVKInternalForces> stVKInternalForces);
    virtual ~LinearFEMForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_stiffnessMatrix->MultiplyVector(u.data(), internalForces.data());
    }

    void getTangentStiffnessMatrix(Vectord& u, std::shared_ptr<SparseMatrixd> tangentStiffnessMatrix)
    {
        InternalForceModel::updateValuesFromMatrix(m_stiffnessMatrix, tangentStiffnessMatrix->valuePtr());
    }

protected:

    std::shared_ptr<vega::SparseMatrix> m_stiffnessMatrix;
};

} // imstk

#endif // imstkLinearFEMForceModel_h
