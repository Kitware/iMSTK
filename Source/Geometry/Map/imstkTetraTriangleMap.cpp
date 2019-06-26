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

#include "imstkTetraTriangleMap.h"
#include "imstkParallelUtils.h"

namespace imstk
{
void
TetraTriangleMap::compute()
{
    if (!m_master || !m_slave)
    {
        LOG(WARNING) << "TetraTriangle map is being applied without valid geometries";
        return;
    }

    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    auto triMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_slave);
    LOG_IF(FATAL, (!tetMesh || !triMesh)) << "Fail to cast from geometry to meshes";

    m_verticesEnclosingTetraId.clear();
    m_verticesWeights.clear();
    m_verticesEnclosingTetraId.resize(triMesh->getNumVertices());
    m_verticesWeights.resize(triMesh->getNumVertices());
    bool bValid = true;

    ParallelUtils::parallelFor(triMesh->getNumVertices(),
        [&](const size_t vertexIdx)
        {
            if (!bValid) // If map is invalid, no need to check further
            {
                return;
            }
            const Vec3d& surfVertPos = triMesh->getVertexPosition(vertexIdx);

            // Find the enclosing or closest tetrahedron
            size_t closestTetId = findEnclosingTetrahedron(tetMesh, surfVertPos);
            if (closestTetId == std::numeric_limits<size_t>::max())
            {
                closestTetId = findClosestTetrahedron(tetMesh, surfVertPos);
            }
            if (closestTetId == std::numeric_limits<size_t>::max())
            {
                LOG(WARNING) << "Could not find closest tetrahedron";
                bValid = false;
                return;
            }

            // Compute the weights
            TetrahedralMesh::WeightsArray weights = { 0.0, 0.0, 0.0, 0.0 };
            tetMesh->computeBarycentricWeights(closestTetId, surfVertPos, weights);

            m_verticesEnclosingTetraId[vertexIdx] = closestTetId; // store nearest tetrahedron
            m_verticesWeights[vertexIdx] = weights; // store weights
    });

    // Clear result if could not find closest tet
    if (!bValid)
    {
        m_verticesEnclosingTetraId.clear();
        m_verticesWeights.clear();
    }
}

void
TetraTriangleMap::apply()
{
    // Check if map is active
    if (!m_isActive)
    {
        LOG(WARNING) << "TetraTriangle map is not active";
        return;
    }

    // Check geometries
    if (!m_master || !m_slave)
    {
        LOG(WARNING) << "TetraTriangle map is being applied without valid geometries";
        return;
    }

    auto tetMesh = static_cast<TetrahedralMesh*>(m_master.get());
    auto triMesh = static_cast<SurfaceMesh*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!dynamic_cast<TetrahedralMesh*>(m_master.get()) ||
                   !dynamic_cast<SurfaceMesh*>(m_slave.get()))) << "Fail to cast from geometry to meshes";
#endif

    ParallelUtils::parallelFor(triMesh->getNumVertices(),
        [&](const size_t vertexId)
        {
            Vec3d newPos(0, 0, 0);
            size_t tetId = m_verticesEnclosingTetraId[vertexId];
            auto tetVerts = tetMesh->getTetrahedronVertices(tetId);
            auto weights = m_verticesWeights[vertexId];
            for (size_t i = 0; i < 4; ++i)
            {
                newPos += tetMesh->getVertexPosition(tetVerts[i]) * weights[i];
            }

            // This writes newPos to position array at individual vertexId, thus should not cause race condition
            triMesh->setVertexPosition(vertexId, newPos);
        });
}

void
TetraTriangleMap::print() const
{
    // Print Type
    GeometryMap::print();

    // Print vertices and weight info
    LOG(INFO) << "Vertex (<vertNum>): Tetrahedra: <TetNum> - Weights: (w1, w2, w3, w4)\n";
    for (size_t vertexId = 0; vertexId < m_verticesEnclosingTetraId.size(); ++vertexId)
    {
        LOG(INFO) << "Vertex (" << vertexId << "):"
                  << "\tTetrahedra: " << m_verticesEnclosingTetraId.at(vertexId)
                  << " - Weights: " << "("
                  << m_verticesWeights.at(vertexId)[0] << ", "
                  << m_verticesWeights.at(vertexId)[1] << ", "
                  << m_verticesWeights.at(vertexId)[2] << ", "
                  << m_verticesWeights.at(vertexId)[3] << ")";
    }
}

