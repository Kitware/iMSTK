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

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkParallelUtils.h"

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
    // Clear collisionData
    m_colData->clearAll();

    // Get plane properties
    auto planePos    = m_plane->getPosition();
    auto planeNormal = m_plane->getNormal();

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(m_pointSet->getVertexPositions().size(),
        [&](const size_t idx)
        {
            const auto p = m_pointSet->getVertexPosition(idx);
            const auto penetrationDist = (p - planePos).dot(planeNormal);

            if (penetrationDist <= 0.0)
            {
                const auto penetrationDir = planeNormal * penetrationDist;
                lock.lock();
                m_colData->MAColData.push_back({ idx, penetrationDir });
                lock.unlock();
            }
        });
}
} // imstk
