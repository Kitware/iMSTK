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

#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
MeshToMeshBruteForceCD::MeshToMeshBruteForceCD(std::shared_ptr<Geometry>      obj1,
                                               std::shared_ptr<SurfaceMesh>   obj2,
                                               std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::MeshToMeshBruteForce, colData),
    m_object1(obj1),
    m_object2(obj2)
{
}

// The above error is caused by the
void
MeshToMeshBruteForceCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Broad phase collision
    doBroadPhaseCollisionCheck();

    // Narrow phase collision
    const auto                     mesh2         = std::dynamic_pointer_cast<SurfaceMesh>(m_object2);
    const VecDataArray<int, 3>&    mesh2Cells    = *mesh2->getTriangleIndices();
    const VecDataArray<double, 3>& mesh2Vertices = *mesh2->getVertexPositions();

    if (m_object1->getTypeName() == "LineMesh")
    {
        auto                           lineMesh      = std::dynamic_pointer_cast<LineMesh>(m_object1);
        const VecDataArray<double, 3>& mesh1Vertices = *lineMesh->getVertexPositions();

        // brute force, use BVH or spatial grid would be much better
        for (int i = 0; i < static_cast<int>(lineMesh->getNumVertices()); ++i)
        {
            const Vec3d p = mesh1Vertices[i];

            for (int j = 0; j < mesh2Cells.size(); ++j)
            {
                const Vec3i& e  = mesh2Cells[j];
                const Vec3d  p0 = mesh2Vertices[e[0]];
                const Vec3d  p1 = mesh2Vertices[e[1]];
                const Vec3d  p2 = mesh2Vertices[e[2]];

                if (CollisionUtils::testPointToTriAABB(p[0], p[1], p[2],
                                       p0[0], p0[1], p0[2],
                                       p1[0], p1[1], p1[2],
                                       p2[0], p2[1], p2[2],
                                       m_proximityTolerance,
                                       m_proximityTolerance))
                {
                    m_colData->VTColData.safeAppend({ static_cast<uint32_t>(i), static_cast<uint32_t>(j), 0.0 });
                }
            }
        }

        const int                      numLines    = static_cast<int>(lineMesh->getNumLines());
        const int                      numVertices = static_cast<int>(mesh2->getNumVertices());
        std::vector<std::vector<bool>> E2(numVertices, std::vector<bool>(numVertices, 1));

        for (int k = 0; k < numLines; ++k)
        {
            const Vec2i& nodes = lineMesh->getLineIndices(k);
            const size_t i1    = nodes[0];
            const size_t i2    = nodes[1];

            const Vec3d P = mesh1Vertices[i1];
            const Vec3d Q = mesh1Vertices[i2];

            for (int j = 0; j < mesh2Cells.size(); ++j)
            {
                const Vec3i& e  = mesh2Cells[j];
                const Vec3d  p0 = mesh2Vertices[e[0]];
                const Vec3d  p1 = mesh2Vertices[e[1]];
                const Vec3d  p2 = mesh2Vertices[e[2]];

                if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                {
                    if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p0[0], p0[1], p0[2],
                        p1[0], p1[1], p1[2], m_proximityTolerance, m_proximityTolerance))
                    {
                        m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[0], (uint32_t)e[1] }, 0.0 });
                    }
                    E2[e[0]][e[1]] = 0;
                }

                if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                {
                    if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p1[0], p1[1], p1[2],
                        p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                    {
                        m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[1], (uint32_t)e[2] }, 0.0 });
                    }
                    E2[e[1]][e[2]] = 0;
                }

                if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                {
                    if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                        Q[0], Q[1], Q[2],
                        p2[0], p2[1], p2[2],
                        p0[0], p0[1], p0[2], m_proximityTolerance, m_proximityTolerance))
                    {
                        m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[2], (uint32_t)e[0] }, 0.0 });
                    }
                    E2[e[2]][e[0]] = 0;
                }
            }
        }
    }
    else if (m_object1->getTypeName() == "PointSet")
    {
        auto                           pointSet      = std::dynamic_pointer_cast<PointSet>(m_object1);
        const VecDataArray<double, 3>& mesh1Vertices = *pointSet->getVertexPositions();

        // brute force, use BVH or spatial grid would be much better
        // point
        for (int i = 0; i < static_cast<int>(pointSet->getNumVertices()); ++i)
        {
            const auto p = mesh1Vertices[i];

            for (int j = 0; j < mesh2Cells.size(); ++j)
            {
                const Vec3i& e  = mesh2Cells[j];
                const Vec3d  p0 = mesh2Vertices[e[0]];
                const Vec3d  p1 = mesh2Vertices[e[1]];
                const Vec3d  p2 = mesh2Vertices[e[2]];

                if (CollisionUtils::testPointToTriAABB(p[0], p[1], p[2],
                    p0[0], p0[1], p0[2],
                    p1[0], p1[1], p1[2],
                    p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                {
                    m_colData->VTColData.safeAppend({ static_cast<uint32_t>(i), static_cast<uint32_t>(j), 0.0 });
                }
            }
        }
    }
    else if (m_object1->getTypeName() == "SurfaceMesh")
    {
        auto                           surfMesh      = std::dynamic_pointer_cast<SurfaceMesh>(m_object1);
        const VecDataArray<double, 3>& mesh1Vertices = *surfMesh->getVertexPositions();
        const VecDataArray<int, 3>&    mesh1Cells    = *surfMesh->getTriangleIndices();

        // brute force, use BVH or spatial grid would be much better
        // point
        for (int i = 0; i < static_cast<int>(surfMesh->getNumVertices()); ++i)
        {
            const Vec3d p = mesh1Vertices[i];

            for (int j = 0; j < mesh2Cells.size(); ++j)
            {
                const Vec3i& e  = mesh2Cells[j];
                const Vec3d  p0 = mesh2Vertices[e[0]];
                const Vec3d  p1 = mesh2Vertices[e[1]];
                const Vec3d  p2 = mesh2Vertices[e[2]];

                if (CollisionUtils::testPointToTriAABB(p[0], p[1], p[2],
                    p0[0], p0[1], p0[2],
                    p1[0], p1[1], p1[2],
                    p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                {
                    m_colData->VTColData.safeAppend({ static_cast<uint32_t>(i), static_cast<uint32_t>(j), 0.0 });
                }
            }
        }

        // edge
        // since we don't have edge structure, the following is not good
        const auto                     nV = surfMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));

        const auto                     nV2 = mesh2->getNumVertices();
        std::vector<std::vector<bool>> E2(nV2, std::vector<bool>(nV2, 1));

        for (int k = 0; k < mesh1Cells.size(); ++k)
        {
            const Vec3i& tri = mesh1Cells[k];

            int i1 = tri[0];
            int i2 = tri[1];

            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = mesh1Vertices[i1];
                const Vec3d Q = mesh1Vertices[i2];
                for (int j = 0; j < mesh2Cells.size(); ++j)
                {
                    const Vec3i& e  = mesh2Cells[j];
                    const Vec3d  p0 = mesh2Vertices[e[0]];
                    const Vec3d  p1 = mesh2Vertices[e[1]];
                    const Vec3d  p2 = mesh2Vertices[e[2]];

                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[0], (uint32_t)e[1] }, 0.0 });
                            E2[e[0]][e[1]] = 0;
                        }
                    }

                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[1], (uint32_t)e[2] }, 0.0 });
                            E2[e[1]][e[2]] = 0;
                        }
                    }

                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[2], (uint32_t)e[0] }, 0.0 });
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }

            i1 = tri[1];
            i2 = tri[2];
            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = mesh1Vertices[i1];
                const Vec3d Q = mesh1Vertices[i2];

                for (int j = 0; j < mesh2Cells.size(); ++j)
                {
                    const Vec3i& e  = mesh2Cells[j];
                    const Vec3d  p0 = mesh2Vertices[e[0]];
                    const Vec3d  p1 = mesh2Vertices[e[1]];
                    const Vec3d  p2 = mesh2Vertices[e[2]];

                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[0], (uint32_t)e[1] }, 0.0 });
                            E2[e[0]][e[1]] = 0;
                        }
                    }

                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[1], (uint32_t)e[2] }, 0.0 });
                            E2[e[1]][e[2]] = 0;
                        }
                    }

                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[2], (uint32_t)e[0] }, 0.0 });
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }

            i1 = tri[2];
            i2 = tri[0];
            if (E[i1][i2] && E[i2][i1])
            {
                const Vec3d P = surfMesh->getVertexPosition(i1);
                const Vec3d Q = surfMesh->getVertexPosition(i2);
                for (int j = 0; j < mesh2Cells.size(); ++j)
                {
                    const Vec3i& e  = mesh2Cells[j];
                    const Vec3d  p0 = mesh2Vertices[e[0]];
                    const Vec3d  p1 = mesh2Vertices[e[1]];
                    const Vec3d  p2 = mesh2Vertices[e[2]];

                    if (E2[e[0]][e[1]] && E2[e[1]][e[0]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p0[0], p0[1], p0[2],
                            p1[0], p1[1], p1[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[0], (uint32_t)e[1] }, 0.0 });
                            E2[e[0]][e[1]] = 0;
                        }
                    }

                    if (E2[e[1]][e[2]] && E2[e[2]][e[1]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p1[0], p1[1], p1[2],
                            p2[0], p2[1], p2[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[1], (uint32_t)e[2] }, 0.0 });
                            E2[e[1]][e[2]] = 0;
                        }
                    }

                    if (E2[e[2]][e[0]] && E2[e[0]][e[2]])
                    {
                        if (CollisionUtils::testLineToLineAABB(P[0], P[1], P[2],
                            Q[0], Q[1], Q[2],
                            p2[0], p2[1], p2[2],
                            p0[0], p0[1], p0[2], m_proximityTolerance, m_proximityTolerance))
                        {
                            m_colData->EEColData.safeAppend({ { (uint32_t)i1, (uint32_t)i2 }, { (uint32_t)e[2], (uint32_t)e[0] }, 0.0 });
                            E2[e[2]][e[0]] = 0;
                        }
                    }
                }
                E[i1][i2] = 0;
            }
        }
    }
}

bool
MeshToMeshBruteForceCD::doBroadPhaseCollisionCheck() const
{
    const auto mesh1 = std::static_pointer_cast<PointSet>(m_object1);
    const auto mesh2 = std::static_pointer_cast<PointSet>(m_object2);

    Vec3d min1, max1;
    mesh1->computeBoundingBox(min1, max1);

    Vec3d min2, max2;
    mesh2->computeBoundingBox(min2, max2);

    return CollisionUtils::testAABBToAABB(min1[0] - m_proximityTolerance,
                          max1[0] + m_proximityTolerance,
                          min1[1] - m_proximityTolerance,
                          max1[1] + m_proximityTolerance,
                          min1[2] - m_proximityTolerance,
                          max1[2] + m_proximityTolerance,
                          min2[0] - m_proximityTolerance,
                          max2[0] + m_proximityTolerance,
                          min2[1] - m_proximityTolerance,
                          max2[1] + m_proximityTolerance,
                          min2[2] - m_proximityTolerance,
                          max2[2] + m_proximityTolerance);
}
} // imstk
