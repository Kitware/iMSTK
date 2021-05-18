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
#include "imstkPbdAreaConstraint.h"
#include "imstkPbdBendConstraint.h"
#include "imstkPbdConstantDensityConstraint.h"
#include "imstkPbdDihedralConstraint.h"
#include "imstkPbdDistanceConstraint.h"
#include "imstkPbdFETetConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkPbdVolumeConstraint.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"

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
    m_integrationPositionNode     = m_taskGraph->addFunction("PbdModel_IntegratePosition", std::bind(&PbdModel::integratePosition, this));
    m_updateCollisionGeometryNode = m_taskGraph->addFunction("PbdModel_UpdateCollisionGeometry", std::bind(&PbdModel::updatePhysicsGeometry, this));
    m_solveConstraintsNode = m_taskGraph->addFunction("PbdModel_SolveConstraints", [&]() { m_pbdSolver->solve(); });      // Avoids rebinding on solver swap
    m_updateVelocityNode   = m_taskGraph->addFunction("PbdModel_UpdateVelocity", std::bind(&PbdModel::updateVelocity, this));
}

void
PBDModelConfig::enableConstraint(PbdConstraint::Type type, double stiffness)
{
    LOG_IF(FATAL, (type == PbdConstraint::Type::FEMTet || type == PbdConstraint::Type::FEMHex))
        << "FEM constraint should be enabled by the enableFEMConstraint function";
    m_regularConstraints.push_back({ type, stiffness });
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
    bool bOK = true; // Return immediately if some constraint failed to initialize

    initState();

    // Initialize constraints
    {
        m_constraints = std::make_shared<PBDConstraintVector>();
        m_partitionedConstraints = std::make_shared<std::vector<PBDConstraintVector>>();

        // Initialize FEM constraints
        for (auto& constraint : m_parameters->m_FEMConstraints)
        {
            computeElasticConstants();
            if (!initializeFEMConstraints(constraint.second))
            {
                return false;
            }
        }

        // Initialize other constraints
        for (auto& constraint : m_parameters->m_regularConstraints)
        {
            if (m_parameters->m_solverType == PbdConstraint::SolverType::PBD && constraint.second > 1.0)
            {
                LOG(WARNING) << "for PBD, k should be between [0, 1]";
            }
            else if (m_parameters->m_solverType == PbdConstraint::SolverType::xPBD && constraint.second <= 1.0)
            {
                LOG(WARNING) << "for xPBD, k is Young's Modulu, and should be much larger than 1";
            }

            if (!bOK)
            {
                return false;
            }
            switch (constraint.first)
            {
            case PbdConstraint::Type::Volume:
                bOK = initializeVolumeConstraints(constraint.second);
                break;

            case PbdConstraint::Type::Distance:
                bOK = initializeDistanceConstraints(constraint.second);
                break;

            case PbdConstraint::Type::Area:
                bOK = initializeAreaConstraints(constraint.second);
                break;

            case PbdConstraint::Type::Bend:
                bOK = initializeBendConstraints(constraint.second);
                break;

            case PbdConstraint::Type::Dihedral:
                bOK = initializeDihedralConstraints(constraint.second);
                break;

            case PbdConstraint::Type::ConstantDensity:
                bOK = initializeConstantDensityConstraint(constraint.second);
                break;

            default:
                LOG(FATAL) << "Invalid constraint type";
            }
        }

        // Partition constraints for parallel computation
        if (m_parameters->m_doPartitioning)
        {
            this->partitionConstraints();
        }
        else
        {
            m_partitionedConstraints->clear();
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
    m_pbdSolver->setPartitionedConstraints(getPartitionedConstraints());
    m_pbdSolver->setTimeStep(m_parameters->m_dt);

    this->setTimeStepSizeType(m_timeStepSizeType);

    return bOK;
}

void
PbdModel::initState()
{
    // Get the mesh
    m_mesh = std::dynamic_pointer_cast<PointSet>(m_geometry);
    const int numParticles = static_cast<int>(m_mesh->getNumVertices());

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
    m_taskGraph->addEdge(m_integrationPositionNode, m_updateCollisionGeometryNode);
    m_taskGraph->addEdge(m_updateCollisionGeometryNode, m_solveConstraintsNode);
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

bool
PbdModel::initializeFEMConstraints(PbdFEMConstraint::MaterialType type)
{
    // Check if constraint type matches the mesh type
    CHECK(m_mesh->getTypeName() == "TetrahedralMesh")
        << "FEM Tetrahedral constraint should come with tetrahedral mesh";

    // Create constraints
    const auto&                 tetMesh  = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
    const VecDataArray<int, 4>& elements = *tetMesh->getTetrahedraIndices();

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(elements.size(),
        [&](const size_t k)
        {
            const Vec4i& tet = elements[k];
            auto c = std::make_shared<PbdFEMTetConstraint>(type);
            c->initConstraint(*m_initialState->getPositions(),
                tet[0], tet[1], tet[2], tet[3], m_parameters->m_femParams);
            lock.lock();
            m_constraints->push_back(std::move(c));
            lock.unlock();
        });
    return true;
}

bool
PbdModel::initializeVolumeConstraints(const double stiffness)
{
    // Check if constraint type matches the mesh type
    CHECK(m_mesh->getTypeName() == "TetrahedralMesh")
        << "Volume constraint should come with volumetric mesh";

    // Create constraints
    const auto&                 tetMesh  = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
    const VecDataArray<int, 4>& elements = *tetMesh->getTetrahedraIndices();

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(elements.size(),
        [&](const size_t k)
        {
            auto& tet = elements[k];
            auto c    = std::make_shared<PbdVolumeConstraint>();
            c->initConstraint(*m_initialState->getPositions(),
                tet[0], tet[1], tet[2], tet[3], stiffness);
            lock.lock();
            m_constraints->push_back(std::move(c));
            lock.unlock();
        });
    return true;
}

bool
PbdModel::initializeDistanceConstraints(const double stiffness)
{
    auto addConstraint =
        [&](std::vector<std::vector<bool>>& E, size_t i1, size_t i2)
        {
            if (i1 > i2)     // Make sure i1 is always smaller than i2
            {
                std::swap(i1, i2);
            }
            if (E[i1][i2])
            {
                E[i1][i2] = 0;
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*m_initialState->getPositions(), i1, i2, stiffness);
                m_constraints->push_back(std::move(c));
            }
        };

    if (m_mesh->getTypeName() == "TetrahedralMesh")
    {
        const auto&                    tetMesh  = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
        const VecDataArray<int, 4>&    elements = *tetMesh->getTetrahedraIndices();
        const auto                     nV       = tetMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

        for (int k = 0; k < elements.size(); ++k)
        {
            auto& tet = elements[k];
            addConstraint(E, tet[0], tet[1]);
            addConstraint(E, tet[0], tet[2]);
            addConstraint(E, tet[0], tet[3]);
            addConstraint(E, tet[1], tet[2]);
            addConstraint(E, tet[1], tet[3]);
            addConstraint(E, tet[2], tet[3]);
        }
    }
    else if (m_mesh->getTypeName() == "SurfaceMesh")
    {
        const auto&                    triMesh  = std::static_pointer_cast<SurfaceMesh>(m_mesh);
        const VecDataArray<int, 3>&    elements = *triMesh->getTriangleIndices();
        const auto                     nV       = triMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

        for (int k = 0; k < elements.size(); ++k)
        {
            auto& tri = elements[k];
            addConstraint(E, tri[0], tri[1]);
            addConstraint(E, tri[0], tri[2]);
            addConstraint(E, tri[1], tri[2]);
        }
    }
    else if (m_mesh->getTypeName() == "LineMesh")
    {
        const auto&                    lineMesh = std::static_pointer_cast<LineMesh>(m_mesh);
        const VecDataArray<int, 2>&    elements = *lineMesh->getLinesIndices();
        const auto&                    nV       = lineMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

        for (int k = 0; k < elements.size(); k++)
        {
            auto& seg = elements[k];
            addConstraint(E, seg[0], seg[1]);
        }
    }

    return true;
}

bool
PbdModel::initializeAreaConstraints(const double stiffness)
{
    // check if constraint type matches the mesh type
    CHECK(m_mesh->getTypeName() == "SurfaceMesh")
        << "Area constraint should come with a triangular mesh";

    // ok, now create constraints
    const auto&                 triMesh  = std::static_pointer_cast<SurfaceMesh>(m_mesh);
    const VecDataArray<int, 3>& elements = *triMesh->getTriangleIndices();

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(elements.size(),
        [&](const size_t k)
        {
            auto& tri = elements[k];
            auto c    = std::make_shared<PbdAreaConstraint>();
            c->initConstraint(*m_initialState->getPositions(), tri[0], tri[1], tri[2], stiffness);
            lock.lock();
            m_constraints->push_back(std::move(c));
            lock.unlock();
        });
    return true;
}

bool
PbdModel::initializeBendConstraints(const double stiffness)
{
    CHECK(m_mesh->getTypeName() == "LineMesh")
        << "Bend constraint should come with a line mesh";

    auto addConstraint =
        [&](const double k, size_t i1, size_t i2, size_t i3)
        {
            // i1 should always come first
            if (i2 < i1)
            {
                std::swap(i1, i2);
            }
            // i3 should always come last
            if (i2 > i3)
            {
                std::swap(i2, i3);
            }

            auto c = std::make_shared<PbdBendConstraint>();
            c->initConstraint(*m_initialState->getPositions(), i1, i2, i3, k);
            m_constraints->push_back(std::move(c));
        };

    // Create constraints
    const auto&                 lineMesh = std::static_pointer_cast<LineMesh>(m_mesh);
    const VecDataArray<int, 2>& elements = *lineMesh->getLinesIndices();

    // Iterate sets of two segments
    for (int k = 0; k < elements.size() - 1; k++)
    {
        auto& seg1 = elements[k];
        auto& seg2 = elements[k + 1];
        int   i3   = seg2[0];
        if (i3 == seg1[0] || i3 == seg1[1])
        {
            i3 = seg2[1];
        }
        addConstraint(stiffness, seg1[0], seg1[1], i3);
    }
    return true;
}

bool
PbdModel::initializeDihedralConstraints(const double stiffness)
{
    CHECK(m_mesh->getTypeName() == "SurfaceMesh")
        << "Dihedral constraint should come with a triangular mesh";

    // Create constraints
    const auto&                      triMesh  = std::static_pointer_cast<SurfaceMesh>(m_mesh);
    const VecDataArray<int, 3>&      elements = *triMesh->getTriangleIndices();
    const auto                       nV       = triMesh->getNumVertices();
    std::vector<std::vector<size_t>> onering(nV);

    for (int k = 0; k < elements.size(); ++k)
    {
        auto& tri = elements[k];
        onering[tri[0]].push_back(k);
        onering[tri[1]].push_back(k);
        onering[tri[2]].push_back(k);
    }

    std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

    auto addConstraint =
        [&](std::vector<size_t>& r1, std::vector<size_t>& r2,
            const size_t k, size_t i1, size_t i2)
        {
            if (i1 > i2) // Make sure i1 is always smaller than i2
            {
                std::swap(i1, i2);
            }
            if (E[i1][i2])
            {
                E[i1][i2] = 0;

                std::vector<size_t> rs(2);
                auto                it = std::set_intersection(r1.begin(), r1.end(), r2.begin(), r2.end(), rs.begin());
                rs.resize(static_cast<size_t>(it - rs.begin()));
                if (rs.size() > 1)
                {
                    size_t      idx  = (rs[0] == k) ? 1 : 0;
                    const auto& tri0 = elements[k];
                    const auto& tri1 = elements[rs[idx]];
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
                    m_constraints->push_back(std::move(c));
                }
            }
        };

    for (int k = 0; k < elements.size(); ++k)
    {
        auto& tri = elements[k];

        auto& r0 = onering[tri[0]];
        auto& r1 = onering[tri[1]];
        auto& r2 = onering[tri[2]];

        std::sort(r0.begin(), r0.end());
        std::sort(r1.begin(), r1.end());
        std::sort(r2.begin(), r2.end());

        addConstraint(r0, r1, k, tri[0], tri[1]);
        addConstraint(r0, r2, k, tri[0], tri[2]);
        addConstraint(r1, r2, k, tri[1], tri[2]);
    }
    return true;
}

bool
PbdModel::initializeConstantDensityConstraint(const double stiffness)
{
    // check if constraint type matches the mesh type
    CHECK(std::dynamic_pointer_cast<PointSet>(m_mesh) != nullptr)
        << "Constant constraint should come with a mesh!";

    auto c = std::make_shared<PbdConstantDensityConstraint>();
    c->initConstraint(*m_initialState->getPositions(), stiffness);
    m_constraints->push_back(std::move(c));

    return true;
}

void
PbdModel::removeConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices)
{
    // constraint removal predicate
    auto removeConstraint =
        [&](std::shared_ptr<PbdConstraint> constraint)
        {
            for (auto i : constraint->getVertexIds())
            {
                if (vertices->find(i) != vertices->end())
                {
                    return true;
                }
            }
            return false;
        };

    // remove constraints from constraint-partition maps and constraint vectors.
    m_constraints->erase(std::remove_if(m_constraints->begin(), m_constraints->end(), removeConstraint),
                         m_constraints->end());
    for (auto& pc : *m_partitionedConstraints)
    {
        pc.erase(std::remove_if(pc.begin(), pc.end(), removeConstraint), pc.end());
    }
}

void
PbdModel::addConstraints(std::shared_ptr<std::unordered_set<size_t>> vertices)
{
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
            m_constraints->push_back(std::move(c));
        };
    auto addAreaConstraint =
        [&](size_t k, double stiffness)
        {
            auto& tri = (*elements)[k];
            auto  c   = std::make_shared<PbdAreaConstraint>();
            c->initConstraint(*m_initialState->getPositions(), tri[0], tri[1], tri[2], stiffness);
            m_constraints->push_back(std::move(c));
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
            m_constraints->push_back(std::move(c));
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
    m_constraints->reserve(m_constraints->size() + distanceSet.size() + areaSet.size() + dihedralSet.size());
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
PbdModel::partitionConstraints(const bool print)
{
    // Form the map { vertex : list_of_constraints_involve_vertex }
    PBDConstraintVector& allConstraints = *m_constraints;

    //std::cout << "---------partitionConstraints: " << allConstraints.size() << std::endl;

    std::unordered_map<size_t, std::vector<size_t>> vertexConstraints;
    for (size_t constrIdx = 0; constrIdx < allConstraints.size(); ++constrIdx)
    {
        const auto& constr = allConstraints[constrIdx];
        for (const auto& vIds : constr->getVertexIds())
        {
            vertexConstraints[vIds].push_back(constrIdx);
        }
    }

    // Add edges to the constraint graph
    // Each edge represent a shared vertex between two constraints
    Graph constraintGraph(allConstraints.size());
    for (const auto& kv : vertexConstraints)
    {
        const auto& constraints = kv.second;     // the list of constraints for a vertex
        for (size_t i = 0; i < constraints.size(); ++i)
        {
            for (size_t j = i + 1; j < constraints.size(); ++j)
            {
                constraintGraph.addEdge(constraints[i], constraints[j]);
            }
        }
    }
    vertexConstraints.clear();

    // do graph coloring for the constraint graph
    const auto coloring = constraintGraph.doColoring(Graph::ColoringMethod::WelshPowell);
    // const auto  coloring = constraintGraph.doColoring(Graph::ColoringMethod::Greedy);
    const auto& partitionIndices = coloring.first;
    const auto  numPartitions    = coloring.second;
    assert(partitionIndices.size() == allConstraints.size());

    std::vector<PBDConstraintVector>& partitionedConstraints = *m_partitionedConstraints;
    partitionedConstraints.resize(0);
    partitionedConstraints.resize(static_cast<size_t>(numPartitions));

    for (size_t constrIdx = 0; constrIdx < partitionIndices.size(); ++constrIdx)
    {
        const auto partitionIdx = partitionIndices[constrIdx];
        partitionedConstraints[partitionIdx].push_back(allConstraints[constrIdx]);
    }

    // If a partition has size smaller than the partition threshold, then move its constraints back
    // These constraints will be processed sequentially
    // Because small size partitions yield bad performance upon running in parallel
    allConstraints.resize(0);
    for (const auto& constraints : partitionedConstraints)
    {
        if (constraints.size() < m_partitionThreshold)
        {
            for (size_t constrIdx = 0; constrIdx < constraints.size(); ++constrIdx)
            {
                allConstraints.push_back(std::move(constraints[constrIdx]));
            }
        }
    }

    // Remove all empty partitions
    size_t writeIdx = 0;
    for (size_t readIdx = 0; readIdx < partitionedConstraints.size(); ++readIdx)
    {
        if (partitionedConstraints[readIdx].size() >= m_partitionThreshold)
        {
            if (readIdx != writeIdx)
            {
                partitionedConstraints[writeIdx] = std::move(partitionedConstraints[readIdx]);
            }
            ++writeIdx;
        }
    }
    partitionedConstraints.resize(writeIdx);

    // Print
    if (print)
    {
        size_t numConstraints = 0;
        int    idx = 0;
        for (const auto& constraints : partitionedConstraints)
        {
            std::cout << "Partition # " << idx++ << " | # nodes: " << constraints.size() << std::endl;
            numConstraints += constraints.size();
        }
        std::cout << "Sequential processing # nodes: " << allConstraints.size() << std::endl;
        numConstraints += allConstraints.size();
        std::cout << "Total constraints: " << numConstraints << " | Graph size: "
                  << constraintGraph.size() << std::endl;
    }
}

void
PbdModel::setTimeStepSizeType(const TimeSteppingType type)
{
    m_timeStepSizeType = type;
    if (type == TimeSteppingType::Fixed)
    {
        m_parameters->m_dt = m_parameters->m_defaultDt;
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
} // imstk
