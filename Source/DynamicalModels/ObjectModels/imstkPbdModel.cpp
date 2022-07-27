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
#include "imstkPbdSolver.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkPbdConstraintFunctor.h"

namespace imstk
{
void
PbdModelConfig::computeElasticConstants()
{
    if (std::abs(m_femParams->m_mu) < std::numeric_limits<double>::min()
        && std::abs(m_femParams->m_lambda) < std::numeric_limits<double>::min())
    {
        const double E  = m_femParams->m_YoungModulus;
        const double nu = m_femParams->m_PoissonRatio;
        m_femParams->m_mu     = E / 2.0 / (1.0 + nu);
        m_femParams->m_lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
    }
    else
    {
        const double mu     = m_femParams->m_mu;
        const double lambda = m_femParams->m_lambda;
        m_femParams->m_YoungModulus = mu * (3.0 * lambda + 2.0 * mu) / (lambda + mu);
        m_femParams->m_PoissonRatio = lambda / 2.0 / (lambda + mu);
    }
}

void
PbdModelConfig::enableConstraint(ConstraintGenType type, double stiffness)
{
    auto& funcs = m_functors[type];
    if (type == ConstraintGenType::Distance)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdDistanceConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdDistanceConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
    }
    else if (type == ConstraintGenType::Volume)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdVolumeConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdVolumeConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
    }
    else if (type == ConstraintGenType::Area)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdAreaConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdAreaConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
    }
    else if (type == ConstraintGenType::Bend)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdBendConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdBendConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
        functor->setStride(1);
    }
    else if (type == ConstraintGenType::Dihedral)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdDihedralConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdDihedralConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
    }
    else if (type == ConstraintGenType::ConstantDensity)
    {
        if (funcs.size() == 0)
        {
            funcs.push_back(std::make_shared<PbdConstantDensityConstraintFunctor>());
        }
        auto functor = std::dynamic_pointer_cast<PbdConstantDensityConstraintFunctor>(funcs.front());
        functor->setStiffness(stiffness);
    }
    else
    {
        LOG(FATAL) << "There exists no standard constraint functor for the ConstraintGenType";
    }
}

void
PbdModelConfig::enableBendConstraint(const double stiffness, const int stride, const bool restLength0)
{
    auto& funcs = m_functors[ConstraintGenType::Bend];

    // Find the functor with the same stride
    std::shared_ptr<PbdBendConstraintFunctor> foundFunctor = nullptr;
    for (auto functor : funcs)
    {
        auto bendFunctor =
            std::dynamic_pointer_cast<PbdBendConstraintFunctor>(functor);
        if (bendFunctor->getStride() == stride)
        {
            foundFunctor = bendFunctor;
            break;
        }
    }

    // If one with stride not found, create our own
    if (foundFunctor == nullptr)
    {
        foundFunctor = std::make_shared<PbdBendConstraintFunctor>();
        funcs.push_back(foundFunctor);
    }

    foundFunctor->setRestLength(restLength0 ? 0.0 : -1.0);
    foundFunctor->setStiffness(stiffness);
    foundFunctor->setStride(stride);
}

void
PbdModelConfig::enableConstantDensityConstraint(const double stiffness,
                                                const double particleRadius, const double restDensity)
{
    auto& funcs = m_functors[ConstraintGenType::ConstantDensity];

    // Find the functor with the same stride
    std::shared_ptr<PbdConstantDensityConstraintFunctor> foundFunctor = nullptr;
    if (funcs.size() != 0)
    {
        foundFunctor = std::dynamic_pointer_cast<PbdConstantDensityConstraintFunctor>(funcs[0]);
    }

    // If not found, create it
    if (foundFunctor == nullptr)
    {
        foundFunctor = std::make_shared<PbdConstantDensityConstraintFunctor>();
        funcs.push_back(foundFunctor);
    }

    foundFunctor->setParticleRadius(particleRadius);
    foundFunctor->setStiffness(stiffness);
    foundFunctor->setRestDensity(restDensity);
}

void
PbdModelConfig::enableFemConstraint(PbdFemConstraint::MaterialType material)
{
    auto& funcs = m_functors[ConstraintGenType::FemTet];
    if (funcs.size() == 0)
    {
        funcs.push_back(std::make_shared<PbdFemTetConstraintFunctor>());
    }
    auto functor = std::dynamic_pointer_cast<PbdFemTetConstraintFunctor>(funcs.front());
    functor->setFemConfig(m_femParams);
    functor->setMaterialType(material);
}

