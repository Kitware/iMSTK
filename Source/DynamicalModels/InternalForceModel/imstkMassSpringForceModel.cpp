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


#include "imstkMassSpringForceModel.h"

namespace imstk
{
MassSpringForceModel::MassSpringForceModel(std::shared_ptr<vega::MassSpringSystem> massSpringSystem) : InternalForceModel()
{
    m_massSpringSystem = massSpringSystem;
}

void
MassSpringForceModel::getInternalForce(const Vectord& u, Vectord& internalForce)
{
    double *data = const_cast<double*>(u.data());
    m_massSpringSystem->ComputeForce(data, internalForce.data());
}

void
MassSpringForceModel::getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
{
    m_massSpringSystem->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
}

void
MassSpringForceModel::getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
{
    double *data = const_cast<double*>(u.data());
    m_massSpringSystem->ComputeStiffnessMatrix(data, m_vegaTangentStiffnessMatrix.get());
    InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
}
} // imstk
