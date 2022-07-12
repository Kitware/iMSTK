/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdModel.h"
#include "imstkGraph.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdModel::PbdModel() : AbstractDynamicalModel(DynamicalModelType::PositionBasedDynamics),
    m_config(std::make_shared<PbdModelConfig>())
{
    // Add a dummy pbdy body for virtual particle addition
    addBody();

    m_validGeometryTypes = {
        "PointSet",
        "LineMesh",
        "SurfaceMesh",
        "TetrahedralMesh",
        "HexahedralMesh"
    };

    // Setup PBD compute nodes
    m_integrationPositionNode = m_taskGraph->addFunction("PbdModel_IntegratePosition",
        [&]() { integratePosition(); });
    m_solveConstraintsNode = m_taskGraph->addFunction("PbdModel_SolveConstraints",
        [&]() { solveConstraints(); });
    m_collisionSolveConstraintsNode = m_taskGraph->addFunction("PbdModel_SolveCollisionConstraints",
        [&]() { solveCollisionConstraints(); });
    m_updateVelocityNode = m_taskGraph->addFunction("PbdModel_UpdateVelocity",
        [&]() { updateVelocity(); });
}

void
PbdModel::resetToInitialState()
{
    m_state.deepCopy(m_initialState);
}

void
PbdModel::configure(std::shared_ptr<PbdModelConfig> config)
{
    m_config = config;
}

std::shared_ptr<PbdBody>
PbdModel::addBody()
{
    m_state.m_bodies.push_back(std::make_shared<PbdBody>(m_iterKey));
    m_modified = true;
    m_iterKey++;
    return m_state.m_bodies.back();
}

void
PbdModel::removeBody(std::shared_ptr<PbdBody> body)
{
    auto iter = std::find(m_state.m_bodies.begin(), m_state.m_bodies.end(), body);
    CHECK(iter != m_state.m_bodies.end()) << "removeBody called but could not find PbdyBody in PbdState";
    m_state.m_bodies.erase(iter);
    m_modified = true;
}

PbdParticleId
PbdModel::addVirtualParticle(
    const Vec3d& pos, const Quatd& orientation,
    const double mass, const Mat3d inertia,
    const Vec3d& velocity, const Vec3d& angularVelocity)
{
    m_state.m_bodies[0]->vertices->push_back(pos);
    m_state.m_bodies[0]->orientations->push_back(orientation);
    m_state.m_bodies[0]->velocities->push_back(velocity);
    m_state.m_bodies[0]->angularVelocities->push_back(angularVelocity);
    m_state.m_bodies[0]->masses->push_back(mass);
    m_state.m_bodies[0]->invMasses->push_back((mass == 0.0) ? 0.0 : 1.0 / mass);
    m_state.m_bodies[0]->inertias->push_back(inertia);
    Mat3d invInertia = Mat3d::Zero();
    if (inertia.determinant() != 0.0)
    {
        invInertia = inertia.inverse();
    }
    m_state.m_bodies[0]->invInertias->push_back(invInertia);
    return { 0, m_state.m_bodies[0]->vertices->size() - 1 };
}

PbdParticleId
PbdModel::addVirtualParticle(
    const Vec3d& pos, const double mass,
    const Vec3d& velocity)
{
    return addVirtualParticle(pos, Quatd::Identity(),
        mass, Mat3d::Identity(),
        velocity, Vec3d::Zero());
}

void
PbdModel::clearVirtualParticles()
{
    CHECK(m_state.m_bodies.size() != 0 || m_state.m_bodies[0] != nullptr) << "Missing virtual/dummy body";
    resizeBodyParticles(*m_state.m_bodies[0], 0);
}

std::shared_ptr<PbdModelConfig>
PbdModel::getConfig() const
{
    CHECK(m_config != nullptr) << "Cannot PbdModel::getConfig, config is nullptr";
    return m_config;
}

bool
PbdModel::initialize()
{
    // Clear the dummy body for virtual particles, init with space for 10 particles
    m_state.m_bodies[0] = std::make_shared<PbdBody>(0);
    m_state.m_bodies[0]->bodyType          = PbdBody::Type::DEFORMABLE_ORIENTED;
    m_state.m_bodies[0]->prevVertices      = std::make_shared<VecDataArray<double, 3>>();
    m_state.m_bodies[0]->vertices          = std::make_shared<VecDataArray<double, 3>>();
    m_state.m_bodies[0]->prevOrientations  = std::make_shared<StdVectorOfQuatd>();
    m_state.m_bodies[0]->orientations      = std::make_shared<StdVectorOfQuatd>();
    m_state.m_bodies[0]->velocities        = std::make_shared<VecDataArray<double, 3>>();
    m_state.m_bodies[0]->angularVelocities = std::make_shared<VecDataArray<double, 3>>();
    m_state.m_bodies[0]->masses      = std::make_shared<DataArray<double>>();
    m_state.m_bodies[0]->invMasses   = std::make_shared<DataArray<double>>();
    m_state.m_bodies[0]->inertias    = std::make_shared<StdVectorOfMat3d>();
    m_state.m_bodies[0]->invInertias = std::make_shared<StdVectorOfMat3d>();

    // Store a copy of the initial state
    m_initialState.deepCopy(m_state);

    // Initialize constraints
    {
        m_constraints = std::make_shared<PbdConstraintContainer>();

        m_config->computeElasticConstants();

        // Run all the functors to generate the constraints
        for (const auto& functorVec : m_config->m_functors)
        {
            for (const auto& functor : functorVec.second)
            {
                (*functor)(*m_constraints);
            }
        }

        // Partition constraints for parallel computation
        if (m_config->m_doPartitioning)
        {
            m_constraints->partitionConstraints(static_cast<int>(m_partitionThreshold));
        }
        else
        {
            m_constraints->clearPartitions();
        }
    }

    // Setup the default pbd solver if none exists
    if (m_pbdSolver == nullptr)
    {
        m_pbdSolver = std::make_shared<PbdSolver>();
    }
    if (m_pbdCollisionSolver == nullptr)
    {
        m_pbdCollisionSolver = std::make_shared<PbdSolver>();
    }

    return true;
}

