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

// imstk
#include "imstkDynamicalModel.h"
#include "imstkInternalForceModelTypes.h"
#include "imstkNonLinearSystem.h"

// vega
#include "sparseMatrix.h"

namespace vega
{
class VolumetricMesh;
}

namespace imstk
{
class TimeIntegrator;
class VegaMeshIO;
// class NewtonSolver<SparseMatrixd>;
// template <typename SystemMatrix>
// class NewtonSolver;
class InternalForceModel;

struct FEMModelConfig
{
    FEMMethodType m_femMethod = FEMMethodType::Invertible;
    HyperElasticMaterialType m_hyperElasticMaterialType = HyperElasticMaterialType::StVK;

    // file names (remove from here?)
    std::string m_fixedDOFFilename;
    std::vector<std::size_t> m_fixedNodeIds;

    double m_dampingMassCoefficient      = 0.1;
    double m_dampingStiffnessCoefficient = 0.01;
    double m_dampingLaplacianCoefficient = 0.0;
    double m_deformationCompliance       = 1.0;
    double m_compressionResistance       = 500.0;
    double m_inversionThreshold = -std::numeric_limits<double>::max();
    double m_gravity = 9.81;

    // \todo remove from here ?
    int m_numberOfThreads = 4;
};

///
/// \class FEMDeformableBodyModel
///
/// \brief Mathematical model of the physics governing the dynamic deformable object
/// Note: Vega specifics will removed in future when the inertial and damping calculations
/// are done with in-house code
///
class FEMDeformableBodyModel : public DynamicalModel<FeDeformBodyState>
{
using kinematicState = FeDeformBodyState;
using System = NonLinearSystem<SparseMatrixd>;

public:
    ///
    /// \brief Constructor
    ///
    FEMDeformableBodyModel();

    ///
    /// \brief Destructor
    ///
    ~FEMDeformableBodyModel();

    ///
    /// \brief Configure the force model from external file
    ///
    void configure(const std::string& configFileName);
    void configure(std::shared_ptr<FEMModelConfig> config = std::make_shared<FEMModelConfig>());

    ///
    /// \brief Initialize the deformable body model
    ///
    bool initialize() override;

    ///
    /// \brief Set/Get force model configuration
    ///
    void setForceModelConfiguration(std::shared_ptr<FEMModelConfig> fmConfig);
    std::shared_ptr<FEMModelConfig> getForceModelConfiguration() const;

    ///
    /// \brief Set/Get internal force model
    ///
    void setInternalForceModel(std::shared_ptr<InternalForceModel> fm);
    std::shared_ptr<InternalForceModel> getInternalForceModel() const;

    ///
    /// \brief Set/Get time integrator
    ///
    void setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator);
    std::shared_ptr<TimeIntegrator> getTimeIntegrator() const;

    ///
    /// \brief Load the initial conditions of the deformable object
    ///
    void loadInitialStates();

    ///
    /// \brief Load the boundary conditions from external file
    ///
    bool loadBoundaryConditions();

    ///
    /// \brief Initialize the force model
    ///
    bool initializeForceModel();

    ///
    /// \brief Initialize the mass matrix from the mesh
    ///
    bool initializeMassMatrix();

    ///
    /// \brief Initialize the damping (combines structural and viscous damping) matrix
    ///
    bool initializeDampingMatrix();

    ///
    /// \brief Initialize the tangent stiffness matrix
    ///
    bool initializeTangentStiffness();

    ///
    /// \brief Initialize the gravity force
    ///
    bool initializeGravityForce();

    ///
    /// \brief Initialize explicit external forces
    ///
    bool initializeExplicitExternalForces();

