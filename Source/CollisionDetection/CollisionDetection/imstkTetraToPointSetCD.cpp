/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version B.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-B.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkTetraToPointSetCD.h"
#include "imstkCollisionData.h"
#include "imstkParallelUtils.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
TetraToPointSetCD::TetraToPointSetCD()
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<PointSet>(1);
}

void
TetraToPointSetCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<TetrahedralMesh> tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(geomA);
    std::shared_ptr<PointSet>        pointSet = std::dynamic_pointer_cast<PointSet>(geomB);

    m_hashTableB.clear();
    m_hashTableB.insertPoints(*pointSet->getVertexPositions());

    constexpr const double eps  = IMSTK_DOUBLE_EPS;
    const double           eps2 = 1.0e-10;

    //const VecDataArray<double, 3>& verticesMeshA = *m_tetMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> verticesMeshBPtr = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           verticesMeshB    = *verticesMeshBPtr;

    // For every tet in meshA, test if any points lie in it
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(tetMesh->getNumTetrahedra(),
        [&](const int tetIdA)
        {
            // Compute the bounding box of the tet
            Vec3d min, max;
            tetMesh->computeTetrahedronBoundingBox(tetIdA, min, max);

            // For every other point in near the bounding box
            std::vector<size_t> meshBVertexIds = m_hashTableB.getPointsInAABB(min, max);
            for (size_t vertexIdB : meshBVertexIds)
            {
                Vec3d vPos = verticesMeshB[vertexIdB];

                // Now compute if the point is actually within the tet
                const Vec4d bCoord = tetMesh->computeBarycentricWeights(tetIdA, vPos);
                if (bCoord[0] >= -eps
                    && bCoord[1] >= -eps
                    && bCoord[2] >= -eps
                    && bCoord[3] >= -eps)
                {
                    const double coordSum = bCoord[0] + bCoord[1] + bCoord[2] + bCoord[3];
                    assert(coordSum <= 1.0 + eps2 && coordSum >= 1.0 - eps2);

                    CellIndexElement elemA;
                    elemA.ids[0]   = tetIdA;
                    elemA.idCount  = 1;
                    elemA.cellType = IMSTK_TETRAHEDRON;

                    CellIndexElement elemB;
                    elemB.ids[0]   = static_cast<int>(vertexIdB);
                    elemB.idCount  = 1;
                    elemB.cellType = IMSTK_VERTEX;

                    lock.lock();
                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                    lock.unlock();
                }
            }
        });
}
}