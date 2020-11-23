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

#include "imstkPointSetToPlaneCD.h"
#include "imstkCollisionData.h"
#include "imstkNarrowPhaseCD.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSetToPlaneCD::PointSetToPlaneCD(std::shared_ptr<PointSet>      pointSet,
                                     std::shared_ptr<Plane>         plane,
                                     std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToSphere, colData),
    m_pointSet(pointSet),
    m_plane(plane)
{
}

void
PointSetToPlaneCD::computeCollisionData()
{
    m_colData->clearAll();
    std::shared_ptr<VecDataArray<double, 3>> vertexData = m_pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::parallelFor(static_cast<unsigned int>(vertices.size()),
        [&](const unsigned int idx)
        {
            NarrowPhaseCD::pointToPlane(vertices[idx], idx, m_plane.get(), m_colData);
        });
}
} // imstk