PbdModel::PbdModel() : DynamicalModel(DynamicalModelType::PositionBasedDynamics),
    m_mass(std::make_shared<DataArray<double>>()),
    m_invMass(std::make_shared<DataArray<double>>()),
    m_fixedNodeInvMass(std::make_shared<std::unordered_map<size_t, double>>()),
    m_config(std::make_shared<PbdModelConfig>())
{
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
    m_updateVelocityNode = m_taskGraph->addFunction("PbdModel_UpdateVelocity",
        [&]() { updateVelocity(); });
}

void
PbdModel::configure(std::shared_ptr<PbdModelConfig> config)
{
    m_config = config;
}

bool
PbdModel::initialize()
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "Model geometry is not yet set! Cannot initialize without model geometry.";

    initState();

    // Initialize constraints
    {
        m_constraints = std::make_shared<PbdConstraintContainer>();

        m_config->computeElasticConstants();

        auto pointSet = std::dynamic_pointer_cast<PointSet>(getModelGeometry());
        for (auto functorVec : m_config->m_functors)
        {
            for (auto functorPtr : functorVec.second)
            {
                PbdConstraintFunctor& functor = *functorPtr;
                functor.setGeometry(pointSet);
                functor(*m_constraints);
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

    // Setup the default pbd solver if none was provided
    if (m_pbdSolver == nullptr)
    {
        m_pbdSolver = std::make_shared<PbdSolver>();
        m_pbdSolver->setIterations(m_config->m_iterations);
        m_pbdSolver->setSolverType(m_config->m_solverType);
    }
    m_pbdSolver->setPositions(getCurrentState()->getPositions());
    m_pbdSolver->setInvMasses(getInvMasses());
    m_pbdSolver->setConstraints(getConstraints());
    m_pbdSolver->setTimeStep(m_config->m_dt);

    this->setTimeStepSizeType(m_timeStepSizeType);

    return true;
}

void
PbdModel::initState()
{
    // Get the mesh
    m_mesh = std::dynamic_pointer_cast<PointSet>(m_geometry);
    const int numParticles = m_mesh->getNumVertices();

    m_initialState  = std::make_shared<PbdState>(numParticles);
    m_previousState = std::make_shared<PbdState>(numParticles);
    m_currentState  = std::make_shared<PbdState>(numParticles);

    // Set the positional values (by ptr reference)
    m_initialState->setPositions(m_mesh->getInitialVertexPositions());
    m_currentState->setPositions(m_mesh->getVertexPositions());
    m_previousState->setPositions(std::make_shared<VecDataArray<double, 3>>(*m_mesh->getVertexPositions()));

    // Initialize Mass+InvMass
    {
        // If the input mesh has masses defined, use those
        std::shared_ptr<AbstractDataArray> masses = m_mesh->getVertexAttribute("Mass");
        if (masses != nullptr && masses->getNumberOfComponents() == 1 && masses->getScalarType() == IMSTK_DOUBLE && masses->size() == numParticles)
        {
            m_mass = std::dynamic_pointer_cast<DataArray<double>>(masses);
            m_invMass->resize(m_mass->size());
            for (int i = 0; i < m_mass->size(); i++)
            {
                (*m_invMass)[i] = ((*m_mass)[i] == 0.0) ? 0.0 : 1.0 / (*m_mass)[i];
            }
        }
        // If not, initialize as uniform and put on mesh
        else
        {
            // Initialize as uniform
            m_mass->resize(numParticles);
            m_invMass->resize(numParticles);

            const double uniformMass = m_config->m_uniformMassValue;
            std::fill(m_mass->begin(), m_mass->end(), uniformMass);
            std::fill(m_invMass->begin(), m_invMass->end(), (uniformMass != 0.0) ? 1.0 / uniformMass : 0.0);

            m_mesh->setVertexAttribute("Mass", m_mass);
        }
        m_mesh->setVertexAttribute("InvMass", m_invMass);
    }

    // Initialize Velocities
    {
        // If the input mesh has per vertex velocities, use those
        std::shared_ptr<AbstractDataArray> velocities = m_mesh->getVertexAttribute("Velocities");
        if (velocities != nullptr && velocities->getNumberOfComponents() == 3 && velocities->getScalarType() == IMSTK_DOUBLE
            && std::dynamic_pointer_cast<VecDataArray<double, 3>>(velocities)->size() == numParticles)
        {
            m_currentState->setVelocities(std::dynamic_pointer_cast<VecDataArray<double, 3>>(velocities));
        }
        // If not, put existing (0 initialized velocities) on mesh
        else
        {
            m_mesh->setVertexAttribute("Velocities", m_currentState->getVelocities());
        }
    }

    // Define accelerations on the geometry
    m_mesh->setVertexAttribute("Accelerations", m_currentState->getAccelerations());

    // Overwrite some masses for specified fixed points
    for (auto i : m_config->m_fixedNodeIds)
    {
        setFixedPoint(i);
    }
}

void
PbdModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_integrationPositionNode);
    m_taskGraph->addEdge(m_integrationPositionNode, m_solveConstraintsNode);
    m_taskGraph->addEdge(m_solveConstraintsNode, m_updateVelocityNode);
    m_taskGraph->addEdge(m_updateVelocityNode, sink);
}

