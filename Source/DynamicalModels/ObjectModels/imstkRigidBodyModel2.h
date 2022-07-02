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
#include "imstkRigidBodyState2.h"

#include <list>
#include <unordered_map>

namespace imstk
{
template<typename ScalarType>
class ProjectedGaussSeidelSolver;
class RbdConstraint;
struct RigidBody;

struct RigidBodyModel2Config
{
    double m_dt     = 0.001; ///< Time step size
    Vec3d m_gravity = Vec3d(0.0, -9.8, 0.0);
    unsigned int m_maxNumIterations = 10;
    double m_velocityDamping = 1.0;
    double m_angularVelocityDamping = 1.0;
    double m_epsilon = 1e-4;
    int m_maxNumConstraints = -1;
};

///
/// \class RigidBodyModel2
///
/// \brief This class implements a constraint based rigid body linear system
/// with pgs solver
///
/// References:
/// David Baraff. 1997. An Introduction to Physically Based Modeling: Rigid Body Simulation II - Nonpenetration Constraints
/// Marijn Tamis and Giuseppe Maggiore. 2015. Constraint based physics solver.
/// David Baraff. 1989. Analyltical Methods for Dynamic Simulation of Non-Penetrating Rigid Bodies. In Computer Graphics (Proc. SIGGRAPH), volume 23, pages 223-232. ACM.
///
class RigidBodyModel2 : public DynamicalModel<RigidBodyState2>
{
public:
    using StorageIndex = Eigen::SparseMatrix<double>::StorageIndex;

    RigidBodyModel2();
    ~RigidBodyModel2() override = default;

    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const double timeStep) override { m_config->m_dt = timeStep; }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_config->m_dt; }

    std::shared_ptr<RigidBodyModel2Config> getConfig() const { return m_config; }
    const std::list<std::shared_ptr<RbdConstraint>>& getConstraints() const { return m_constraints; }
    std::shared_ptr<ProjectedGaussSeidelSolver<double>> getSolver() const { return m_pgsSolver; }

    ///
    /// \brief Adds a body to the system, must call initialize for changes to effect
    /// returns its reference
    ///
    std::shared_ptr<RigidBody> addRigidBody();

    ///
    /// \brief Adds a constraint to be solved
    ///
    void addConstraint(std::shared_ptr<RbdConstraint> constraint) { m_constraints.push_back(constraint); }

    ///
    /// \brief Removes a body from the system, must call initialize for changes to effect
    ///
    void removeRigidBody(std::shared_ptr<RigidBody> body);

    ///
    /// \brief Initialize the RigidBody model to the initial state
    ///
    bool initialize() override;

    ///
    /// \brief Updates mass and inertia matrices to those provided
    /// by the bodies. Not often needed unless mass/inertia is changing
    /// at runtime.
    ///
    void updateMass();

    ///
    /// \brief Configure the model
    ///
    void configure(std::shared_ptr<RigidBodyModel2Config> config);

    ///
    /// \brief Computes the velocities
    ///
    void computeTentativeVelocities();

    ///
    /// \brief Solve the current constraints of the model, then discards
    ///
    void solveConstraints();

    ///
    /// \brief Integrate the model state
    ///
    void integrate();

    std::shared_ptr<TaskNode> getComputeTentativeVelocitiesNode() const { return m_computeTentativeVelocities; }
    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveNode; }
    std::shared_ptr<TaskNode> getIntegrateNode() const { return m_integrateNode; }

protected:
    ///
    /// \brief Setup the task graph of the RigidBodyModel
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<RigidBodyModel2Config> m_config;

    std::shared_ptr<TaskNode> m_computeTentativeVelocities;
    std::shared_ptr<TaskNode> m_solveNode;
    std::shared_ptr<TaskNode> m_integrateNode;

    std::shared_ptr<ProjectedGaussSeidelSolver<double>> m_pgsSolver;
    Eigen::SparseMatrix<double> m_Minv;
    std::list<std::shared_ptr<RbdConstraint>>    m_constraints;
    std::vector<std::shared_ptr<RigidBody>>      m_bodies;
    std::unordered_map<RigidBody*, StorageIndex> m_locations;
    bool   m_modified = true;
    size_t m_maxBodiesParallel = 10; // After 10 bodies, parallel for's are used

    Eigen::VectorXd F;               // Reaction forces
};
} // namespace imstk
