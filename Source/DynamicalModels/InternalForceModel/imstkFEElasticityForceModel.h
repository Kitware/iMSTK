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

#include "g3log/g3log.hpp"

//imstk
#include "imstkInternalForceModel.h"
#include "imstkMath.h"

// Vega
#include "forceModel.h"

namespace imstk
{
///
/// \class Force Model
///
/// \brief Base class for internal force model for elasticity based on vega's FE
///
class FEElasticityForceModel : public InternalForceModel
{
using class vega::LinearFEMForceModel FEElasticForceModel;

public:
    ///
    /// \brief Constructor
    ///
    FEElasticityForceModel();

    ///
    /// \brief Destructor
    ///
    ~FEElasticityForceModel() = default;

    void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double *data = const_cast<double*>(u.data());
        m_feElasticForceModel->GetInternalForce(data, internalForce.data());
    }

    virtual void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
    {
        m_feElasticForceModel->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
    }

    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        double *data = const_cast<double*>(u.data());
        m_feElasticForceModel->GetTangentStiffnessMatrixTopology(data, );
    }

protected:
    std::shared_ptr<FEElasticForceModel> m_feElasticForceModel;
};
}
