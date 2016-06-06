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

#ifndef imstkDeformableBodyModel_h
#define imstkDeformableBodyModel_h

#include <memory>

#include "imstkDynamicalModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkInternalForceModel.h"

namespace imstk {

///
/// \class DeformableBodyModel
///
/// \brief Mathematical model of the physics governing the dynamic deformable object
///
class DeformableBodyModel : public DynamicalModel
{
public:
    ///
    /// \brief Constructor
    ///
    DeformableBodyModel();

    ///
    /// \brief Destructor
    ///
    virtual ~DeformableBodyModel() = default;

    ///
    /// \brief Returns the tangent linear system for a given state
    ///
    void getLinearSystem();

protected:
    std::shared_ptr<InternalForceModel> m_internalForceModel; ///> Mathematical model for intenal forces
    std::shared_ptr<TimeIntegrator> m_timeIntegrator; ///> Time integrator

    // Matrices typical to a elasto-dynamics and 2nd order analogous systems
    std::shared_ptr<SparseMatrixd> m_massMatrix;                ///> Mass matrix
    std::shared_ptr<SparseMatrixd> m_dampingMatrix;             ///> Damping coefficient matrix (usually a linear combination of mass and tangent stiffness)
    std::shared_ptr<SparseMatrixd> m_tangentStiffnessMatrix;    ///> Tangent (derivative of internal force w.r.t displacements) stiffness matrix

    std::shared_ptr<SparseMatrixd> m_effectiveStiffnessMatrix;  ///> Effective stiffness matrix (dependent on internal force model and time integrator)

};

}

#endif // ifndef imstkDeformableBodyModel_h
