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
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkPbdVolumeConstraint.h"
#include "imstkPbdDistanceConstraint.h"
#include "imstkPbdDihedralConstraint.h"
#include "imstkPbdAreaConstraint.h"
#include "imstkPbdFETetConstraint.h"
#include "imstkPbdFEHexConstraint.h"

#include <g3log/g3log.hpp>

namespace imstk
{

PbdModel::PbdModel() :
DynamicalModel(DynamicalModelType::positionBasedDynamics)
{
    m_initialState = std::make_shared<PbdState>();
    m_previousState = std::make_shared<PbdState>();
    m_currentState = std::make_shared<PbdState>();
}

void
PbdModel::setModelGeometry(std::shared_ptr<Mesh> m)
{
    m_mesh = m;
}

bool
PbdModel::initialize()
{
    if (m_mesh)
    {
        bool option[3] = { 1, 0, 0 };
        m_initialState->initialize(m_mesh, option);
        m_previousState->initialize(m_mesh, option);

        option[1] = option[2] = 1;
        m_currentState->initialize(m_mesh, option);

        m_initialState->setPositions(m_mesh->getVertexPositions());
        m_currentState->setPositions(m_mesh->getVertexPositions());

        auto nP = m_mesh->getNumVertices();
        m_invMass.resize(nP, 0);
        m_mass.resize(nP, 0);

        return true;
    }
    else
    {
        LOG(WARNING) << "Model geometry is not yet set! Cannot initialize without model geometry.";
        return false;
    }
}

void PbdModel::computeLameConstants(const double& E, const double nu)
{
    m_mu = E/(2*(1+nu));
    m_lambda = E*nu/((1-2*nu)*(1+nu));
}

bool
PbdModel::initializeFEMConstraints(PbdFEMConstraint::MaterialType type)
{
    // Check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::TetrahedralMesh)
    {
        LOG(WARNING) << "FEM Tetrahedral constraint should come with tetrahedral mesh";
        return false;
    }

    // Create constraints
    auto tetMesh = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
    auto elements = tetMesh->getTetrahedraVertices();

    for (size_t k = 0; k < elements.size(); ++k)
    {
        auto& tet = elements[k];

        auto c = std::make_shared<PbdFEMTetConstraint>(type);
        c->initConstraint(*this, tet[0], tet[1], tet[2], tet[3]);
        m_constraints.push_back(c);
    }
    return true;
}

bool
PbdModel::initializeVolumeConstraints(const double& stiffness)
{
    // Check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::TetrahedralMesh)
    {
        LOG(WARNING) << "Volume constraint should come with volumetric mesh";
        return false;
    }

    // Create constraints
    auto tetMesh = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
    auto elements = tetMesh->getTetrahedraVertices();

    for (size_t k = 0; k < elements.size(); ++k)
    {
        auto& tet = elements[k];

        auto c = std::make_shared<PbdVolumeConstraint>();
        c->initConstraint(*this, tet[0], tet[1], tet[2], tet[3], stiffness);
        m_constraints.push_back(c);
    }
    return true;
}

bool
PbdModel::initializeDistanceConstraints(const double& stiffness)
{
    if (m_mesh->getType() == Geometry::Type::TetrahedralMesh)
    {
        auto tetMesh = std::static_pointer_cast<TetrahedralMesh>(m_mesh);
        auto nV = tetMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));
        auto elements = tetMesh->getTetrahedraVertices();

        for (size_t k = 0; k < elements.size(); ++k)
        {
            auto& tet = elements[k];

            auto i1 = tet[0];
            auto i2 = tet[1];

            // check if added or not
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tet[1];
            i2 = tet[2];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tet[2];
            i2 = tet[0];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tet[0];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tet[1];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tet[2];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
        }
    }
    else if (m_mesh->getType() == Geometry::Type::SurfaceMesh)
    {
        auto triMesh = std::static_pointer_cast<SurfaceMesh>(m_mesh);
        auto nV = triMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));
        auto elements = triMesh->getTrianglesVertices();

        for (size_t k = 0; k < elements.size(); ++k)
        {
            auto& tri = elements[k];

            auto i1 = tri[0];
            auto i2 = tri[1];

            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tri[1];
            i2 = tri[2];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

            i1 = tri[2];
            i2 = tri[0];
            if (E[i1][i2] && E[i2][i1])
            {
                auto c = std::make_shared<PbdDistanceConstraint>();
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
        }
    }
    return true;
}

bool
PbdModel::initializeAreaConstraints(const double& stiffness)
{
    // check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(WARNING) << "Area constraint should come with a triangular mesh";
        return false;
    }

    // ok, now create constraints
    auto triMesh = std::static_pointer_cast<SurfaceMesh>(m_mesh);
    std::vector<SurfaceMesh::TriangleArray> elements = triMesh->getTrianglesVertices();

    for (size_t k = 0; k < elements.size(); ++k)
    {
        auto& tri = elements[k];

        auto c = std::make_shared<PbdAreaConstraint>();
        c->initConstraint(*this, tri[0], tri[1], tri[2], stiffness);
        m_constraints.push_back(c);
    }
    return true;
}

