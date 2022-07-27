/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTriangleToTetMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
TriangleToTetMap::TriangleToTetMap()
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);
}

TriangleToTetMap::TriangleToTetMap(
    std::shared_ptr<Geometry> parent,
    std::shared_ptr<Geometry> child)
{
    setParentGeometry(parent);
    setChildGeometry(child);

    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);
}

void
TriangleToTetMap::compute()
{
    PointwiseMap::compute();

    m_triToTetMap.clear();
    computeTriToTetMap(m_triToTetMap);
}

void
TriangleToTetMap::computeTriToTetMap(std::unordered_map<int, int>& triToTetMap)
{
    triToTetMap.clear();

    const std::array<Vec3i, 4> facePattern = {
        Vec3i(0, 1, 2), Vec3i(0, 1, 3), Vec3i(0, 2, 3), Vec3i(1, 2, 3)
    };

    auto tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(getParentGeometry());
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(getChildGeometry());

    std::shared_ptr<VecDataArray<int, 4>> tetIndicesPtr = tetMesh->getCells();
    const VecDataArray<int, 4>&           tetIndices    = *tetIndicesPtr;

    std::shared_ptr<VecDataArray<int, 3>> surfIndicesPtr = surfMesh->getCells();
    const VecDataArray<int, 3>&           surfIndices    = *surfIndicesPtr;

    // Hash all the triangles from the surface
    std::unordered_map<TriCell, int> triToFaceId;
    for (int i = 0; i < surfIndices.size(); i++)
    {
        const Vec3i& tri = surfIndices[i];

        // Find the corresponding parent id
        // These are the vertex ids of the tet mesh
        const int id0 = getParentVertexId(tri[0]);
        const int id1 = getParentVertexId(tri[1]);
        const int id2 = getParentVertexId(tri[2]);

        // Hash the triangle with the tet mesh ids
        TriCell cell_a(id0, id1, id2);

        triToFaceId[cell_a] = i;
    }

    // Hash all the triangles from the tetrahedron faces. Take note of
    // collisions
    // Find the corresponding tet
    for (int i = 0; i < tetIndices.size(); i++)
    {
        const Vec4i& tet = tetIndices[i];

        // For every face of the tet
        for (int k = 0; k < 4; k++)
        {
            TriCell cell_b(
                tet[facePattern[k][0]],
                tet[facePattern[k][1]],
                tet[facePattern[k][2]]);

            // If this tet face exists in the map of hashed triangles
            auto iter = triToFaceId.find(cell_b);
            if (iter != triToFaceId.end())
            {
                const int triId = iter->second;
                triToTetMap[triId] = i; // Map tri to tet id
            }
        }
    }
}

int
TriangleToTetMap::getParentTetId(const int triId) const
{
    auto citer = m_triToTetMap.find(triId);
    return (citer != m_triToTetMap.end()) ? citer->second : -1;
}
} // namespace imstk