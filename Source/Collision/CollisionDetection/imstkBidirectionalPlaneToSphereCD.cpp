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

#include "imstkBidirectionalPlaneToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

#include <g3log/g3log.hpp>

namespace imstk
{
void
BidirectionalPlaneToSphere::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Get geometry properties
    Vec3d sphereBPos = m_sphereB->getPosition();
    double r = m_sphereB->getRadius() * m_sphereB->getScaling();
    Vec3d planeAPos = m_planeA->getPosition();
    Vec3d n = m_planeA->getNormal();

    // Compute shortest distance
    double d = (sphereBPos - planeAPos).dot(n);

    // Define sphere to plane direction
    Vec3d dirAToB = n;
    if (d < 0)
    {
        d = -d;
        dirAToB = -n;
    }

    // Return if no penetration
    double penetrationDepth = r - d;
    if (penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    Vec3d planeAColPt = sphereBPos - dirAToB * d;
    Vec3d sphereBColPt = sphereBPos - dirAToB * r;

    // Set collisionData
    m_colData->PDColData.push_back({planeAColPt, sphereBColPt, dirAToB, penetrationDepth});
}
}
