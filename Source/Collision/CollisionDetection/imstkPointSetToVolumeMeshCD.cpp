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

#include "imstkPointSetToVolumeMeshCD.h"
#include "imstkNarrowPhaseCD.h"
#include "imstkCollisionData.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
PointSetToVolumeMeshCD::PointSetToVolumeMeshCD(std::shared_ptr<PointSet>      pointset,
                                               std::shared_ptr<SurfaceMesh>   triMesh,
                                               std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToVolumeMesh, colData),
    m_pointset(pointset), m_triMesh(triMesh)
{
}

void
PointSetToVolumeMeshCD::computeCollisionData()
{
    m_colData->clearAll();

    // This is brute force collision detection
    // TODO: replace by octree
    ParallelUtils::parallelFor(static_cast<unsigned int>(m_pointset->getVertexPositions().size()),
        [&](const unsigned int idx)
        {
            const auto& point = m_pointset->getVertexPosition(idx);
            for (unsigned int idx2 = 0; idx2 < static_cast<unsigned int>(m_triMesh->getNumVertices()); ++idx2)
            {
                NarrowPhaseCD::pointToTriangle(point, idx, idx2, m_triMesh.get(), m_colData);
            }
       });
}
} // imstk