    ///
    /// \brief Initialize the Eigen matrix with data inside vega sparse matrix
    ///
    static void initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix, SparseMatrixd& eigenMatrix);

    ///
    /// \brief Compute the RHS of the resulting linear system
    ///
    void computeImplicitSystemRHS(kinematicState& prevState, kinematicState& newState, const StateUpdateType updateType);

    ///
    /// \brief Compute the RHS of the resulting linear system using semi-implicit scheme
    ///
    void computeSemiImplicitSystemRHS(kinematicState& stateAtT, kinematicState& newState, const StateUpdateType updateType);

    ///
    /// \brief Compute the LHS of the resulting linear system
    ///
    void computeImplicitSystemLHS(const kinematicState& prevState, kinematicState& newState, const StateUpdateType updateType);

    ///
    /// \brief Update damping Matrix
    ///
    void updateDampingMatrix();

    ///
    /// \brief Applies boundary conditions to matrix and a vector
    ///
    void applyBoundaryConditions(SparseMatrixd& M, const bool withCompliance = false) const;
    void applyBoundaryConditions(Vectord& x) const;

    ///
    /// \brief Update mass matrix
    /// Note: Not supported yet!
    ///
    void updateMassMatrix();

    ///
    /// \brief Updates the Physics geometry
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Update states
    ///
    void updateBodyStates(const Vectord& solution, const StateUpdateType updateType) override;
    void updateBodyIntermediateStates(const Vectord& solution, const StateUpdateType updateType);

    ///
    /// \brief Update the previous states given the current state
    ///
    void updateBodyPreviousStates();

    ///
    /// \brief Returns the "function" that evaluates the nonlinear function given
    /// the state vector
    ///
    System::VectorFunctionType getFunction();

    ///
    /// \brief Get the function that updates the model given the solution
    ///
    System::UpdateFunctionType getUpdateFunction();
    System::UpdatePrevStateFunctionType getUpdatePrevStateFunction();

    ///
    /// \brief Returns the "function" that evaluates the gradient of the nonlinear
    /// function given the state vector
    ///
    System::MatrixFunctionType getFunctionGradient();

    ///
    /// \brief Get the contact force vector
    ///
    Vectord& getContactForce();

    ///
    /// \brief Returns the unknown vectors
    ///
    Vectord& getUnknownVec() { return m_qSol; }

    ///
    /// \brief Set/Get the update type
    ///
    void setUpdateType(const StateUpdateType& updateType) { m_updateType = updateType; }
    const StateUpdateType& getUpdateType() const { return m_updateType; }

    /// \brief Returns the unknown vectors
    ///
    std::vector<std::size_t>& getFixNodeIds() { return m_fixedNodeIds; }

    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const double timeStep);

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const;

    ///
    /// \brief Set the time step size to fixed size
    ///
    void setFixedSizeTimeStepping();

    ///
    /// \brief Set the fixed BC implementation state
    ///
    void enableFixedBC() { m_implementFixedBC = true; };
    void disableFixedBC() { m_implementFixedBC = false; };
    bool isFixedBCImplemented() const { return m_implementFixedBC; };

protected:
    std::shared_ptr<InternalForceModel> m_internalForceModel;       ///> Mathematical model for intenal forces
    std::shared_ptr<TimeIntegrator>     m_timeIntegrator;           ///> Time integrator
    std::shared_ptr<System>    m_nonLinearSystem;          ///> Nonlinear system resulting from TI and force model

    std::shared_ptr<FEMModelConfig> m_FEModelConfig;

    /// Matrices typical to a elastodynamics and 2nd order analogous systems
    SparseMatrixd m_M;                                                  ///> Mass matrix
    SparseMatrixd m_C;                                                  ///> Damping coefficient matrix
    SparseMatrixd m_K;                                                  ///> Tangent (derivative of internal force w.r.t displacements) stiffness matrix
    SparseMatrixd m_Keff;                                               ///> Effective stiffness matrix (dependent on internal force model and time integrator)

    Vectord m_Finternal;                                                ///> Vector of internal forces
    Vectord m_Feff;                                                     ///> Vector of effective forces
    Vectord m_Fcontact;                                                 ///> Vector of contact forces
    Vectord m_Fgravity;                                                 ///> Vector of gravity forces
    Vectord m_FexplicitExternal;                                        ///> Vector of explicitly defined external forces
    Vectord m_qSol;                                                     ///> Vector to maintain solution at each iteration of nonlinear solver

    std::shared_ptr<vega::VolumetricMesh> m_vegaPhysicsMesh;            ///> Mesh used for Physics
    std::shared_ptr<vega::SparseMatrix>   m_vegaMassMatrix;             ///> Vega mass matrix
    std::shared_ptr<vega::SparseMatrix>   m_vegaTangentStiffnessMatrix; ///> Vega Tangent stiffness matrix
    std::shared_ptr<vega::SparseMatrix>   m_vegaDampingMatrix;          ///> Vega Laplacian damping matrix

    std::vector<std::size_t> m_fixedNodeIds;                            ///> Nodal IDs of the nodes that are fixed

    StateUpdateType m_updateType = StateUpdateType::DeltaVelocity;      ///> Update type of the model

    bool m_damped = false;                                              ///> Viscous or structurally damped system

    // If this is true, the tangent stiffness and force vector will be modified to
    // accommodate (the rows and columns will be nullified) the fixed boundary conditions
    bool m_implementFixedBC = true;
};
} // imstk
