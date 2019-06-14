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

#include <g3log/g3log.hpp>

namespace imstk
{
void
PointSetToSphereCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Get sphere properties
    auto spherePos = m_sphere->getPosition();
    auto radius = m_sphere->getRadius();

    size_t nodeId = 0;
    for (const auto& p : m_pointSet->getVertexPositions())
    {
        auto dist = (spherePos - p).norm();
        if (dist <= radius)
        {
            auto direction = (spherePos - p)/dist;
            auto pointOnSphere = spherePos - radius*direction;
            m_colData->MAColData.push_back({ nodeId, p - pointOnSphere });
        }
        nodeId++;
    }
}
} // imstk
