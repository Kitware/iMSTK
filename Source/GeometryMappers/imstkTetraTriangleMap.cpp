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
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
TetraTriangleMap::TetraTriangleMap() : m_boundingBoxAvailable(false)
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<PointSet>(1);
}
TetraTriangleMap::TetraTriangleMap(
    std::shared_ptr<Geometry> parent,
    std::shared_ptr<Geometry> child)
    : m_boundingBoxAvailable(false)
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<PointSet>(1);

    setParentGeometry(parent);
    setChildGeometry(child);
}

void
TetraTriangleMap::compute()
{
    if (!areInputsValid())
    {
        LOG(WARNING) << "TetraTriangleMap failed to run, inputs not satisfied";
        return;
    }
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    auto triMesh = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    m_verticesEnclosingTetraId.clear();
    m_verticesWeights.clear();
    m_verticesEnclosingTetraId.resize(triMesh->getNumVertices());
    m_verticesWeights.resize(triMesh->getNumVertices());
    m_childVerts = triMesh->getVertexPositions();
    bool bValid = true;

    if (!m_boundingBoxAvailable)
    {
        // calling this function inside findEnclosingTetrahedron is not thread-safe.
        updateBoundingBox();
    }

    ParallelUtils::parallelFor(triMesh->getNumVertices(),
        [&](const int vertexIdx)
        {
            if (!bValid) // If map is invalid, no need to check further
            {
                return;
            }
            const Vec3d& surfVertPos = triMesh->getVertexPosition(vertexIdx);

            // Find the enclosing or closest tetrahedron
            int closestTetId = findEnclosingTetrahedron(surfVertPos);
            if (closestTetId == IMSTK_INT_MAX)
            {
                closestTetId = findClosestTetrahedron(surfVertPos);
            }
            if (closestTetId == IMSTK_INT_MAX)
            {
                LOG(WARNING) << "Could not find closest tetrahedron";
                bValid = false;
                return;
            }

            // Compute the weights
            const Vec4d weights = tetMesh->computeBarycentricWeights(closestTetId, surfVertPos);

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
TetraTriangleMap::requestUpdate()
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    auto pointSet = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    VecDataArray<double, 3>& vertices = *m_childVerts;
    ParallelUtils::parallelFor(pointSet->getNumVertices(),
        [&](const int vertexId)
        {
            const Vec4i& tetVerts =
                tetMesh->getTetrahedronIndices(m_verticesEnclosingTetraId[vertexId]);
            const Vec4d& weights = m_verticesWeights[vertexId];

            vertices[vertexId] = tetMesh->getVertexPosition(tetVerts[0]) * weights[0] +
                                 tetMesh->getVertexPosition(tetVerts[1]) * weights[1] +
                                 tetMesh->getVertexPosition(tetVerts[2]) * weights[2] +
                                 tetMesh->getVertexPosition(tetVerts[3]) * weights[3];
        });

    pointSet->postModified();

    setOutput(pointSet);
}

int
TetraTriangleMap::findClosestTetrahedron(const Vec3d& pos) const
{
    auto   tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    double closestDistanceSqr = IMSTK_DOUBLE_MAX;
    int closestTetrahedron = IMSTK_INT_MAX;
    Vec3d center(0.0, 0.0, 0.0);

    for (int tetId = 0; tetId < tetMesh->getNumTetrahedra(); tetId++)
    {
        center = Vec3d::Zero();
        const Vec4i& vert = tetMesh->getTetrahedronIndices(tetId);
        for (int i = 0; i < 4; i++)
        {
            center += tetMesh->getInitialVertexPosition(vert[i]);
        }
        center = center / 4.0;

        const double distSqr = (pos - center).squaredNorm();
        if (distSqr < closestDistanceSqr)
        {
            closestDistanceSqr = distSqr;
            closestTetrahedron = tetId;
        }
    }

    return closestTetrahedron;
}

int
TetraTriangleMap::findEnclosingTetrahedron(const Vec3d& pos) const
{
    auto   tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    int enclosingTetrahedron = IMSTK_INT_MAX;

    for (int idx = 0; idx < tetMesh->getNumTetrahedra(); idx++)
    {
        const bool inBox = (pos[0] >= m_bBoxMin[idx][0] && pos[0] <= m_bBoxMax[idx][0])
                     && (pos[1] >= m_bBoxMin[idx][1] && pos[1] <= m_bBoxMax[idx][1])
                     && (pos[2] >= m_bBoxMin[idx][2] && pos[2] <= m_bBoxMax[idx][2]);

        // If the point is outside the bounding box, it is for sure not inside
        // the element
        if (!inBox)
        {
            continue;
        }

        const Vec4d weights = tetMesh->computeBarycentricWeights(idx, pos);
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
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    m_bBoxMin.resize(tetMesh->getNumTetrahedra());
    m_bBoxMax.resize(tetMesh->getNumTetrahedra());

    ParallelUtils::parallelFor(tetMesh->getNumTetrahedra(),
        [&](const int tid)
        {
            tetMesh->computeTetrahedronBoundingBox(tid, m_bBoxMin[tid], m_bBoxMax[tid]);
        });

    m_boundingBoxAvailable = true;
}
} // namespace imstk