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

#include "imstkPbdModel.h"
#include "imstkGraph.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPbdFETetConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkPbdConstraintFunctor.h"

#include <map>
#include <set>

namespace imstk
{
PbdModel::PbdModel() : DynamicalModel(DynamicalModelType::PositionBasedDynamics),
    m_mass(std::make_shared<DataArray<double>>()),
    m_invMass(std::make_shared<DataArray<double>>()),
    m_fixedNodeInvMass(std::make_shared<std::unordered_map<size_t, double>>()),
    m_parameters(std::make_shared<PBDModelConfig>())
{
    m_validGeometryTypes = {
        "PointSet",
        "LineMesh",
        "SurfaceMesh",
        "TetrahedralMesh",
        "HexahedralMesh"
    };

    // Setup PBD compute nodes
    m_integrationPositionNode = m_taskGraph->addFunction("PbdModel_IntegratePosition", std::bind(&PbdModel::integratePosition, this));
    m_solveConstraintsNode    = m_taskGraph->addFunction("PbdModel_SolveConstraints", [&]() { m_pbdSolver->solve(); });   // Avoids rebinding on solver swap
    m_updateVelocityNode      = m_taskGraph->addFunction("PbdModel_UpdateVelocity", std::bind(&PbdModel::updateVelocity, this));
}

void
PBDModelConfig::enableConstraint(PbdConstraint::Type type, double stiffness)
{
    LOG_IF(FATAL, (type == PbdConstraint::Type::FEMTet || type == PbdConstraint::Type::FEMHex))
        << "FEM constraint should be enabled by the enableFEMConstraint function";
    m_regularConstraints.push_back({ type, stiffness });
}

void
PBDModelConfig::enableBendConstraint(const double stiffness, const int stride)
{
    m_regularConstraints.push_back({ PbdConstraint::Type::Bend, stiffness });
    m_constraintStrides[m_regularConstraints.size() - 1] = stride;
}

void
PBDModelConfig::enableFEMConstraint(PbdConstraint::Type type, PbdFEMConstraint::MaterialType material)
{
    LOG_IF(FATAL, (type != PbdConstraint::Type::FEMTet && type != PbdConstraint::Type::FEMHex))
        << "Non-FEM constraint should be enabled by the enableConstraint function";
    m_FEMConstraints.push_back({ type, material });
}

void
PBDModelConfig::setSolverType(const PbdConstraint::SolverType& type)
{
    if (type == PbdConstraint::SolverType::GCD)
    {
        LOG(WARNING) << "GCD is NOT implemented yet, use xPBD instead";
        m_solverType = PbdConstraint::SolverType::xPBD;
        return;
    }

    m_solverType = type;
}

void
PbdModel::configure(std::shared_ptr<PBDModelConfig> params)
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "Set PBD Model geometry before configuration!";

    m_parameters = params;
    this->setNumDegreeOfFreedom(std::dynamic_pointer_cast<PointSet>(m_geometry)->getNumVertices() * 3);
}