bool
PbdModel::initializeDihedralConstraints(const double& stiffness)
{
    if (m_mesh->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(WARNING) << "Dihedral constraint should come with a triangular mesh";
        return false;
    }

    // Create constraints
    auto triMesh = std::static_pointer_cast<SurfaceMesh>(m_mesh);
    auto elements = triMesh->getTrianglesVertices();
    // following algorithm is terrible, should use half-edge instead
    std::vector<std::vector<size_t>> onering(triMesh->getNumVertices());

    for (size_t k = 0; k < elements.size(); ++k)
    {
        auto& tri = elements[k];
        onering[tri[0]].push_back(k);
        onering[tri[1]].push_back(k);
        onering[tri[2]].push_back(k);
    }

    std::vector<std::vector<bool>> E(triMesh->getNumVertices(), std::vector<bool>(triMesh->getNumVertices(), 1));
    for (size_t k = 0; k < elements.size(); ++k)
    {
        auto& tri = elements[k];

        auto& r1 = onering[tri[0]];
        auto& r2 = onering[tri[1]];
        auto& r3 = onering[tri[2]];

        std::sort(r1.begin(), r1.end());
        std::sort(r2.begin(), r2.end());
        std::sort(r3.begin(), r3.end());

        std::vector<unsigned int> rs;
        std::vector<unsigned int>::iterator it;
        // check if processed or not
        if (E[tri[0]][tri[1]] && E[tri[1]][tri[0]])
        {
            rs.resize(2);
            it = std::set_intersection(r1.begin(), r1.end(), r2.begin(), r2.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1)
            {
                int idx = (rs[0] == k)?1:0;
                SurfaceMesh::TriangleArray& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i)
                {
                    if (t[i] != tri[0] && t[i] != tri[1])
                    {
                        idx = i;
                        break;
                    }
                }
                auto c = std::make_shared<PbdDihedralConstraint>();
                c->initConstraint(*this, tri[2], t[idx], tri[0], tri[1], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[0]][tri[1]] = 0;
        }

        if (E[tri[1]][tri[2]] && E[tri[2]][tri[1]])
        {
            rs.resize(2);
            it = std::set_intersection(r2.begin(), r2.end(), r3.begin(), r3.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1)
            {
                int idx = (rs[0] == k)?1:0;
                auto& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i)
                {
                    if (t[i] != tri[1] && t[i] != tri[2])
                    {
                        idx = i;
                        break;
                    }
                }

                auto c = std::make_shared<PbdDihedralConstraint>();
                c->initConstraint(*this, tri[0], t[idx], tri[1], tri[2], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[1]][tri[2]] = 0;
        }

        if (E[tri[2]][tri[0]] && E[tri[0]][tri[2]])
        {
            rs.resize(2);
            it = std::set_intersection(r3.begin(), r3.end(), r1.begin(), r1.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1)
            {
                int idx = (rs[0] == k)?1:0;
                auto& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i)
                {
                    if (t[i] != tri[2] && t[i] != tri[0])
                    {
                        idx = i;
                        break;
                    }
                }

                auto c = std::make_shared<PbdDihedralConstraint>();
                c->initConstraint(*this, tri[1], t[idx], tri[2], tri[0], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[2]][tri[0]] = 0;
        }
    }
    return true;
}

void
PbdModel::projectConstraints()
{
    unsigned int i = 0;
    while (++i < m_maxIter)
    {
        for (auto c: m_constraints)
        {
            c->solvePositionConstraint(*this);
        }
    }
}

void
PbdModel::updatePhysicsGeometry()
{
    m_mesh->setVerticesPositions(m_currentState->getPositions());
}

void
PbdModel::updatePbdStateFromPhysicsGeometry()
{
    m_currentState->setPositions(m_mesh->getVertexPositions());
}

void
PbdModel::setUniformMass(const double& val)
{
    if (val != 0.0)
    {
        std::fill(m_mass.begin(), m_mass.end(), val);
        std::fill(m_invMass.begin(), m_invMass.end(), 1 / val);
    }
    else
    {
        std::fill(m_invMass.begin(), m_invMass.end(), 0.0);
        std::fill(m_mass.begin(), m_mass.end(), 0.0);
    }
}

void
PbdModel::setParticleMass(const double& val, const size_t& idx)
{
    if (idx < m_mesh->getNumVertices())
    {
        m_mass[idx] = val;
        m_invMass[idx] = 1 / val;
    }
}

void
PbdModel::setFixedPoint(const size_t& idx)
{
    if (idx < m_mesh->getNumVertices())
    {
        m_invMass[idx] = 0;
    }
}

double
PbdModel::getInvMass(const size_t& idx) const
{
    return m_invMass[idx];
}

void
PbdModel::integratePosition()
{
    auto& prevPos = m_previousState->getPositions();
    auto& pos = m_currentState->getPositions();
    auto& vel = m_currentState->getVelocities();
    auto& accn = m_currentState->getAccelerations();

    for (size_t i = 0; i < m_mesh->getNumVertices(); ++i)
    {
        if (m_invMass[i] != 0.0)
        {
            vel[i] += (accn[i] + m_gravity)*m_dt;
            prevPos[i] = pos[i];
            pos[i] += vel[i] * m_dt;
        }
    }
}

void
PbdModel::integrateVelocity()
{
    auto& prevPos = m_previousState->getPositions();
    auto& pos = m_currentState->getPositions();
    auto& vel = m_currentState->getVelocities();

    for (size_t i = 0; i < m_mesh->getNumVertices(); ++i)
    {
        if (m_invMass[i] != 0.0)
        {
            vel[i] = (pos[i] - prevPos[i]) / m_dt;
        }
    }
}

} // imstk
