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

#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkNarrowPhaseCD.h"
#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
SurfaceMeshToSurfaceMeshCD::SurfaceMeshToSurfaceMeshCD(std::shared_ptr<SurfaceMesh>   meshA,
                                                       std::shared_ptr<SurfaceMesh>   meshB,
                                                       std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::SurfaceMeshToSurfaceMesh, colData),
    m_meshA(meshA), m_meshB(meshB)
{
}

void
SurfaceMeshToSurfaceMeshCD::computeCollisionData()
{
    m_colData->clearAll();

    // This is brute force collision detection
    // TODO: use octree
    ParallelUtils::parallelFor(static_cast<unsigned int>(m_meshA->getNumTriangles()),
        [&](const unsigned int idx1)
        {
            for (unsigned int idx2 = 0; idx2 < static_cast<unsigned int>(m_meshA->getNumTriangles()); ++idx2)
            {
                NarrowPhaseCD::triangleToTriangle(idx1, m_meshA.get(), idx2, m_meshB.get(), m_colData);
            }
       });
}
} // imstk
