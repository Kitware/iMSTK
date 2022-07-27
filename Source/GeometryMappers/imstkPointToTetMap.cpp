/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointToTetMap.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointToTetMap::PointToTetMap() : m_boundingBoxAvailable(false)
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<PointSet>(1);
}

PointToTetMap::PointToTetMap(
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
PointToTetMap::compute()
{
    if (!areInputsValid())
    {
        LOG(WARNING) << "PointToTetMap failed to run, inputs not satisfied";
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
PointToTetMap::requestUpdate()
{
    auto tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    auto pointSet = std::dynamic_pointer_cast<PointSet>(getChildGeometry());

    std::shared_ptr<VecDataArray<int, 4>>    parentIndicesPtr = tetMesh->getCells();
    const VecDataArray<int, 4>&              parentIndices    = *parentIndicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> parentVertsPtr   = tetMesh->getVertexPositions();
    const VecDataArray<double, 3>&           parentVerts      = *parentVertsPtr;

    VecDataArray<double, 3>& childVerts = *m_childVerts;

    ParallelUtils::parallelFor(pointSet->getNumVertices(),
        [&](const int vertexId)
        {
            const Vec4i& tet =
                parentIndices[m_verticesEnclosingTetraId[vertexId]];
            const Vec4d& weights = m_verticesWeights[vertexId];

            childVerts[vertexId] = parentVerts[tet[0]] * weights[0] +
                                   parentVerts[tet[1]] * weights[1] +
                                   parentVerts[tet[2]] * weights[2] +
                                   parentVerts[tet[3]] * weights[3];
        });

    pointSet->postModified();

    setOutput(pointSet);
}

int
PointToTetMap::findClosestTetrahedron(const Vec3d& pos) const
{
    auto   tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    double closestDistanceSqr = IMSTK_DOUBLE_MAX;
    int    closestTetrahedron = IMSTK_INT_MAX;
    Vec3d  center(0.0, 0.0, 0.0);

    for (int tetId = 0; tetId < tetMesh->getNumCells(); tetId++)
    {
        center = Vec3d::Zero();
        const Vec4i& vert = (*tetMesh->getCells())[tetId];
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
PointToTetMap::findEnclosingTetrahedron(const Vec3d& pos) const
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    int  enclosingTetrahedron = IMSTK_INT_MAX;

    for (int idx = 0; idx < tetMesh->getNumCells(); idx++)
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
PointToTetMap::updateBoundingBox()
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    m_bBoxMin.resize(tetMesh->getNumCells());
    m_bBoxMax.resize(tetMesh->getNumCells());

    ParallelUtils::parallelFor(tetMesh->getNumCells(),
        [&](const int tid)
        {
            tetMesh->computeTetrahedronBoundingBox(tid, m_bBoxMin[tid], m_bBoxMax[tid]);
        });

    m_boundingBoxAvailable = true;
}
} // namespace imstk