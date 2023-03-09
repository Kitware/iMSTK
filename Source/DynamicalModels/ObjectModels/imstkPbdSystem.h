/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractDynamicalSystem.h"
#include "imstkPbdBody.h"
#include "imstkPbdConstraint.h"

#include <unordered_map>
#include <unordered_set>

namespace imstk
{
class PbdConstraintContainer;
class PbdModelConfig;
class PbdSolver;

///
/// \class PbdSystem
///
/// \brief This class implements the position based dynamics model. The
/// PbdSystem is a constraint based model that iteratively solves constraints
/// to simulate the dynamics of a body.
///
/// PbdSystem supports multiple bodies which may either be rigid, deformable
/// or fluid. While a rigid body only consists of one node, the geometry of
/// deformable bodies may be defined through Line, Surface or Tetrahedral
/// Meshes. Fluid meshes require a Pointset for their geometry.
///
/// One of the distinct properties of the PbdSystem is that it is first order.
/// This means it simulates dynamics by modifying positions directly.
/// Velocities of the model are computed after positions are solved. Velocities
/// from the previous iteration are applied at the start of the update.
///
/// References:
/// [1] Matthias Muller, Bruno Heidelberger, Marcus Hennix, and John Ratcliff.
/// 2007. Position based dynamics.
///
/// [2] Miles Macklin, Matthias Muller, and Nuttapong Chentanez 1.
/// XPBD: position-based simulation of compliant constrained dynamics.
///
/// [3] Matthias Mullerm, Miles Macklin, Nuttapong Chentanez, Stefan Jeschke,
/// and Tae-Yong Kim. 2020.
///
/// [4] Detailed Rigid Body Simulation with Extended Position Based Dynamics
/// Jan Bender, Matthias Muller, Miles Macklin.
/// A Survey on Position Based Dynamics, 2017.
///
class PbdSystem : public AbstractDynamicalSystem
{
public:
    using AbstractDynamicalSystem::initGraphEdges;

    PbdSystem();
    ~PbdSystem() override = default;

    void resetToInitialState() override;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(std::shared_ptr<PbdModelConfig> params);

    ///
    /// \brief Add/remove PbdBody
    /// @{
    std::shared_ptr<PbdBody> addBody();
    void removeBody(std::shared_ptr<PbdBody> body);
    /// @}

    PbdState& getBodies() { return m_state; }

    ///
    /// \brief Add a particle to a virtual pool/buffer of particles for quick removal/insertion
    /// The persist flag indicates if it should be cleared at the end of the frame or not
    ///
    PbdParticleId addVirtualParticle(
        const Vec3d& pos, const Quatd& orientation,
        const double mass, const Mat3d inertia,
        const Vec3d& velocity = Vec3d::Zero(), const Vec3d& angularVelocity = Vec3d::Zero(),
        const bool persist    = false);

    ///
    /// \brief Add a particle to a virtual pool/buffer of particles for quick removal/insertion
    /// The persist flag indicates if it should be cleared at the end of the frame or not
    ///
    PbdParticleId addVirtualParticle(
        const Vec3d& pos, const double mass,
        const Vec3d& velocity = Vec3d::Zero(),
        const bool persist    = false);

    ///
    /// \brief Resize 0 the virtual particles
    ///
    void clearVirtualParticles();

    ///
    /// \brief Get the simulation parameters
    ///
    std::shared_ptr<PbdModelConfig> getConfig() const;

    ///
    /// \brief Add/generate constraints for given set of vertices on the body, useful for
    /// topology changes
    /// \brief Does not check for duplicating pre-existed constraints.
    /// \todo: Move to containers and functors
    ///
    void addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices, const int bodyId);

    void setTimeStep(const double timeStep) override;
    double getTimeStep() const override;

    ///
    /// \brief Return all constraints that are solved sequentially
    ///
    std::shared_ptr<PbdConstraintContainer> getConstraints() { return m_constraints; }

    ///
    /// \brief Time integrate the position
    ///@{
    void integratePosition();
    void integratePosition(PbdBody& body);
    ///@}

    ///
    /// \brief Time integrate the velocity
    ///@{
    void updateVelocity();
    void updateVelocity(PbdBody& body);
    ///@}

    ///
    /// \brief Solve the internal constraints
    ///
    void solveConstraints();

    ///
    /// \brief Initialize the PBD model
    ///
    bool initialize() override;

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

    std::shared_ptr<TaskNode> getIntegratePositionNode() const { return m_integrationPositionNode; }
    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveConstraintsNode; }
    std::shared_ptr<TaskNode> getUpdateVelocityNode() const { return m_updateVelocityNode; }

protected:
    // Hide this function as PbdSystem doesn't require it. It can support multiple bodies
    using AbstractDynamicalSystem::setModelGeometry;

    ///
    /// \brief Resize the amount of particles for a body
    ///
    void resizeBodyParticles(PbdBody& body, const int particleCount);

    ///
    /// \brief Setup the computational graph of Pbd
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    size_t m_partitionThreshold = 16; ///< Threshold for constraint partitioning

    bool m_modified = true;
    int  m_iterKey  = 0;     ///< Iterative key used for body ids

    PbdState m_initialState;
    PbdState m_state;

    std::shared_ptr<PbdSolver>      m_pbdSolver;           ///< PBD solver
    std::shared_ptr<PbdModelConfig> m_config;              ///< Model parameters, must be set before simulation
    std::shared_ptr<PbdConstraintContainer> m_constraints; ///< The set of constraints to update/use

    ///< Computational Nodes
    ///@{
    std::shared_ptr<TaskNode> m_integrationPositionNode;
    std::shared_ptr<TaskNode> m_solveConstraintsNode;
    std::shared_ptr<TaskNode> m_updateVelocityNode;
    ///@}
};
} // namespace imstk