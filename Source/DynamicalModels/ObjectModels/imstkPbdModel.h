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

#include "imstkDynamicalModel.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkPbdState.h"
#include "imstkLogger.h"

namespace imstk
{
class PointSet;
class PbdSolver;

///
/// \struct PBDModelConfig
/// \brief Parameters for PBD simulation
///
struct PBDModelConfig
{
    double m_uniformMassValue    = 1.0;  ///> Mass properties
    double m_viscousDampingCoeff = 0.01; ///> Viscous damping coefficient [0, 1]

    std::shared_ptr<PbdCollisionConstraintConfig> collisionParams =
        std::make_shared<PbdCollisionConstraintConfig>(PbdCollisionConstraintConfig
        {
            0.1,                             // Proximity
            1.0                              // Contact stiffness
        });                                  ///> Info shared between the collision constraints

    unsigned int m_iterations = 10;          ///> Pbd iterations
    unsigned int m_collisionIterations = 5;  ///> Pbd collision iterations
    double m_dt = 0.0;                       ///> Time step size
    double m_defaultDt = 0.01;               ///> Default Time step size

    std::vector<std::size_t> m_fixedNodeIds; ///> Nodal IDs of the nodes that are fixed
    Vec3r m_gravity = Vec3r(0, -9.81, 0);    ///> Gravity

    std::shared_ptr<PbdFEMConstraintConfig> m_femParams =
        std::make_shared<PbdFEMConstraintConfig>(PbdFEMConstraintConfig
        {
            0.0,                                                                                  // Lame constant, if constraint type is FEM
            0.0,                                                                                  // Lame constant, if constraint type is FEM
            1000.0,                                                                               // FEM parameter, if constraint type is FEM
            0.2                                                                                   // FEM parameter, if constraint type is FEM
        });                                                                                       ///> Info shared between the fem constraints

    std::vector<std::pair<PbdConstraint::Type, double>> m_regularConstraints;                     ///> Constraints except FEM
    std::vector<std::pair<PbdConstraint::Type, PbdFEMConstraint::MaterialType>> m_FEMConstraints; ///> Constraints except FEM

    ///
    /// \brief Enable a regular constraint (constraint that is not FEM constraint)
    /// with a given constraint stiffness
    ///
    void enableConstraint(PbdConstraint::Type type, double stiffness);

    ///
    /// \brief Enable a FEM constraint with mu, lambda
    ///
    void enableFEMConstraint(PbdConstraint::Type type, PbdFEMConstraint::MaterialType material);

    ///
    /// \brief Set the PBD solver type
    ///
    void setSolverType(const PbdConstraint::SolverType& type)
    {
        if (type == PbdConstraint::SolverType::GCD)
        {
            LOG(WARNING) << "GCD is NOT implemented yet, use xPBD instead";
            m_solverType = PbdConstraint::SolverType::xPBD;
            return;
        }

        m_solverType = type;
    }

    PbdConstraint::SolverType m_solverType = PbdConstraint::SolverType::xPBD;
};

///
/// \class PbdModel
///
/// \brief This class implements position based dynamics mathematical model
///
class PbdModel : public DynamicalModel<PbdState>
{
public:
    ///
    /// \brief Constructor
    ///
    PbdModel();

    ///
    /// \brief Destructor
    ///
    virtual ~PbdModel() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(std::shared_ptr<PBDModelConfig> params);

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<PBDModelConfig>& getParameters() const { assert(m_parameters); return m_parameters; }

    ///
    /// \brief Compute elastic constants: Young Modulus, Poisson Ratio, first and second Lame
    /// \brief If both Young Modulus or Poisson Ratio are zero, then compute them from the Lame coefficients
    /// \brief And vice versas, if both Lame coefficients are zero, compute them from Young Modulus and Poisson Ratio
    ///
    void computeElasticConstants();

    ///
    /// \brief Initialize FEM constraints
    ///
    bool initializeFEMConstraints(PbdFEMConstraint::MaterialType type);

    ///
    /// \brief Initialize volume constraints
    ///
    bool initializeVolumeConstraints(const double stiffness);

    ///
    /// \brief Initialize distance constraints
    ///
    bool initializeDistanceConstraints(const double stiffness);

    ///
    /// \brief Initialize area constraints
    ///
    bool initializeAreaConstraints(const double stiffness);

    ///
    /// \brief Initialize bend constraints
    ///
    bool initializeBendConstraints(const double stiffness);

    ///
    /// \brief Initialize dihedral constraints
    ///
    bool initializeDihedralConstraints(const double stiffness);