void
PbdModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_integrationPositionNode);
    m_taskGraph->addEdge(m_integrationPositionNode, m_solveConstraintsNode);
    m_taskGraph->addEdge(m_solveConstraintsNode, m_collisionSolveConstraintsNode);
    m_taskGraph->addEdge(m_collisionSolveConstraintsNode, m_updateVelocityNode);
    m_taskGraph->addEdge(m_updateVelocityNode, sink);
}

void
PbdModel::addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices, const int bodyId)
{
    for (const auto& functorVec : m_config->m_functors)
    {
        for (const auto& functor : functorVec.second)
        {
            if (auto bodyFunctor = std::dynamic_pointer_cast<PbdBodyConstraintFunctor>(functor))
            {
                if (bodyFunctor->m_bodyIndex == bodyId)
                {
                    bodyFunctor->addConstraints(*m_constraints, vertices);
                }
            }
        }
    }
}

void
PbdModel::setTimeStep(const double timeStep)
{
    m_config->m_dt = timeStep;
}

double
PbdModel::getTimeStep() const
{
    return m_config->m_dt;
}

void
PbdModel::integratePosition()
{
    // resize 0 virtual particles (avoids reallocation)
    clearVirtualParticles();

    //for (const auto& body : m_state.m_bodies)
    for (auto bodyIter = std::next(m_state.m_bodies.begin());
         bodyIter != m_state.m_bodies.end(); bodyIter++)
    {
        integratePosition(**bodyIter);
    }
}

void
PbdModel::integratePosition(const PbdBody& body)
{
    VecDataArray<double, 3>& pos       = *body.vertices;
    VecDataArray<double, 3>& prevPos   = *body.prevVertices;
    VecDataArray<double, 3>& vel       = *body.velocities;
    const DataArray<double>& invMasses = *body.invMasses;
    const Vec3d              bodyExternalForce  = body.externalForce;
    const Vec3d              bodyExternalTorque = body.externalTorque;

    // Check all the arrays are the same
    const int numParticles = pos.size();
    CHECK(numParticles == prevPos.size()) << "PbdModel data corrupt";
    CHECK(numParticles == vel.size()) << "PbdModel data corrupt";
    CHECK(numParticles == invMasses.size()) << "PbdModel data corrupt";

    const double dt = m_config->m_dt;
    const double linearVelocityDamp = 1.0 - m_config->getLinearDamping(body.bodyHandle);
    ParallelUtils::parallelFor(numParticles,
        [&](const int i)
        {
            if (std::abs(invMasses[i]) > 0.0)
            {
                const Vec3d accel = m_config->m_gravity + bodyExternalForce * invMasses[i];
                vel[i]    += accel * dt;
                prevPos[i] = pos[i];
                pos[i]    += linearVelocityDamp * vel[i] * dt;
            }
        }, numParticles > 50); // Only run parallel when more than 50 pts

    // If using oriented particles update those too
    if (body.getOriented())
    {
        StdVectorOfQuatd&        orientations      = *body.orientations;
        StdVectorOfQuatd&        prevOrientations  = *body.prevOrientations;
        VecDataArray<double, 3>& angularVelocities = *body.angularVelocities;
        const StdVectorOfMat3d&  inertias    = *body.inertias;
        const StdVectorOfMat3d&  invInertias = *body.invInertias;

        // Check all the arrays are the same
        CHECK(numParticles == orientations.size()) << "PbdModel data corrupt";
        CHECK(numParticles == prevOrientations.size()) << "PbdModel data corrupt";
        CHECK(numParticles == angularVelocities.size()) << "PbdModel data corrupt";
        CHECK(numParticles == invInertias.size()) << "PbdModel data corrupt";

        const double angularVelocityDamp = 1.0 - m_config->getAngularDamping(body.bodyHandle);
        ParallelUtils::parallelFor(numParticles,
            [&](const int i)
            {
                if (!invInertias[i].isZero())
                {
                    Vec3d& w = angularVelocities[i];
                    const Vec3d accel = invInertias[i] *
                                        (bodyExternalTorque - (w.cross(inertias[i] * w)));
                    w += dt * accel;
                    prevOrientations[i] = orientations[i];

                    const Vec3d w1 = angularVelocities[i] * angularVelocityDamp * 0.5;
                    const Quatd dq = Quatd(0.0, w1[0], w1[1], w1[2]) * orientations[i];
                    orientations[i].coeffs() += dq.coeffs() * dt;
                    orientations[i].normalize();
                }
            }, numParticles > 50); // Only run parallel when more than 50 pts
    }
}