void
PbdModel::addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices)
{
    for (const auto& functorVec : m_config->m_functors)
    {
        for (const auto& functor : functorVec.second)
        {
            functor->addConstraints(*m_constraints, vertices);
        }
    }
}

void
PbdModel::setParticleMass(const double val, const size_t idx)
{
    DataArray<double>& masses    = *m_mass;
    DataArray<double>& invMasses = *m_invMass;
    if (idx < m_mesh->getNumVertices())
    {
        masses[idx]    = val;
        invMasses[idx] = 1.0 / val;
    }
}

void
PbdModel::setFixedPoint(const size_t idx)
{
    DataArray<double>&                  invMasses = *m_invMass;
    std::unordered_map<size_t, double>& fixedNodeInvMass = *m_fixedNodeInvMass;
    if (idx < m_mesh->getNumVertices())
    {
        fixedNodeInvMass[idx] = invMasses[idx];
        invMasses[idx] = 0.0;
    }
}

void
PbdModel::setPointUnfixed(const size_t idx)
{
    DataArray<double>&                  invMasses = *m_invMass;
    std::unordered_map<size_t, double>& fixedNodeInvMass = *m_fixedNodeInvMass;
    if (fixedNodeInvMass.find(idx) != fixedNodeInvMass.end())
    {
        invMasses[idx] = fixedNodeInvMass[idx];
        fixedNodeInvMass.erase(idx);
    }
}

void
PbdModel::integratePosition()
{
    std::shared_ptr<VecDataArray<double, 3>> prevPosPtr = m_previousState->getPositions();
    VecDataArray<double, 3>&                 prevPos    = *prevPosPtr;

    std::shared_ptr<VecDataArray<double, 3>> posPtr = m_currentState->getPositions();
    VecDataArray<double, 3>&                 pos    = *posPtr;

    std::shared_ptr<VecDataArray<double, 3>> velPtr = m_currentState->getVelocities();
    VecDataArray<double, 3>&                 vel    = *velPtr;

    std::shared_ptr<VecDataArray<double, 3>> accnPtr   = m_currentState->getAccelerations();
    VecDataArray<double, 3>&                 accn      = *accnPtr;
    const DataArray<double>&                 invMasses = *m_invMass;

    ParallelUtils::parallelFor(m_mesh->getNumVertices(),
        [&](const size_t i)
        {
            if (std::abs(invMasses[i]) > 0.0)
            {
                vel[i]    += (accn[i] + m_config->m_gravity) * m_config->m_dt;
                accn[i]    = Vec3d::Zero();
                prevPos[i] = pos[i];
                pos[i]    += (1.0 - m_config->m_viscousDampingCoeff) * vel[i] * m_config->m_dt;
            }
        }, m_mesh->getNumVertices() > 50);
}

void
PbdModel::updateVelocity()
{
    std::shared_ptr<VecDataArray<double, 3>> prevPosPtr = m_previousState->getPositions();
    const VecDataArray<double, 3>&           prevPos    = *prevPosPtr;
    std::shared_ptr<VecDataArray<double, 3>> posPtr     = m_currentState->getPositions();
    const VecDataArray<double, 3>&           pos       = *posPtr;
    std::shared_ptr<VecDataArray<double, 3>> velPtr    = m_currentState->getVelocities();
    VecDataArray<double, 3>&                 vel       = *velPtr;
    const DataArray<double>&                 invMasses = *m_invMass;

    if (m_config->m_dt > 0.0)
    {
        const double invDt = 1.0 / m_config->m_dt;
        ParallelUtils::parallelFor(m_mesh->getNumVertices(),
            [&](const size_t i)
            {
                if (std::abs(invMasses[i]) > 0.0)
                {
                    vel[i] = (pos[i] - prevPos[i]) * invDt;
                }
            }, m_mesh->getNumVertices() > 50);
    }
}

void
PbdModel::solveConstraints()
{
    m_pbdSolver->setPositions(m_currentState->getPositions());
    m_pbdSolver->setInvMasses(m_invMass);
    m_pbdSolver->setConstraints(getConstraints());
    m_pbdSolver->setTimeStep(m_config->m_dt);
    m_pbdSolver->setIterations(m_config->m_iterations);
    m_pbdSolver->setSolverType(m_config->m_solverType);
    m_pbdSolver->solve();
}
} // namespace imstk