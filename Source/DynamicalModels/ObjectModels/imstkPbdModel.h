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

#include <unordered_map>
#include <unordered_set>

namespace imstk
{
struct PbdConstraintFunctor;
class PointSet;
class PbdConstraintContainer;
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
    double m_dt = 0.0;                       ///> Time step size

    bool m_doPartitioning = true;

    std::vector<std::size_t> m_fixedNodeIds; ///> Nodal IDs of the nodes that are fixed
    Vec3d m_gravity = Vec3d(0, -9.81, 0);    ///> Gravity

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
    void setSolverType(const PbdConstraint::SolverType& type);

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
    /// \brief Add constraints related to a set of vertices.
    /// \brief Does not check for duplicating pre-existed constraints.
    /// \todo: Move to containers and functors
    ///
    void addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices);

    virtual void setTimeStep(const double timeStep) override { m_parameters->m_dt = timeStep; }
    double getTimeStep() const override { return m_parameters->m_dt; }

    ///
    /// \brief Return all constraints that are solved sequentially
    ///
    std::shared_ptr<PbdConstraintContainer> getConstraints() { return m_constraints; }

    ///
    /// \brief Set mass to particular node
    ///
    void setParticleMass(const double val, const size_t idx);

    ///
    /// \brief Set the node as fixed
    ///
    void setFixedPoint(const size_t idx);

    ///
    /// \brief Set the node as unfixed
    ///
    void setPointUnfixed(const size_t idx);

    ///
    /// \brief Get the inverse of mass from a certain node
    ///
    double getInvMass(const size_t idx) const { return (*m_invMass)[idx]; }

    ///
    /// \brief Get the inverse masses
    ///
    std::shared_ptr<DataArray<double>> getInvMasses() { return m_invMass; }

    ///
    /// \brief Time integrate the position
    ///
    void integratePosition();

    ///
    /// \brief Time integrate the velocity
    ///
    void updateVelocity();

    ///
    /// \brief Initialize the PBD model
    ///
    virtual bool initialize() override;

    ///
    /// \brief Initialize the PBD State
    ///
    void initState();

    ///
    /// \brief Set the threshold for constraint partitioning
    ///
    void setConstraintPartitionThreshold(size_t threshold) { m_partitionThreshold = threshold; }

    ///
    /// \brief Returns the solver used for internal constraints
    ///
    std::shared_ptr<PbdSolver> getSolver() const { return m_pbdSolver; }

    ///
    /// \brief Sets the solver used for internal constraints
    ///
    void setSolver(std::shared_ptr<PbdSolver> solver) { this->m_pbdSolver = solver; }

    ///
    /// \brief Adds a functor to generate constraints
    ///
    void addPbdConstraintFunctor(std::shared_ptr<PbdConstraintFunctor> functor) { m_functors.push_back(functor); }

    std::shared_ptr<TaskNode> getIntegratePositionNode() const { return m_integrationPositionNode; }

    std::shared_ptr<TaskNode> getUpdateCollisionGeometryNode() const { return m_updateCollisionGeometryNode; }

    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveConstraintsNode; }

    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNode; }

protected:
    ///
    /// \brief Setup the computational graph of PBD
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    size_t m_partitionThreshold = 16;                                                     ///> Threshold for constraint partitioning

    std::shared_ptr<PbdSolver> m_pbdSolver       = nullptr;                               ///> PBD solver
    std::shared_ptr<PointSet>  m_mesh            = nullptr;                               ///> PointSet on which the pbd model operates on
    std::shared_ptr<DataArray<double>> m_mass    = nullptr;                               ///> Mass of nodes
    std::shared_ptr<DataArray<double>> m_invMass = nullptr;                               ///> Inverse of mass of nodes
    std::shared_ptr<std::unordered_map<size_t, double>> m_fixedNodeInvMass = nullptr;     ///> Map for archiving fixed nodes' mass.

    std::shared_ptr<PBDModelConfig> m_parameters = nullptr;                               ///> Model parameters, must be set before simulation

protected:
    std::shared_ptr<PbdConstraintContainer> m_constraints;         ///> The set of constraints to update/use
    std::vector<std::shared_ptr<PbdConstraintFunctor>> m_functors; ///> The set of functors for constraint generation

protected:
    // Computational Nodes
    std::shared_ptr<TaskNode> m_integrationPositionNode     = nullptr;
    std::shared_ptr<TaskNode> m_updateCollisionGeometryNode = nullptr;
    std::shared_ptr<TaskNode> m_solveConstraintsNode = nullptr;
    std::shared_ptr<TaskNode> m_updateVelocityNode   = nullptr;
};
} // imstk
