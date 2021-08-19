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

#include "imstkTetraToLineMeshCD.h"
#include "imstkCollisionData.h"
#include "imstkTetrahedralMesh.h"
#include "imstkLineMesh.h"
#include "imstkCollisionUtils.h"

namespace imstk
{
TetraToLineMeshCD::TetraToLineMeshCD()
{
    setRequiredInputType<TetrahedralMesh>(0);
    setRequiredInputType<LineMesh>(1);
}

void
TetraToLineMeshCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<TetrahedralMesh> tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(geomA);
    std::shared_ptr<LineMesh>        lineMesh = std::dynamic_pointer_cast<LineMesh>(geomB);

    std::shared_ptr<VecDataArray<int, 4>>    tetsPtr = tetMesh->getTetrahedraIndices();
    std::shared_ptr<VecDataArray<double, 3>> tetVerticesPtr = tetMesh->getVertexPositions();
    const VecDataArray<int, 4>&              tets     = *tetsPtr;
    const VecDataArray<double, 3>&           tetVerts = *tetVerticesPtr;

    std::shared_ptr<VecDataArray<int, 2>>    linesPtr    = lineMesh->getLinesIndices();
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = lineMesh->getVertexPositions();
    const VecDataArray<int, 2>&              lines       = *linesPtr;
    const VecDataArray<double, 3>&           lineVerts   = *verticesPtr;

    // Brute force
    std::array<Vec3d, 4> tet;
    for (int i = 0; i < lines.size(); i++)
    {
        const Vec3d& x0 = lineVerts[lines[i][0]];
        const Vec3d& x1 = lineVerts[lines[i][1]];
        for (int j = 0; j < tets.size(); j++)
        {
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

                elementsA.safeAppend(elemA);
                elementsB.safeAppend(elemB);
            }
        }
    }
}
}