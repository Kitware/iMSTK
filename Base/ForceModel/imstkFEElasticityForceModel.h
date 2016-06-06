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

#ifndef imstkFeElasticityForceModel_h
#define imstkFeElasticityForceModel_h

#include "g3log/g3log.hpp"

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
    using class vega::ForceModel FEElasticForceModel;

public:
    ///
    /// \brief Constructor
    ///
    FEElasticityForceModel(const TimeIntegrator::Type type);

    ///
    /// \brief Destructor
    ///
    ~FEElasticityForceModel() = default;

    void getInternalForce(Vectord& u, Vectord& internalForce) override
    {
        m_feElasticForceModel->GetInternalForce(u.data(), internalForce.data());
    };

    void getTangentStiffnessMatrix(Vectord& u, std::shared_ptr<SparseMatrixd> tangentStiffnessMatrix) override
    {
        m_feElasticForceModel->GetTangentStiffnessMatrixTopology(u.data(), );
    }

protected:
    std::shared_ptr<FEElasticForceModel> m_feElasticForceModel;
};

}

#endif // ifndef imstkTimeIntegrator_h
