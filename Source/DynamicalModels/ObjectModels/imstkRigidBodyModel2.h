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

namespace expiremental
{
class RbdConstraint;
struct RigidBody;

struct RigidBodyModel2Config
{
    double m_dt = 0.001; ///> Time step size
    Vec3d m_gravity = Vec3d(0.0, -9.8, 0.0);
    unsigned int m_maxNumIterations = 10;
};

///
/// \class RigidBodyModel2
///
/// \brief This class implements a constraint based rigid body linear system
/// with pgs solver
///
class RigidBodyModel2 : public DynamicalModel<RigidBodyState2>
{
public:
    using StorageIndex = Eigen::SparseMatrix<double>::StorageIndex;

public:
    ///
    /// \brief Constructor
    ///
    RigidBodyModel2();

    ///
    /// \brief Destructor
    ///
    virtual ~RigidBodyModel2() override = default;

public:
    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const Real timeStep) override { m_config->m_dt = timeStep; }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_config->m_dt; }

    std::shared_ptr<RigidBodyModel2Config> getConfig() const { return m_config; }

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
    /// \todo: remove
    ///
    virtual void updateBodyStates(const Vectord& /*q*/, const StateUpdateType /*updateType = stateUpdateType::displacement*/) override {}

    ///
    /// \brief Initialize the RigidBody model
    ///
    bool initialize() override;

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
    /// \brief Setup the task graph of the LSM
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<RigidBodyModel2Config> m_config;

    std::shared_ptr<TaskNode> m_computeTentativeVelocities;
    std::shared_ptr<TaskNode> m_solveNode;
    std::shared_ptr<TaskNode> m_integrateNode;

protected:
    std::shared_ptr<ProjectedGaussSeidelSolver<double>> m_pgsSolver;
    Eigen::SparseMatrix<double> m_Minv;
    std::list<std::shared_ptr<RbdConstraint>>    m_constraints;
    std::vector<std::shared_ptr<RigidBody>>      m_bodies;
    std::unordered_map<RigidBody*, StorageIndex> m_locations;
    bool m_modified = true;
};
}
}