    ///
    /// \brief Initialize constant density constraints for PBD fluid
    ///
    bool initializeConstantDensityConstraint(const double stiffness);

    ///
    /// \brief Update the model geometry from the newest PBD state
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Update the PBD state from the model geometry
    ///
    void updatePbdStateFromPhysicsGeometry();

    ///
    /// \brief Returns true if there is at least one constraint
    ///
    bool hasConstraints() const { return !m_constraints->empty() || !m_partitionedConstraints->empty(); }

    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const Real timeStep) override { m_parameters->m_dt = timeStep; }
    void setDefaultTimeStep(const Real timeStep) { m_parameters->m_defaultDt = static_cast<Real>(timeStep); }

    ///
    /// \brief Set the time step size to fixed size
    ///
    virtual void setTimeStepSizeType(const TimeSteppingType type) override;

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_parameters->m_dt; }
    double getDefaultTimeStep() const { return m_parameters->m_defaultDt; }

    ///
    /// \brief Return all constraints that are solved sequentially
    ///
    std::shared_ptr<PBDConstraintVector> getConstraints() { return m_constraints; }

    ///
    /// \brief Return the constraints that are colored and run in parallel
    ///
    std::shared_ptr<std::vector<PBDConstraintVector>> getPartitionedConstraints() { return m_partitionedConstraints; }

    ///
    /// \brief Set uniform mass to all the nodes
    ///
    void setUniformMass(const double val);

    ///
    /// \brief Set mass to particular node
    ///
    void setParticleMass(const double val, const size_t idx);

    ///
    /// \brief Se the node as fixed
    ///
    void setFixedPoint(const size_t idx);

    ///
    /// \brief Get the inverse of mass of a certain node
    ///
    double getInvMass(const size_t idx) const { return (*m_invMass)[idx]; }

    std::shared_ptr<StdVectorOfReal> getInvMasses() { return m_invMass; }

    ///
    /// \brief Time integrate the position
    ///
    void integratePosition();

    ///
    /// \brief Time integrate the velocity
    ///
    void updateVelocity();

    ///
    /// \brief Update body states given the newest update and the type of update
    ///
    virtual void updateBodyStates(const Vectord& /*q*/,
                                  const StateUpdateType /*updateType = stateUpdateType::displacement*/) override {}

    ///
    /// \brief Initialize the PBD model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Set the threshold for constraint partitioning
    ///
    void setConstraintPartitionThreshold(size_t threshold) { m_partitionThreshold = threshold; }

    std::shared_ptr<PbdSolver> getSolver() const { return m_pbdSolver; }

    void setSolver(std::shared_ptr<PbdSolver> solver) { this->m_pbdSolver = solver; }

    std::shared_ptr<TaskNode> getIntegratePositionNode() const { return m_integrationPositionNode; }

    std::shared_ptr<TaskNode> getUpdateCollisionGeometryNode() const { return m_updateCollisionGeometryNode; }

    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveConstraintsNode; }

    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNode; }

protected:
    ///
    /// \brief Partition constraints for parallelization
    ///
    void partitionConstraints(const bool print = false);

    ///
    /// \brief Setup the computational graph of PBD
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    bool   m_partitioned = false;                                                         /// \todo this is used in initialize() as a temporary fix to problems on linux
    size_t m_partitionThreshold = 16;                                                     ///> Threshold for constraint partitioning

    std::shared_ptr<PbdSolver>       m_pbdSolver = nullptr;                               ///> PBD solver
    std::shared_ptr<PointSet>        m_mesh      = nullptr;                               ///> PointSet on which the pbd model operates on
    std::shared_ptr<StdVectorOfReal> m_mass      = nullptr;                               ///> Mass of nodes
    std::shared_ptr<StdVectorOfReal> m_invMass   = nullptr;                               ///> Inverse of mass of nodes

    std::shared_ptr<PBDConstraintVector> m_constraints = nullptr;                         ///> List of pbd constraints
    std::shared_ptr<std::vector<PBDConstraintVector>> m_partitionedConstraints = nullptr; ///> List of pbd constraints
    std::shared_ptr<PBDModelConfig> m_parameters = nullptr;                               ///> Model parameters, must be set before simulation

protected:
    // Computational Nodes
    std::shared_ptr<TaskNode> m_integrationPositionNode     = nullptr;
    std::shared_ptr<TaskNode> m_updateCollisionGeometryNode = nullptr;
    std::shared_ptr<TaskNode> m_solveConstraintsNode = nullptr;
    std::shared_ptr<TaskNode> m_updateVelocityNode   = nullptr;
};
} // imstk
