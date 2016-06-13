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

#ifndef imstkMassSpringForceModel_h
#define imstkMassSpringForceModel_h

#include "g3log/g3log.hpp"

#include "imstkInternalForceModel.h"

//vega
#include "massSpringSystem.h"

namespace imstk
{

class MassSpringForceModel : virtual public InternalForceModel
{
public:
    MassSpringForceModel(std::shared_ptr<vega::MassSpringSystem> massSpringSystem)
    {
        m_massSpringSystem = massSpringSystem;
    }
    virtual ~MassSpringForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_massSpringSystem->ComputeForce(u.data(), internalForce.data());
    }

    void getTangentStiffnessMatrix(Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
    {
        m_massSpringSystem->ComputeStiffnessMatrix(u.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

protected:
    std::shared_ptr<vega::MassSpringSystem> m_massSpringSystem;// Need to be initialized
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
};

} // imstk

#endif // imstkMassSpringForceModel_h
