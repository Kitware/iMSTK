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

#include "imstkGeometry.h"
#include "imstkDynamicalModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkInternalForceModel.h"
#include "imstkForceModelConfig.h"
#include "imstkKinematicState.h"

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
    /// \brief Set the geometry on which the force model acts
    ///
    void setForceModelGeometry(std::shared_ptr<Geometry> fmGeometry);

    ///
    /// \brief Set/Get force model configuration
    ///
    void setForceModelConfiguration(std::shared_ptr<ForceModelConfig> fmConfig);
    std::shared_ptr<ForceModelConfig> getForceModelConfiguration() const;

    ///
    /// \brief Set/Get force model
    ///
    void setForceModel(std::shared_ptr<InternalForceModel> fm);
    std::shared_ptr<InternalForceModel> getForceModel() const;

    ///
    /// \brief Set/Get time integrator
    ///
    void setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator);
    std::shared_ptr<TimeIntegrator> getTimeIntegrator() const;

    ///
    /// \brief Returns the tangent linear system for a given state
    ///
    void getLinearSystem();

    ///
    /// \brief Configure the force model from external file
    ///
    void configure(const std::string& configFileName);

    ///
    /// \brief Load the boundary conditions from external file
    ///
    bool loadBoundaryConditions();

    ///
    /// \brief Initialize the force model
    ///
    void initializeForceModel();

    ///
    /// \brief Initialize the mass matrix from the mesh
    ///
    void initializeMassMatrix(const bool saveToDisk = false);

    ///
    /// \brief Initialize the damping (combines structural and viscous damping) matrix
    ///
    void initializeDampingMatrix();

    ///
    /// \brief Initialize the tangent stiffness matrix
    ///
    void initializeTangentStiffness();

    ///
    /// \brief Initialize the gravity force
    ///
    void initializeGravity();

    ///
    /// \brief Initialize explicit external forces
    ///
    void initializeExplicitExternalForces();

protected:

    std::shared_ptr<ForceModelConfig>   m_forceModelConfiguration;  ///> Store the configuration here

    std::shared_ptr<InternalForceModel> m_internalForceModel;       ///> Mathematical model for intenal forces
    std::shared_ptr<TimeIntegrator>     m_timeIntegrator;           ///> Time integrator

    std::shared_ptr<Geometry> m_forceModelGeometry;    ///> Geometry used by force model

    /// Matrices typical to a elastodynamics and 2nd order analogous systems
    std::shared_ptr<SparseMatrixd> m_M;    ///> Mass matrix
    std::shared_ptr<SparseMatrixd> m_C;    ///> Damping coefficient matrix
    std::shared_ptr<SparseMatrixd> m_K;    ///> Tangent (derivative of internal force w.r.t displacements) stiffness matrix
    std::shared_ptr<SparseMatrixd> m_Keff; ///> Effective stiffness matrix (dependent on internal force model and time integrator)

    // External field forces
    Vectord m_gravityForce;   ///> Vector of gravity forces

    // Explicit external forces
    Vectord m_explicitExternalForce;   ///> Vector of explicitly defined external forces
};

} // imstk

#endif // ifndef imstkDeformableBodyModel_h
