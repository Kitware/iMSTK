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
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
void
TetraTriangleMap::compute()
{
    CHECK(m_master && m_slave) << "TetraTriangle map is being applied without valid geometries";

    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    auto triMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_slave);

    CHECK(tetMesh && triMesh) << "Fail to cast from geometry to meshes";

    m_verticesEnclosingTetraId.clear();
    m_verticesWeights.clear();
    m_verticesEnclosingTetraId.resize(triMesh->getNumVertices());
    m_verticesWeights.resize(triMesh->getNumVertices());
    m_slaveVerts = triMesh->getVertexPositions();
    bool bValid = true;

    if (!m_boundingBoxAvailable)
    {
        // calling this function inside findEnclosingTetrahedron is not thread-safe.
        updateBoundingBox();
    }

    ParallelUtils::parallelFor(triMesh->getNumVertices(), [&](const size_t vertexIdx) {
            if (!bValid) // If map is invalid, no need to check further
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
            TetrahedralMesh::WeightsArray weights = { 0.0, 0.0, 0.0, 0.0 };
            tetMesh->computeBarycentricWeights(closestTetId, surfVertPos, weights);

            m_verticesEnclosingTetraId[vertexIdx] = closestTetId; // store nearest tetrahedron
            m_verticesWeights[vertexIdx] = weights;               // store weights
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
    CHECK(m_master && m_slave) << "TetraTriangle map is being applied without valid  geometries";

    auto tetMesh = static_cast<TetrahedralMesh*>(m_master.get());
    auto triMesh = static_cast<SurfaceMesh*>(m_slave.get());

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    CHECK(dynamic_cast<TetrahedralMesh*>(m_master.get()) && dynamic_cast<SurfaceMesh*>(m_slave.get()))
        << "Fail to cast from geometry to meshes";
#endif

    VecDataArray<double, 3>& vertices = *m_slaveVerts;
    ParallelUtils::parallelFor(triMesh->getNumVertices(), [&](const size_t vertexId) {
            const Vec4i& tetVerts = tetMesh->getTetrahedronIndices(m_verticesEnclosingTetraId[vertexId]);
            const auto& weights   = m_verticesWeights[vertexId];

            vertices[vertexId] = tetMesh->getVertexPosition(tetVerts[0]) * weights[0] +
                                 tetMesh->getVertexPosition(tetVerts[1]) * weights[1] +
                                 tetMesh->getVertexPosition(tetVerts[2]) * weights[2] +
                                 tetMesh->getVertexPosition(tetVerts[3]) * weights[3];
        });

    triMesh->postModified();
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
    CHECK(dynamic_cast<TetrahedralMesh*>(m_master.get())) << "Fail to cast from geometry to mesh";
#endif

    auto totalElementsMaster = meshMaster->getNumTetrahedra();
    bool bOK = true;

    ParallelUtils::parallelFor(m_verticesEnclosingTetraId.size(), [&](const size_t tetId) {
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
    CHECK(master->getType() == Geometry::Type::TetrahedralMesh) <<
        "The geometry provided as master is not of tetrahedral type";

    GeometryMap::setMaster(master);
}

void
TetraTriangleMap::setSlave(std::shared_ptr<Geometry> slave)
{
    CHECK(slave->getType() == Geometry::Type::SurfaceMesh) <<
        "The geometry provided as slave is not of triangular type (surface)";

    GeometryMap::setSlave(slave);
}

size_t
TetraTriangleMap::findClosestTetrahedron(const Vec3d& pos) const
{
    const auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    double     closestDistanceSqr = MAX_D;
    size_t     closestTetrahedron = std::numeric_limits<size_t>::max();
    Vec3d      center(0, 0, 0);

    for (size_t tetId = 0; tetId < tetMesh->getNumTetrahedra(); ++tetId)
    {
        center = Vec3d::Zero();
        const Vec4i& vert = tetMesh->getTetrahedronIndices(tetId);
        for (size_t i = 0; i < 4; ++i)
        {
            center += tetMesh->getInitialVertexPosition(vert[i]);
        }
        center = center / 4.;

        const double distSqr = (pos - center).squaredNorm();
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
    const auto                    tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    TetrahedralMesh::WeightsArray weights = { 0.0, 0.0, 0.0, 0.0 };
    size_t                        enclosingTetrahedron = std::numeric_limits<size_t>::max();

    for (size_t idx = 0; idx < tetMesh->getNumTetrahedra(); ++idx)
    {
        bool inBox = (pos[0] >= m_bBoxMin[idx][0] && pos[0] <= m_bBoxMax[idx][0])
                     && (pos[1] >= m_bBoxMin[idx][1] && pos[1] <= m_bBoxMax[idx][1])
                     && (pos[2] >= m_bBoxMin[idx][2] && pos[2] <= m_bBoxMax[idx][2]);

        // If the point is outside the bounding box, it is for sure not inside
        // the element
        if (!inBox)
        {
            continue;
        }

        tetMesh->computeBarycentricWeights(idx, pos, weights);

        if (weights[0] >= 0 && weights[1] >= 0 && weights[2] >= 0 && weights[3] >= 0)
        {
            enclosingTetrahedron = idx;
            break;
        }
    }

    return enclosingTetrahedron;
}

void
TetraTriangleMap::updateBoundingBox()
{
    /// \todo use parallelFor
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_master);
    m_bBoxMin.resize(tetMesh->getNumTetrahedra());
    m_bBoxMax.resize(tetMesh->getNumTetrahedra());

    ParallelUtils::parallelFor(tetMesh->getNumTetrahedra(), [&](const size_t tid) {
            tetMesh->computeTetrahedronBoundingBox(tid, m_bBoxMin[tid], m_bBoxMax[tid]);
        });

    m_boundingBoxAvailable = true;
}
}  // namespace imstk
