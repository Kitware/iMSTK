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

#include "imstkUnidirectionalPlaneToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

#include <g3log/g3log.hpp>

namespace imstk {
void
UnidirectionalPlaneToSphereCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Get geometry properties
    const Vec3d sphereBPos = m_sphereB->getPosition();
    const double r = m_sphereB->getRadius();
    const Vec3d planeAPos = m_planeA->getPosition();
    const Vec3d n = m_planeA->getNormal();

    // Compute shortest distance
    double d = (sphereBPos-planeAPos).dot(n);

    // Compute penetration depth
    // Half-space defined by the normal of the plane is considered as "outside".
    double penetrationDepth = r-d;
    if (penetrationDepth <= 0.0)
    {
        return;
    }

    // Compute collision points
    Vec3d planeAColPt = sphereBPos - n*d;
    Vec3d sphereBColPt = sphereBPos - n*r;

    // Set collisionData
    m_colData->PDColData.push_back({planeAColPt, sphereBColPt, n, penetrationDepth});
}
} //iMSTK
