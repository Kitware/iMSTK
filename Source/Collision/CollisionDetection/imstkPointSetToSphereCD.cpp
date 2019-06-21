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

#include "imstkPointSetToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkSphere.h"
#include "imstkPointSet.h"
#include "imstkParallelUtils.h"

namespace imstk
{
PointSetToSphereCD::PointSetToSphereCD(std::shared_ptr<PointSet> pointSet,
                                       std::shared_ptr<Sphere> sphere,
                                       std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToSphere, colData),
    m_pointSet(pointSet),
    m_sphere(sphere)
{
}

void
PointSetToSphereCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Get sphere properties
    const auto sphereCenter = m_sphere->getPosition();
    const auto sphereRadius = m_sphere->getRadius();
    const auto sphereRadiusSqr = sphereRadius * sphereRadius;

    ParallelUtils::ParallelSpinLock lock;
    ParallelUtils::parallelFor(m_pointSet->getVertexPositions().size(),
        [&](const size_t idx)
        {
            const auto p = m_pointSet->getVertexPosition(idx);
            const auto pc = sphereCenter - p;
            const auto distSqr = pc.squaredNorm();
            if (distSqr <= sphereRadiusSqr && distSqr > Real(1e-12))
            {
                const auto direction = pc / std::sqrt(distSqr);
                const auto pointOnSphere = sphereCenter - sphereRadius * direction;
                const auto penetrationDir = p - pointOnSphere;

                lock.lock();
                m_colData->MAColData.push_back({ idx, penetrationDir });
                lock.unlock();
            }
       });
}
} // imstk
