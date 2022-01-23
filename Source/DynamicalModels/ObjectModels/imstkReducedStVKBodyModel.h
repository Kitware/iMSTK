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

#include <memory>

#include "imstkDynamicalModel.h"
#include "imstkVectorizedState.h"
#include "imstkNonLinearSystem.h"
#include "imstkLogger.h"

namespace vega
{
class VolumetricMesh;
class ReducedStVKForceModel;
class StVKReducedInternalForces;
class ModalMatrix;
} // namespace vega

namespace imstk
{
class InternalForceModel;
class SolverBase;
class TaskNode;
class TimeIntegrator;
class VegaMeshIO;

struct ReducedStVKConfig
{
    std::string m_cubicPolynomialFilename;
    std::string m_modesFileName;
    // int         r;

    double m_dampingMassCoefficient      = 0.1;
    double m_dampingStiffnessCoefficient = 0.01;
    double m_dampingLaplacianCoefficient = 0.0;
    double m_deformationCompliance       = 1.0;
    double m_gravity = 9.81;

    int m_numberOfThreads = 4;
};

class ReducedStVK : public DynamicalModel<FeDeformBodyState>
{
using kinematicState = FeDeformBodyState;
// using Matrixd = Eigen::MatrixXd;
public:
    ///
    /// \brief Constructor
    ///
    ReducedStVK();

    ///
    /// \brief Destructor
    ///
    virtual ~ReducedStVK() override;

public:
    ///
    /// \brief Configure the force model from external file
    ///
    void configure(const std::string& configFileName);
    void configure(std::shared_ptr<ReducedStVKConfig> config = std::make_shared<ReducedStVKConfig>());

    ///
    /// \brief Initialize the deformable body model
    ///
    bool initialize() override;

    ///
    /// \brief Set/Get internal force model
    ///
    void setForceModelConfiguration(std::shared_ptr<ReducedStVKConfig> config);
    std::shared_ptr<ReducedStVKConfig> getForceModelConfiguration() const;

    ///
    /// \brief Set/Get time integrator
    ///
    /// \note the return type is different from FEMDeformableBodyModel::setInternalForceModel
    void setInternalForceModel(std::shared_ptr<vega::StVKReducedInternalForces> fm);
    std::shared_ptr<imstk::InternalForceModel> getInternalForceModel() const;

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
    static void initializeEigenMatrixFromStdVector(const std::vector<double>& v,
                                                   Matrixd&                   eigenMatrix);

    ///
    /// \brief Compute the RHS of the resulting linear system
    ///
    void computeImplicitSystemRHS(kinematicState&       prevState,
                                  kinematicState&       newState,
                                  const StateUpdateType updateType);

    ///
    /// \brief Compute the RHS of the resulting linear system using semi-implicit scheme
    ///
    void computeSemiImplicitSystemRHS(kinematicState&       stateAtT,
                                      kinematicState&       newState,
                                      const StateUpdateType updateType);

    ///
    /// \brief Compute the LHS of the resulting linear system
    ///
    void computeImplicitSystemLHS(const kinematicState& prevState,
                                  kinematicState&       newState,
                                  const StateUpdateType updateType);

    ///
    /// \brief Update damping Matrix
    ///
    void updateDampingMatrix();

    ///
    /// \brief Update mass matrix
    /// Note: Not supported yet!
    ///
    void updateMassMatrix();

    ///
    /// \brief Applies boundary conditions to matrix and a vector
    ///
    void applyBoundaryConditions(Matrixd& M, const bool withCompliance = false) const;
    void applyBoundaryConditions(Vectord& x) const;

    ///
    /// \brief Updates the Physics geometry
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Update states
    ///
    void updateBodyStates(const Vectord& solution, const StateUpdateType updateType);
    void updateBodyIntermediateStates(const Vectord& solution, const StateUpdateType updateType);

    ///
    /// \brief Update the previous states given the current state
    ///
    void updateBodyPreviousStates();

    ///
    /// \brief Returns the "function" that evaluates the nonlinear function given
    /// the state vector
    ///
    NonLinearSystem<Matrixd>::VectorFunctionType getFunction();

    ///
    /// \brief Get the function that updates the model given the solution
    ///
    NonLinearSystem<Matrixd>::UpdateFunctionType          getUpdateFunction();
    NonLinearSystem<Matrixd>::UpdatePrevStateFunctionType getUpdatePrevStateFunction();

    ///
    /// \brief Returns the "function" that evaluates the gradient of the nonlinear
    /// function given the state vector
    ///
    NonLinearSystem<Matrixd>::MatrixFunctionType getFunctionGradient();

    ///
    /// \brief Get the contact force vector
    ///
    Vectord& getContactForce();

    ///
    /// \brief Returns the unknown vectors
    ///
    Vectord& getUnknownVec()
    {
        return m_qSol;
    }

