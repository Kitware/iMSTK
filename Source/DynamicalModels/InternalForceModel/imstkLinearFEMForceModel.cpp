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

#include "imstkLinearFemForceModel.h"

#include "imstkMacros.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_HIDES_CLASS_MEMBER

#include <StVKElementABCDLoader.h>
#include <StVKStiffnessMatrix.h>
#include <tetMesh.h>

DISABLE_WARNING_POP

namespace imstk
{
LinearFemForceModel::LinearFemForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                                         const bool withGravity, const double gravity) : InternalForceModel()
{
    auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);

    vega::StVKElementABCD* precomputedIntegrals = vega::StVKElementABCDLoader::load(tetMesh.get());
    m_stVKInternalForces = std::make_shared<vega::StVKInternalForces>(tetMesh.get(), precomputedIntegrals, withGravity, gravity);

    auto stVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());

    stVKStiffnessMatrix->GetStiffnessMatrixTopology(&m_stiffnessMatrixRawPtr);
    std::shared_ptr<vega::SparseMatrix> m_stiffnessMatrix2(m_stiffnessMatrixRawPtr);
    m_stiffnessMatrix = m_stiffnessMatrix2;

    auto K = m_stiffnessMatrix.get();
    stVKStiffnessMatrix->GetStiffnessMatrixTopology(&K);
    double* zero = (double*)calloc(m_stiffnessMatrix->GetNumRows(), sizeof(double));
    stVKStiffnessMatrix->ComputeStiffnessMatrix(zero, m_stiffnessMatrix.get());
    free(zero);
};

LinearFemForceModel::~LinearFemForceModel()
{
    if (m_stiffnessMatrixRawPtr)
    {
        delete m_stiffnessMatrixRawPtr;
    }
}
} // namespace imstk