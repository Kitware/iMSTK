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

#ifndef imstkStVKForceModel_h
#define imstkStVKForceModel_h

#include <memory>
#include "g3log/g3log.hpp"

#include "imstkInternalForceModel.h"

//vega
#include "StVKInternalForces.h"
#include "StVKStiffnessMatrix.h"

namespace imstk
{

class StVKForceModel : virtual public InternalForceModel
{
public:
    StVKForceModel(std::shared_ptr<vega::VolumetricMesh> mesh, bool withGravity = true, double gravity = 10.0)
    {
        m_stVKInternalForces = std::make_shared<vega::StVKInternalForces>(mesh.get(), 0, withGravity, gravity);

        m_vegaTangentStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());
    }

    virtual ~StVKForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_stVKInternalForces->ComputeForces(u.data(), internalForce.data());
    }

    void getTangentStiffnessMatrix(Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
    {
        m_stVKInternalForces->ComputeStiffnessMatrix(u.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

protected:

    std::shared_ptr<vega::StVKInternalForces> m_stVKInternalForces;
    std::shared_ptr<vega::StVKStiffnessMatrix> m_vegaTangentStiffnessMatrix;
    bool ownStiffnessMatrix;
};

} // imstk

#endif // imstkStVKForceModel_h