    ///
    /// \brief Set/Get the update type
    ///
    void setUpdateType(const StateUpdateType& updateType)
    {
        m_updateType = updateType;
    }

    const StateUpdateType& getUpdateType() const
    {
        return m_updateType;
    }

    /// \brief Returns the unknown vectors
    ///
    std::vector<std::size_t>& getFixNodeIds()
    {
        return m_fixedNodeIds;
    }

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
    void enableFixedBC()
    {
        LOG(WARNING) << "Boundary conditions are not allowed to change";
        m_implementFixedBC = true;
    };
    void disableFixedBC()
    {
        // LOG(FATAL, "Boundary conditions are not allowed to change");
        m_implementFixedBC = false;
    };
    bool isFixedBCImplemented() const
    {
        return m_implementFixedBC;
    };

    ///
    /// \brief prolongate reduced vector into full space, ie, u = U * uReduced
    ///
    void prolongate(const Vectord& uReduced, Vectord& u) const;

    ///
    /// \brief prolongate reduced state into full space
    ///
    void prolongate(kinematicState& uReduced, kinematicState& u) const;

    ///
    /// \brief project full-space vector into reduced space, uReduced = U^T u
    ///
    void project(const Vectord& u, Vectord& uReduced) const;
    ///
    /// \brief Read in the basis file and create m_modalMatrix
    /// \todo: make it private/protected?
    ///
    void readModalMatrix(const std::string& fname);

    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveNode; }

    std::shared_ptr<SolverBase> getSolver() const { return m_solver; }
    void setSolver(std::shared_ptr<SolverBase> solver) { this->m_solver = solver; }

protected:
    ///
    /// \brief Setup the computational graph of FEM
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<SolverBase> m_solver = nullptr;
    std::shared_ptr<vega::StVKReducedInternalForces> m_internalForceModel; ///> Mathematical model for intenal forces
    std::shared_ptr<vega::ReducedStVKForceModel>     m_forceModel;
    std::shared_ptr<TimeIntegrator> m_timeIntegrator;                      ///> Time integrator
    std::shared_ptr<NonLinearSystem<Matrixd>> m_nonLinearSystem;           ///> Nonlinear system resulting from TI and force model
    std::shared_ptr<vega::ModalMatrix> m_modalMatrix;

    std::shared_ptr<ReducedStVKConfig> m_config;

    /// Matrices typical to a elastodynamics and 2nd order analogous systems
    Matrixd m_M;    ///> Mass matrix
    Matrixd m_C;    ///> Damping coefficient matrix
    Matrixd m_K;    ///> Tangent (derivative of internal force w.r.t displacements) stiffness matrix
    Matrixd m_Keff; ///> Effective stiffness matrix (dependent on internal force model and time
    /// integrator)

    // full-space vectors
    Vectord m_Fcontact;          ///> Vector of contact forces
    Vectord m_Fgravity;          ///> Vector of gravity forces
    Vectord m_FexplicitExternal; ///> Vector of explicitly defined external forces
    Vectord m_qSol;              ///> Vector to maintain solution at each iteration of nonlinear solver

    // reduced-space vectors
    Vectord m_Feff;                     ///> Vector of effective forces
    Vectord m_Finternal;                ///> Vector of internal forces
    Vectord m_qSolReduced;              ///> Vector to maintain solution at each iteration of nonlinear solver
    Vectord m_FcontactReduced;
    Vectord m_FgravityReduced;          ///> Vector of gravity forces
    Vectord m_FexplicitExternalReduced; ///> Vector of explicitly defined external forces
    size_t  m_numDOFReduced;

    std::shared_ptr<vega::VolumetricMesh> m_vegaPhysicsMesh;       ///> Mesh used for Physics
    std::vector<double> m_massMatrix;                              ///> Vega mass matrix
    std::vector<double> m_stiffnessMatrix;                         ///> Vega Tangent stiffness matrix
    std::vector<double> m_dampingMatrix;                           ///> Vega Laplacian damping matrix

    std::vector<std::size_t> m_fixedNodeIds;                       ///> Nodal IDs of the nodes that are fixed

    StateUpdateType m_updateType = StateUpdateType::DeltaVelocity; ///> Update type of the model

    bool m_damped = false;                                         ///> Viscous or structurally damped system

    // If this is true, the tangent stiffness and force vector will be modified to
    // accommodate (the rows and columns will be nullified) the fixed boundary conditions
    bool m_implementFixedBC = false;

    std::shared_ptr<kinematicState> m_initialStateReduced;
    std::shared_ptr<kinematicState> m_previousStateReduced;
    std::shared_ptr<kinematicState> m_currentStateReduced;

private:
    std::shared_ptr<TaskNode> m_solveNode = nullptr;
};
}  // namespace imstk