bool
TetraTriangleMap::isValid() const
{
    auto meshMaster = static_cast<TetrahedralMesh*>(m_master.get());
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!dynamic_cast<TetrahedralMesh*>(m_master.get()))) << "Fail to cast from geometry to mesh";
#endif

    auto totalElementsMaster = meshMaster->getNumTetrahedra();
    bool bOK = true;

    ParallelUtils::parallelFor(m_verticesEnclosingTetraId.size(),
        [&](const size_t tetId){
            if (!bOK) // If map is invalid, no need to check further
            {
                return;
            }
            if (!(m_verticesEnclosingTetraId[tetId] < totalElementsMaster))
            {
                bOK = false;
            }
        });

    return bOK;
}

void
TetraTriangleMap::setMaster(std::shared_ptr<Geometry> master)
{
    if (master->getType() != Geometry::Type::TetrahedralMesh)
    {
        LOG(WARNING) << "The geometry provided as master is not of tetrahedral type";
        return;
    }
    GeometryMap::setMaster(master);
}

void
TetraTriangleMap::setSlave(std::shared_ptr<Geometry> slave)
{
    if (slave->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(WARNING) << "The geometry provided as slave is not of triangular type (surface)";
        return;
    }
    GeometryMap::setSlave(slave);
}

size_t
TetraTriangleMap::findClosestTetrahedron(std::shared_ptr<TetrahedralMesh> tetraMesh, const Vec3d& pos)
{
    double closestDistanceSqr = MAX_D;
    size_t closestTetrahedron = std::numeric_limits<size_t>::max();
    for (size_t tetId = 0; tetId < tetraMesh->getNumTetrahedra(); ++tetId)
    {
        Vec3d center(0, 0, 0);
        auto vert = tetraMesh->getTetrahedronVertices(tetId);
        for (size_t i = 0; i < 4; ++i)
        {
            center += tetraMesh->getInitialVertexPosition(vert[i]);
        }
        center = center / 4.0;
        double distSqr = (pos - center).squaredNorm();
        if (distSqr < closestDistanceSqr)
        {
            closestDistanceSqr = distSqr;
            closestTetrahedron = tetId;
        }
    }
    return closestTetrahedron;
}

size_t
TetraTriangleMap::findEnclosingTetrahedron(std::shared_ptr<TetrahedralMesh> tetraMesh, const Vec3d& pos)
{
    Vec3d boundingBoxMin;
    Vec3d boundingBoxMax;
    std::vector<size_t> probablesTetId;

    // Eliminate the improbables based in bounding box test
    for (size_t tetId = 0; tetId < tetraMesh->getNumTetrahedra(); ++tetId)
    {
        tetraMesh->computeTetrahedronBoundingBox(tetId, boundingBoxMin, boundingBoxMax);

        if ((pos[0] >= boundingBoxMin[0] && pos[0] <= boundingBoxMax[0]) &&
            (pos[1] >= boundingBoxMin[1] && pos[1] <= boundingBoxMax[1]) &&
            (pos[2] >= boundingBoxMin[2] && pos[2] <= boundingBoxMax[2]))
        {
            probablesTetId.push_back(tetId);
        }
    }

    // Check which probable tetrahedron the point belongs to
    size_t enclosingTetrahedron = std::numeric_limits<size_t>::max();
    TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};
    for (const size_t& tetId : probablesTetId)
    {
        tetraMesh->computeBarycentricWeights(tetId, pos, weights);

        if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
        {
            enclosingTetrahedron = tetId;
            break;
        }
    }
    return enclosingTetrahedron;
}
} // imstk
