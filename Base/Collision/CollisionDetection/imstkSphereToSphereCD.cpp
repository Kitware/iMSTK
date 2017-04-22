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

#include "imstkSphereToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkSphere.h"

#include <g3log/g3log.hpp>

namespace imstk {

void
SphereToSphereCD::computeCollisionData()
{
    // Clear collisionData
    m_colData.clearAll();

    // Get geometry properties
    Vec3d sphereAPos = m_sphereA->getPosition();
    double rA = m_sphereA->getRadius();
    Vec3d sphereBPos = m_sphereB->getPosition();
    double rB = m_sphereB->getRadius();

    // Compute direction vector
    Vec3d dirAToB = sphereBPos - sphereAPos;

    // Compute shortest distance
    double d = dirAToB.norm();

    // Return if no penetration
    double penetrationDepth = rA+rB-d;
    if ( penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    dirAToB.normalize();
    Vec3d sphereAColPt = sphereAPos + dirAToB*rA;
    Vec3d sphereBColPt = sphereBPos - dirAToB*rB;

    // Set collisionData
    m_colData.PDColData.push_back({sphereAColPt, sphereBColPt, dirAToB, penetrationDepth});
}

}
