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

#include "imstkRigidBodyModel2.h"
#include "imstkParallelFor.h"
#include "imstkProjectedGaussSeidelSolver.h"
#include "imstkRbdConstraint.h"
#include "imstkTaskGraph.h"
#include "imstkLogger.h"

namespace imstk
{
RigidBodyModel2::RigidBodyModel2() :
    m_config(std::make_shared<RigidBodyModel2Config>()),
    m_pgsSolver(std::make_shared<ProjectedGaussSeidelSolver<double>>())
{
    m_computeTentativeVelocities = std::make_shared<TaskNode>(
        std::bind(&RigidBodyModel2::computeTentativeVelocities, this), "RigidBodyModel_ComputeTentativeVelocities");
    m_solveNode     = std::make_shared<TaskNode>(std::bind(&RigidBodyModel2::solveConstraints, this), "RigidBodyModel_Solve");
    m_integrateNode = std::make_shared<TaskNode>(std::bind(&RigidBodyModel2::integrate, this), "RigidBodyModel_Integrate");

    m_taskGraph->addNode(m_computeTentativeVelocities);
    m_taskGraph->addNode(m_solveNode);
    m_taskGraph->addNode(m_integrateNode);
}

std::shared_ptr<RigidBody>
RigidBodyModel2::addRigidBody()
{
    m_bodies.push_back(std::make_shared<RigidBody>());
    m_modified = true;
    return m_bodies.back();
}

void
RigidBodyModel2::removeRigidBody(std::shared_ptr<RigidBody> rbd)
{
    std::vector<std::shared_ptr<RigidBody>>::iterator bodyIter = std::find(m_bodies.begin(), m_bodies.end(), rbd);
    if (bodyIter != m_bodies.end())
    {
        m_bodies.erase(bodyIter);
    }
    m_modified = true;
}

bool
RigidBodyModel2::initialize()
{
    // Only run initialize if a body has been added/removed
    if (!m_modified)
    {
        return true;
    }

    // Compute the initial state
    std::shared_ptr<RigidBodyState2> state = std::make_shared<RigidBodyState2>();
    state->resize(m_bodies.size());
    std::vector<bool>& isStatic  = state->getIsStatic();
    StdVectorOfReal&   invMasses = state->getInvMasses();
    StdVectorOfMat3d&  invInteriaTensors = state->getInvIntertiaTensors();
    StdVectorOfVec3d&  positions           = state->getPositions();
    StdVectorOfQuatd&  orientations        = state->getOrientations();
    StdVectorOfVec3d&  velocities          = state->getVelocities();
    StdVectorOfVec3d&  angularVelocities   = state->getAngularVelocities();
    StdVectorOfVec3d&  tentativeVelocities = state->getTentatveVelocities();
    StdVectorOfVec3d&  tentativeAngularVelocities = state->getTentativeAngularVelocities();
    StdVectorOfVec3d&  forces  = state->getForces();
    StdVectorOfVec3d&  torques = state->getTorques();

    m_Minv = Eigen::SparseMatrix<double>(m_bodies.size() * 6, m_bodies.size() * 6);
    std::vector<Eigen::Triplet<double>> mInvTriplets;
    mInvTriplets.reserve((9 + 3) * m_bodies.size());
    for (size_t i = 0; i < m_bodies.size(); i++)
    {
        RigidBody& body = *m_bodies[i];

        // Set the intial state
        isStatic[i]  = body.m_isStatic;
        invMasses[i] = (body.m_mass == 0.0) ? 0.0 : 1.0 / body.m_mass;
        if (body.m_intertiaTensor.determinant() == 0.0)
        {
            LOG(WARNING) << "Inertia tensor provided is not invertible, check that it makes sense";
            return false;
        }
        invInteriaTensors[i] = body.m_intertiaTensor.inverse();
        positions[i]           = body.m_initPos;
        orientations[i]        = body.m_initOrientation;
        velocities[i]          = body.m_initVelocity;
        angularVelocities[i]   = body.m_initAngularVelocity;
        tentativeVelocities[i] = body.m_initVelocity;
        tentativeAngularVelocities[i] = body.m_initAngularVelocity;
        forces[i]  = body.m_initForce;
        torques[i] = body.m_initTorque;

        // Link it up with the state
        body.m_pos = &positions[i];
        body.m_orientation     = &orientations[i];
        body.m_velocity        = &velocities[i];
        body.m_angularVelocity = &angularVelocities[i];
        body.m_force  = &forces[i];
        body.m_torque = &torques[i];
        m_locations[m_bodies[i].get()] = static_cast<StorageIndex>(i);

        if (!body.m_isStatic)
        {
            // invMass expanded to 3x3 matrix
            const double invMass     = invMasses[i];
            const Mat3d& invInvertia = invInteriaTensors[i];
            int          index       = static_cast<int>(i * 6);
            mInvTriplets.push_back(Eigen::Triplet<double>(index, index, invMass));
            index++;
            mInvTriplets.push_back(Eigen::Triplet<double>(index, index, invMass));
            index++;
            mInvTriplets.push_back(Eigen::Triplet<double>(index, index, invMass));
            index++;
            for (unsigned int c = 0; c < 3; c++)
            {
                for (unsigned int r = 0; r < 3; r++)
                {
                    mInvTriplets.push_back(Eigen::Triplet<double>(index + r, index + c, invInvertia(c, r)));
                }
            }
        }
    }
    m_Minv.setFromTriplets(mInvTriplets.begin(), mInvTriplets.end());

    // Copy to initial state
    m_initialState  = std::make_shared<RigidBodyState2>(*state);
    m_currentState  = state;
    m_previousState = std::make_shared<RigidBodyState2>(*state);
    m_modified      = false;

    return true;
}

void
RigidBodyModel2::configure(std::shared_ptr<RigidBodyModel2Config> config)
{
    m_config = config;
}

void
RigidBodyModel2::computeTentativeVelocities()
{
    const double            dt = m_config->m_dt;
    const StdVectorOfReal&  invMasses = getCurrentState()->getInvMasses();
    const StdVectorOfMat3d& invInteriaTensors   = getCurrentState()->getInvIntertiaTensors();
    StdVectorOfVec3d&       tentativeVelocities = getCurrentState()->getTentatveVelocities();
    StdVectorOfVec3d&       tentativeAngularVelocities = getCurrentState()->getTentativeAngularVelocities();
    StdVectorOfVec3d&       forces  = getCurrentState()->getForces();
    StdVectorOfVec3d&       torques = getCurrentState()->getTorques();
    const Vec3d&            fG      = m_config->m_gravity;

    // Sum gravity to the forces
    ParallelUtils::parallelFor(forces.size(), [&forces, &fG](const size_t& i)
        {
            forces[i] += fG;
        }, forces.size() > m_maxBodiesParallel);

    // Compute the desired velocites, later we will solve for the proper velocities,
    // adjusted for the constraints
    ParallelUtils::parallelFor(tentativeVelocities.size(), [&](const size_t& i)
        {
            tentativeVelocities[i] += forces[i] * invMasses[i] * dt;
            tentativeAngularVelocities[i] += invInteriaTensors[i] * torques[i] * dt;
        }, tentativeVelocities.size() > m_maxBodiesParallel);
}

void
RigidBodyModel2::solveConstraints()
{
    // Clear
    F = Eigen::VectorXd();

    // Solves the current constraints of the system, then discards them
    if (m_constraints.size() == 0)
    {
        return;
    }
    if (m_config->m_maxNumConstraints != -1 && static_cast<int>(m_constraints.size()) > m_config->m_maxNumConstraints * 2)
    {
        m_constraints.resize(m_config->m_maxNumConstraints * 2);
    }

    //printf("solving\n");

    std::shared_ptr<RigidBodyState2> state    = getCurrentState();
    const std::vector<bool>&         isStatic = state->getIsStatic();
    const StdVectorOfVec3d&          tentativeVelocities = state->getTentatveVelocities();
    const StdVectorOfVec3d&          tentativeAngularVelocities = state->getTentativeAngularVelocities();
    StdVectorOfVec3d&                forces  = state->getForces();
    StdVectorOfVec3d&                torques = state->getTorques();

    Eigen::VectorXd V    = Eigen::VectorXd(state->size() * 6);
    Eigen::VectorXd Fext = Eigen::VectorXd(state->size() * 6);

    // Fill the forces and tenative velocities vectors
    StorageIndex j = 0;
    for (size_t i = 0; i < state->size(); i++)
    {
        if (!isStatic[i])
        {
            const Vec3d& velocity = tentativeVelocities[i];
            const Vec3d& angularVelocity = tentativeAngularVelocities[i];
            const Vec3d& force  = forces[i];
            const Vec3d& torque = torques[i];
            Fext(j) = force[0];
            V(j)    = velocity[0];
            j++;
            Fext(j) = force[1];
            V(j)    = velocity[1];
            j++;
            Fext(j) = force[2];
            V(j)    = velocity[2];
            j++;
            Fext(j) = torque[0];
            V(j)    = angularVelocity[0];
            j++;
            Fext(j) = torque[1];
            V(j)    = angularVelocity[1];
            j++;
            Fext(j) = torque[2];
            V(j)    = angularVelocity[2];
            j++;
        }
        else
        {
            for (StorageIndex k = 0; k < 6; k++)
            {
                Fext(j + k) = 0.0;
                V(j + k)    = 0.0;
            }
            j += 6;
        }
    }

    // Now construct the sparse jacobian matrix for every constraint (object vs constraint)
    Eigen::SparseMatrix<double>         J(m_constraints.size(), state->size() * 6);
    Eigen::VectorXd                     Vu(m_constraints.size());    // Push Factor
    Eigen::MatrixXd                     cu(m_constraints.size(), 2); // Mins and maxes
    std::vector<Eigen::Triplet<double>> JTriplets;
    JTriplets.reserve(m_constraints.size() * 12);
    j = 0;
    for (std::list<std::shared_ptr<RbdConstraint>>::iterator iter = m_constraints.begin(); iter != m_constraints.end(); iter++)
    {
        std::shared_ptr<RbdConstraint> constraint = *iter;
        Vu(j) = constraint->vu;

        // Object 1
        StorageIndex k = 0;
        if (constraint->m_obj1 != nullptr)
        {
            const StorageIndex obj1Location = m_locations[constraint->m_obj1.get()];
            const StorageIndex start1       = obj1Location * 6;
            for (StorageIndex c = 0; c < 2; c++)
            {
                for (StorageIndex r = 0; r < 3; r++)
                {
                    JTriplets.push_back(Eigen::Triplet<double>(j, start1 + k, constraint->J(r, c)));
                    k++;
                }
            }
        }

        // Object 2
        if (constraint->m_obj2 != nullptr)
        {
            const StorageIndex obj2Location = m_locations[constraint->m_obj2.get()];
            const StorageIndex start2       = obj2Location * 6;
            k = 0;
            for (StorageIndex c = 2; c < 4; c++)
            {
                for (StorageIndex r = 0; r < 3; r++)
                {
                    JTriplets.push_back(Eigen::Triplet<double>(j, start2 + k, constraint->J(r, c)));
                    k++;
                }
            }
        }

        cu(j, 0) = constraint->range[0];
        cu(j, 1) = constraint->range[1];
        j++;
    }
    J.setFromTriplets(JTriplets.begin(), JTriplets.end());

    const double                dt = m_config->m_dt;
    Eigen::SparseMatrix<double> A  = J * m_Minv * J.transpose();
    Eigen::VectorXd             b  = Vu / dt - J * (V / dt + m_Minv * Fext);

    /*std::cout << "Minv: " << std::endl << m_Minv.toDense() << std::endl;
    std::cout << "J: " << std::endl << J.toDense() << std::endl;
    std::cout << "A: " << std::endl << A.toDense() << std::endl;
    std::cout << std::endl;
    std::cout << "Vu: " << std::endl << Vu << std::endl;
    std::cout << "V: " << std::endl << V << std::endl;
    std::cout << "b: " << std::endl << b << std::endl;*/

    m_pgsSolver->setA(&A);
    //pgsSolver.setGuess(F); // Not using warm starting
    m_pgsSolver->setMaxIterations(m_config->m_maxNumIterations);
    m_pgsSolver->setEpsilon(m_config->m_epsilon);
    F = J.transpose() * m_pgsSolver->solve(b, cu);   // Reaction force,torque

    // Apply reaction impulse
    j = 0;
    for (size_t i = 0; i < state->size(); i++, j += 6)
    {
        forces[i]  += Vec3d(F(j), F(j + 1), F(j + 2));
        torques[i] += Vec3d(F(j + 3), F(j + 4), F(j + 5));
    }

    //std::cout << std::endl << "Forces/Torques: " << F << std::endl;

    m_constraints.clear();
}

void
RigidBodyModel2::integrate()
{
    // Just a basic symplectic euler
    const double dt = m_config->m_dt;
    const double velocityDamping = m_config->m_velocityDamping;
    const double angularVelocityDamping = m_config->m_angularVelocityDamping;

    const std::vector<bool>& isStatic = getCurrentState()->getIsStatic();

    const StdVectorOfReal&  invMasses = getCurrentState()->getInvMasses();
    const StdVectorOfMat3d& invInteriaTensors = getCurrentState()->getInvIntertiaTensors();

    StdVectorOfVec3d& positions    = getCurrentState()->getPositions();
    StdVectorOfQuatd& orientations = getCurrentState()->getOrientations();

    StdVectorOfVec3d& velocities = getCurrentState()->getVelocities();
    StdVectorOfVec3d& angularVelocities = getCurrentState()->getAngularVelocities();

    StdVectorOfVec3d& tentativeVelocities = getCurrentState()->getTentatveVelocities();
    StdVectorOfVec3d& tentativeAngularVelocities = getCurrentState()->getTentativeAngularVelocities();

    StdVectorOfVec3d& forces  = getCurrentState()->getForces();
    StdVectorOfVec3d& torques = getCurrentState()->getTorques();

    ParallelUtils::parallelFor(positions.size(), [&](const size_t& i)
        {
            if (!isStatic[i])
            {
                velocities[i] += forces[i] * invMasses[i] * dt;
                velocities[i] *= velocityDamping;
                angularVelocities[i] += invInteriaTensors[i] * torques[i] * dt;
                angularVelocities[i] *= angularVelocityDamping;
                positions[i] += velocities[i] * dt;
                {
                    const Quatd q = Quatd(0.0,
                            angularVelocities[i][0],
                            angularVelocities[i][1],
                            angularVelocities[i][2]) * orientations[i];
                    orientations[i].x() += q.x() * dt;
                    orientations[i].y() += q.y() * dt;
                    orientations[i].z() += q.z() * dt;
                    orientations[i].w() += q.w() * dt;
                    orientations[i].normalize();
                }
            }

            // Reset
            m_bodies[i]->m_prevForce = forces[i];
            forces[i]  = Vec3d(0.0, 0.0, 0.0);
            torques[i] = Vec3d(0.0, 0.0, 0.0);
            tentativeVelocities[i] = velocities[i];
            tentativeAngularVelocities[i] = angularVelocities[i];
        }, positions.size() > m_maxBodiesParallel);
}

void
RigidBodyModel2::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, m_computeTentativeVelocities);
    m_taskGraph->addEdge(m_computeTentativeVelocities, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, m_integrateNode);
    m_taskGraph->addEdge(m_integrateNode, sink);
}
}