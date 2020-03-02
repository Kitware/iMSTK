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
        LOG(FATAL) << "TetraTriangle map is being applied without valid "
                      "geometries";
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

    ParallelUtils::parallelFor(triMesh->getNumVertices(), [&](const size_t vertexIdx) {
        if (!bValid)  // If map is invalid, no need to check further
        {
            return;
        }
        const Vec3d& surfVertPos = triMesh->getVertexPosition(vertexIdx);

        // Find the enclosing or closest tetrahedron
        size_t closestTetId = findEnclosingTetrahedron(surfVertPos);
        if (closestTetId == std::numeric_limits<size_t>::max())
        {
            closestTetId = findClosestTetrahedron(surfVertPos);
        }
        if (closestTetId == std::numeric_limits<size_t>::max())
        {
            LOG(WARNING) << "Could not find closest tetrahedron";
            bValid = false;
            return;
        }

        // Compute the weights
        TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};
        tetMesh->computeBarycentricWeights(closestTetId, surfVertPos, weights);

        m_verticesEnclosingTetraId[vertexIdx] = closestTetId;  // store nearest tetrahedron
        m_verticesWeights[vertexIdx]          = weights;       // store weights
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
        LOG(FATAL) << "TetraTriangle map is being applied without valid "
                      "geometries";
        return;
    }

    auto tetMesh = static_cast<TetrahedralMesh*>(m_master.get());
    auto triMesh = static_cast<SurfaceMesh*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL,
           (!dynamic_cast<TetrahedralMesh*>(m_master.get()) ||
            !dynamic_cast<SurfaceMesh*>(m_slave.get())))
            << "Fail to cast from geometry to meshes";
#endif

    ParallelUtils::parallelFor(triMesh->getNumVertices(), [&](const size_t vertexId) {
        Vec3d  newPos(0, 0, 0);
        size_t tetId    = m_verticesEnclosingTetraId[vertexId];
        auto   tetVerts = tetMesh->getTetrahedronVertices(tetId);
        auto   weights  = m_verticesWeights[vertexId];
        for (size_t i = 0; i < 4; ++i)
        {
            newPos += tetMesh->getVertexPosition(tetVerts[i]) * weights[i];
        }

        // This writes newPos to position array at individual vertexId,
        // thus should not cause race condition
        triMesh->setVertexPosition(vertexId, newPos);
    });
}

void
TetraTriangleMap::print() const
{
    // Print Type
    GeometryMap::print();

    // Print vertices and weight info
    LOG(INFO) << "Vertex (<vertNum>): Tetrahedra: <TetNum> - Weights: (w1, w2, "
                 "w3, w4)\n";
    for (size_t vertexId = 0; vertexId < m_verticesEnclosingTetraId.size(); ++vertexId)
    {
        LOG(INFO) << "Vertex (" << vertexId << "):"
                  << "\tTetrahedra: " << m_verticesEnclosingTetraId.at(vertexId) << " - Weights: "
                  << "(" << m_verticesWeights.at(vertexId)[0] << ", "
                  << m_verticesWeights.at(vertexId)[1] << ", " << m_verticesWeights.at(vertexId)[2]
                  << ", " << m_verticesWeights.at(vertexId)[3] << ")";
    }
}

bool
TetraTriangleMap::isValid() const
{
    auto meshMaster = static_cast<TetrahedralMesh*>(m_master.get());
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!dynamic_cast<TetrahedralMesh*>(m_master.get())))
            << "Fail to cast from geometry to mesh";
#endif

    auto totalElementsMaster = meshMaster->getNumTetrahedra();
    bool bOK                 = true;

    ParallelUtils::parallelFor(m_verticesEnclosingTetraId.size(), [&](const size_t tetId) {
        if (!bOK)  // If map is invalid, no need to check further
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
        LOG(FATAL) << "The geometry provided as master is not of tetrahedral type";
        return;
    }
    GeometryMap::setMaster(master);
}

void
TetraTriangleMap::setSlave(std::shared_ptr<Geometry> slave)
{
    if (slave->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(FATAL) << "The geometry provided as slave is not of triangular "
                      "type (surface)";
        return;
    }
    GeometryMap::setSlave(slave);
}

size_t
TetraTriangleMap::findClosestTetrahedron(const Vec3d& pos) const
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    double closestDistanceSqr = MAX_D;
    size_t closestTetrahedron = std::numeric_limits<size_t>::max();
    Vec3d  center(0, 0, 0);

    for (size_t tetId = 0; tetId < tetMesh->getNumTetrahedra(); ++tetId)
    {
        center[0] = 0.0;
        center[1] = 0.0;
        center[2] = 0.0;

        auto vert = tetraMesh->getTetrahedronVertices(tetId);
        for (size_t i = 0; i < 4; ++i)
        {
            center += tetraMesh->getInitialVertexPosition(vert[i]);
        }
        center         = center / 4.0;
        double distSqr = (Vec3d(pos - center)).squaredNorm();
        if (distSqr < closestDistanceSqr)
        {
            closestDistanceSqr = distSqr;
            closestTetrahedron = tetId;
        }
    }
    
    return closestTetrahedron;
}

size_t
TetraTriangleMap::findEnclosingTetrahedron(const Vec3d& pos) const
{
    bool  inBox;
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);

    TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};

    size_t enclosingTetrahedron = std::numeric_limits<size_t>::max();

    for (size_t idx = 0; idx < tetMesh->getNumVertices(); ++idx)
    {
        inBox = (pos[0] >= m_bBoxMin[idx][0] && pos[0] <= m_bBoxMax[idx][0]) &&
                (pos[1] >= m_bBoxMin[idx][1] && pos[1] <= m_bBoxMax[idx][1]) &&
                (pos[2] >= m_bBoxMin[idx][2] && pos[2] <= m_bBoxMax[idx][2]);

        // if the point is outside the bounding box, it is for sure not inside
        // the element
        if (!inBox) continue;

        tetMesh->computeBarycentricWeights(idx, pos, weights);

        if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
        {
            enclosingTetrahedron = idx;
            break;
        }
    }

    return enclosingTetrahedron;
}

void
TetraTriangleMap::m_updateBoundingBox(void)
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);

    for (size_t idx = 0; idx < tetMesh->getNumVertices(); ++idx)
    {
        tetMesh->computeTetrahedronBoundingBox(idx, m_bBoxMin[idx], m_bBoxMax[idx]);
    }
    return;
}
}  // namespace imstk