bool
PbdModel::initialize()
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "Model geometry is not yet set! Cannot initialize without model geometry.";

    initState();

    // Initialize constraints
    {
        m_constraints = std::make_shared<PbdConstraintContainer>();

        computeElasticConstants();

        auto pointSet = std::dynamic_pointer_cast<PointSet>(getModelGeometry());
        for (auto constraintType : m_parameters->m_FEMConstraints)
        {
            auto functor = std::make_shared<PbdFemConstraintFunctor>();
            functor->setFemConfig(m_parameters->m_femParams);
            functor->setMaterialType(constraintType.second);
            m_functors.push_back(functor);
        }
        int i = 0;
        for (auto constraintType : m_parameters->m_regularConstraints)
        {
            if (constraintType.first == PbdConstraint::Type::Area)
            {
                auto functor = std::make_shared<PbdAreaConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Bend)
            {
                // If bend constraint requested but no stride provided use 1
                int stride = 1;
                if (m_parameters->m_constraintStrides.count(i) > 0)
                {
                    stride = m_parameters->m_constraintStrides[i];
                }
                auto functor = std::make_shared<PbdBendConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                functor->setStride(stride);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::ConstantDensity)
            {
                auto functor = std::make_shared<PbdConstantDensityConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Dihedral)
            {
                auto functor = std::make_shared<PbdDihedralConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Distance)
            {
                auto functor = std::make_shared<PbdDistanceConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Volume)
            {
                auto functor = std::make_shared<PbdVolumeConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Distance)
            {
                auto functor = std::make_shared<PbdDistanceConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            else if (constraintType.first == PbdConstraint::Type::Volume)
            {
                auto functor = std::make_shared<PbdVolumeConstraintFunctor>();
                functor->setStiffness(constraintType.second);
                m_functors.push_back(functor);
            }
            i++;
        }

        for (auto functorPtr : m_functors)
        {
            PbdConstraintFunctor& functor = *functorPtr;
            functor.setGeometry(pointSet);
            functor(*m_constraints);
        }

        // Partition constraints for parallel computation
        if (m_parameters->m_doPartitioning)
        {
            m_constraints->partitionConstraints(m_partitionThreshold);
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
        m_pbdSolver->setIterations(m_parameters->m_iterations);
        m_pbdSolver->setSolverType(m_parameters->m_solverType);
    }
    m_pbdSolver->setPositions(getCurrentState()->getPositions());
    m_pbdSolver->setInvMasses(getInvMasses());
    m_pbdSolver->setConstraints(getConstraints());
    m_pbdSolver->setTimeStep(m_parameters->m_dt);

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

            const double uniformMass = m_parameters->m_uniformMassValue;
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
    for (auto i : m_parameters->m_fixedNodeIds)
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
PbdModel::computeElasticConstants()
{
    if (std::abs(m_parameters->m_femParams->m_mu) < MIN_REAL
        && std::abs(m_parameters->m_femParams->m_lambda) < MIN_REAL)
    {
        const auto E  = m_parameters->m_femParams->m_YoungModulus;
        const auto nu = m_parameters->m_femParams->m_PoissonRatio;
        m_parameters->m_femParams->m_mu     = E / Real(2.0) / (Real(1.0) + nu);
        m_parameters->m_femParams->m_lambda = E * nu / ((Real(1.0) + nu) * (Real(1.0) - Real(2.0) * nu));
    }
    else
    {
        const auto mu     = m_parameters->m_femParams->m_mu;
        const auto lambda = m_parameters->m_femParams->m_lambda;
        m_parameters->m_femParams->m_YoungModulus = mu * (Real(3.0) * lambda + Real(2.0) * mu) / (lambda + mu);
        m_parameters->m_femParams->m_PoissonRatio = lambda / Real(2.0) / (lambda + mu);
    }
}

void
PbdModel::addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices)
{
    // \todo: Refactor into functors, then move to ConstrainerContainer

    // check if constraint type matches the mesh type
    CHECK(m_mesh->getTypeName() == "SurfaceMesh")
        << "Add element constraints does not support current mesh type.";

    const auto&                      triMesh  = std::static_pointer_cast<SurfaceMesh>(m_mesh);
    const auto                       nV       = triMesh->getNumVertices();
    const auto                       elements = triMesh->getTriangleIndices();
    std::vector<std::vector<size_t>> onering(nV);

    // build onering
    for (auto& vertOnering : onering)
    {
        vertOnering.reserve(10);
    }
    for (int k = 0; k < elements->size(); ++k)
    {
        auto& tri = (*elements)[k];
        onering[tri[0]].push_back(k);
        onering[tri[1]].push_back(k);
        onering[tri[2]].push_back(k);
    }
    for (auto& vertOnering : onering)
    {
        std::sort(vertOnering.begin(), vertOnering.end());
    }

    // functions for adding constraints
    auto addDistanceConstraint =
        [&](size_t i1, size_t i2, double stiffness)
        {
            auto c = std::make_shared<PbdDistanceConstraint>();
            c->initConstraint(*m_initialState->getPositions(), i1, i2, stiffness);
            m_constraints->addConstraint(c);
        };
    auto addAreaConstraint =
        [&](size_t k, double stiffness)
        {
            auto& tri = (*elements)[k];
            auto  c   = std::make_shared<PbdAreaConstraint>();
            c->initConstraint(*m_initialState->getPositions(), tri[0], tri[1], tri[2], stiffness);
            m_constraints->addConstraint(c);
        };
    auto addDihedralConstraint =
        [&](size_t t0, size_t t1, size_t i1, size_t i2, double stiffness)
        {
            const auto& tri0 = (*elements)[t0];
            const auto& tri1 = (*elements)[t1];
            size_t      idx0 = 0;
            size_t      idx1 = 0;
            for (size_t i = 0; i < 3; ++i)
            {
                if (tri0[i] != i1 && tri0[i] != i2)
                {
                    idx0 = tri0[i];
                }
                if (tri1[i] != i1 && tri1[i] != i2)
                {
                    idx1 = tri1[i];
                }
            }
            auto c = std::make_shared<PbdDihedralConstraint>();
            c->initConstraint(*m_initialState->getPositions(), idx0, idx1, i1, i2, stiffness);
            m_constraints->addConstraint(c);
        };

    // count constraints to be added for pre-allocation
    std::set<std::pair<size_t, size_t>>                            distanceSet;
    std::unordered_set<size_t>                                     areaSet;
    std::map<std::pair<size_t, size_t>, std::pair<size_t, size_t>> dihedralSet;
    for (auto& constraint : m_parameters->m_regularConstraints)
    {
        switch (constraint.first)
        {
        case PbdConstraint::Type::Distance:
            for (const auto& vertIdx : *vertices)
            {
                for (const auto& triIdx : onering[vertIdx])
                {
                    const auto& tri = (*elements)[triIdx];
                    size_t      i1  = 0;
                    size_t      i2  = 0;
                    for (size_t i = 0; i < 3; i++)
                    {
                        if (tri[i] == vertIdx)
                        {
                            i1 = tri[(i + 1) % 3];
                            i2 = tri[(i + 2) % 3];
                            break;
                        }
                    }
                    auto pair1 = std::make_pair(std::min(vertIdx, i1), std::max(vertIdx, i1));
                    auto pair2 = std::make_pair(std::min(vertIdx, i2), std::max(vertIdx, i2));
                    distanceSet.insert(pair1);
                    distanceSet.insert(pair2);
                }
            }
            break;
        case PbdConstraint::Type::Area:
            for (const auto& vertIdx : *vertices)
            {
                for (const auto& triIdx : onering[vertIdx])
                {
                    areaSet.insert(triIdx);
                }
            }
            break;
        case PbdConstraint::Type::Dihedral:
            for (const auto& vertIdx : *vertices)
            {
                for (const auto& triIdx : onering[vertIdx])
                {
                    const auto& tri = (*elements)[triIdx];
                    for (size_t i = 0; i < 3; i++)
                    {
                        size_t j  = (i + 1) % 3;
                        size_t i0 = tri[i];
                        size_t i1 = tri[j];
                        if (i0 > i1)
                        {
                            std::swap(i0, i1);
                        }
                        auto&               r0 = onering[i0];
                        auto&               r1 = onering[i1];
                        std::vector<size_t> rs(2);
                        auto                it = std::set_intersection(r0.begin(), r0.end(), r1.begin(), r1.end(), rs.begin());
                        rs.resize(static_cast<size_t>(it - rs.begin()));
                        if (rs.size() > 1)
                        {
                            dihedralSet[std::make_pair(i0, i1)] = std::make_pair(rs[0], rs[1]);
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    // add constraints
    m_constraints->reserve(m_constraints->getConstraints().size() + distanceSet.size() + areaSet.size() + dihedralSet.size());
    for (auto& constraint : m_parameters->m_regularConstraints)
    {
        switch (constraint.first)
        {
        case PbdConstraint::Type::Distance:
            for (auto& c : distanceSet)
            {
                addDistanceConstraint(c.first, c.second, constraint.second);
            }
        case PbdConstraint::Type::Area:
            for (auto& c : areaSet)
            {
                addAreaConstraint(c, constraint.second);
            }
        case PbdConstraint::Type::Dihedral:
            for (auto& c : dihedralSet)
            {
                addDihedralConstraint(c.second.first, c.second.second, c.first.first, c.first.second, constraint.second);
            }
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
    VecDataArray<double, 3>& prevPos   = *m_previousState->getPositions();
    VecDataArray<double, 3>& pos       = *m_currentState->getPositions();
    VecDataArray<double, 3>& vel       = *m_currentState->getVelocities();
    VecDataArray<double, 3>& accn      = *m_currentState->getAccelerations();
    const DataArray<double>& invMasses = *m_invMass;

    ParallelUtils::parallelFor(m_mesh->getNumVertices(),
        [&](const size_t i)
        {
            if (std::abs(invMasses[i]) > 0.0)
            {
                vel[i]    += (accn[i] + m_parameters->m_gravity) * m_parameters->m_dt;
                accn[i]    = Vec3d::Zero();
                prevPos[i] = pos[i];
                pos[i]    += (1.0 - m_parameters->m_viscousDampingCoeff) * vel[i] * m_parameters->m_dt;
            }
        }, m_mesh->getNumVertices() > 50);
}

void
PbdModel::updateVelocity()
{
    const VecDataArray<double, 3>& prevPos   = *m_previousState->getPositions();
    const VecDataArray<double, 3>& pos       = *m_currentState->getPositions();
    VecDataArray<double, 3>&       vel       = *m_currentState->getVelocities();
    const DataArray<double>&       invMasses = *m_invMass;

    if (m_parameters->m_dt > 0.0)
    {
        const double invDt = 1.0 / m_parameters->m_dt;
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
}