void
PbdModel::updateVelocity()
{
    for (auto bodyIter = std::next(m_state.m_bodies.begin());
         bodyIter != m_state.m_bodies.end(); bodyIter++)
    {
        updateVelocity(**bodyIter);
    }

    // Correctly velocities for friction and restitution
    // Unfortunately the constraint would be clear after a solve
    for (const auto& colConstraintList : m_pbdCollisionSolver->getConstraintLists())
    {
        for (auto& colConstraint : *colConstraintList)
        {
            colConstraint->correctVelocity(m_state, m_config->m_dt);
        }
    }
    m_pbdCollisionSolver->clearConstraintLists();
}

void
PbdModel::updateVelocity(const PbdBody& body)
{
    if (m_config->m_dt > 0.0)
    {
        const VecDataArray<double, 3>& pos       = *body.vertices;
        const VecDataArray<double, 3>& prevPos   = *body.prevVertices;
        VecDataArray<double, 3>&       vel       = *body.velocities;
        const DataArray<double>&       invMasses = *body.invMasses;

        // Check all the arrays are the same
        const int numParticles = pos.size();
        CHECK(numParticles == prevPos.size()) << "PbdModel data corrupt";
        CHECK(numParticles == vel.size()) << "PbdModel data corrupt";
        CHECK(numParticles == invMasses.size()) << "PbdModel data corrupt";

        const double invDt = 1.0 / m_config->m_dt;
        ParallelUtils::parallelFor(numParticles,
            [&](const int i)
            {
                if (std::abs(invMasses[i]) > 0.0)
                {
                    vel[i] = (pos[i] - prevPos[i]) * invDt;
                }
            }, numParticles > 50);

        if (body.getOriented())
        {
            const StdVectorOfQuatd&  orientations      = *body.orientations;
            const StdVectorOfQuatd&  prevOrientations  = *body.prevOrientations;
            VecDataArray<double, 3>& angularVelocities = *body.angularVelocities;
            const StdVectorOfMat3d&  invInertias       = *body.invInertias;

            // Check all the arrays are the same
            CHECK(numParticles == orientations.size()) << "PbdModel data corrupt";
            CHECK(numParticles == prevOrientations.size()) << "PbdModel data corrupt";
            CHECK(numParticles == angularVelocities.size()) << "PbdModel data corrupt";
            CHECK(numParticles == invInertias.size()) << "PbdModel data corrupt";

            ParallelUtils::parallelFor(numParticles,
                [&](const int i)
                {
                    if (!invInertias[i].isZero())
                    {
                        const Quatd dq = orientations[i] * prevOrientations[i].inverse();
                        //const Quatd dq = prevOrientations[i] * orientations[i].inverse();
                        const Vec3d angularVel = 2.0 * Vec3d(dq.x(), dq.y(), dq.z()) * invDt;
                        angularVelocities[i]   = dq.w() >= 0.0 ? angularVel : -angularVel;
                    }
                }, numParticles > 50);
        }
    }
}

void
PbdModel::solveConstraints()
{
    m_pbdSolver->setPbdBodies(&m_state);
    m_pbdSolver->setConstraints(getConstraints());
    m_pbdSolver->setTimeStep(m_config->m_dt);
    m_pbdSolver->setIterations(m_config->m_iterations);
    m_pbdSolver->setSolverType(m_config->m_solverType);
    m_pbdSolver->solve();
}

void
PbdModel::solveCollisionConstraints()
{
    m_pbdCollisionSolver->setPbdBodies(&m_state);
    m_pbdCollisionSolver->setTimeStep(m_config->m_dt);
    m_pbdCollisionSolver->setIterations(m_config->m_collisionIterations);
    m_pbdCollisionSolver->setSolverType(m_config->m_solverType);
    m_pbdCollisionSolver->solve();
}

void
PbdModel::resizeBodyParticles(PbdBody& body, const int particleCount)
{
    body.prevVertices->resize(particleCount);
    body.vertices->resize(particleCount);
    body.velocities->resize(particleCount);
    body.masses->resize(particleCount);
    body.invMasses->resize(particleCount);
    if (body.getOriented())
    {
        body.prevOrientations->resize(particleCount);
        body.orientations->resize(particleCount);
        body.angularVelocities->resize(particleCount);
        body.inertias->resize(particleCount);
        body.invInertias->resize(particleCount);
    }
}
} // namespace imstk