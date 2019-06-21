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

#include "imstkPointSetToSpherePickingCD.h"
#include "imstkCollisionData.h"
#include "imstkPointSet.h"
#include "imstkSphere.h"
#include "imstkParallelUtils.h"

namespace imstk
{
PointSetToSpherePickingCD::PointSetToSpherePickingCD(std::shared_ptr<PointSet> pointSet,
                                                     std::shared_ptr<Sphere> sphere,
                                                     std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::PointSetToSphere, colData),
    m_pointSet(pointSet),
    m_sphere(sphere)
{
}

void
PointSetToSpherePickingCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    if (!m_deviceTracker || !m_deviceTracker->getDeviceClient()->getButton(m_buttonId))
    {
        return;
    }

    // Get sphere properties
    auto spherePos = m_sphere->getPosition();
    auto radius = m_sphere->getRadius() * m_sphere->getScaling();

    ParallelUtils::ParallelSpinLock lock;
    ParallelUtils::parallelFor(m_pointSet->getVertexPositions().size(),
        [&](const size_t idx)
        {
            const auto p = m_pointSet->getVertexPosition(idx);
            auto dist = (spherePos - p).norm();
            if (dist <= radius)
            {
                lock.lock();
                m_colData->NodePickData.push_back({ spherePos - p, idx, 0 });
                lock.unlock();
            }
        });
}

void PointSetToSpherePickingCD::setDeviceTrackerAndButton(const std::shared_ptr<DeviceTracker> devTracker,
                                                          const unsigned int )
{
    m_deviceTracker = devTracker;
}
} // imstk
