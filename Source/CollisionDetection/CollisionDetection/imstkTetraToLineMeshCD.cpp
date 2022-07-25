/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTetraToLineMeshCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkParallelUtils.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
TetraToLineMeshCD::TetraToLineMeshCD()
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<LineMesh>(1);
}

void
TetraToLineMeshCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<TetrahedralMesh> tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(geomA);
    std::shared_ptr<LineMesh>        lineMesh = std::dynamic_pointer_cast<LineMesh>(geomB);

    std::shared_ptr<VecDataArray<int, 4>>    tetsPtr = tetMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> tetVerticesPtr = tetMesh->getVertexPositions();
    const VecDataArray<int, 4>&              tets     = *tetsPtr;
    const VecDataArray<double, 3>&           tetVerts = *tetVerticesPtr;

    std::shared_ptr<VecDataArray<int, 2>>    linesPtr    = lineMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = lineMesh->getVertexPositions();
    const VecDataArray<int, 2>&              lines       = *linesPtr;
    const VecDataArray<double, 3>&           lineVerts   = *verticesPtr;

    // Brute force
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(lines.size(), [&](int i)
        {
            const Vec3d& x0 = lineVerts[lines[i][0]];
            const Vec3d& x1 = lineVerts[lines[i][1]];
            for (int j = 0; j < tets.size(); j++)
            {
                std::array<Vec3d, 4> tet;
                tet[0] = tetVerts[tets[j][0]];
                tet[1] = tetVerts[tets[j][1]];
                tet[2] = tetVerts[tets[j][2]];
                tet[3] = tetVerts[tets[j][3]];
                if (CollisionUtils::testTetToSegment(tet, x0, x1))
                {
                    CellIndexElement elemA;
                    elemA.ids[0]   = j;
                    elemA.idCount  = 1;
                    elemA.cellType = IMSTK_TETRAHEDRON;

                    CellIndexElement elemB;
                    elemB.ids[0]   = i;
                    elemB.idCount  = 1;
                    elemB.cellType = IMSTK_EDGE;

                    lock.lock();
                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                    lock.unlock();
                }
            }
        });
}
} // namespace imstk