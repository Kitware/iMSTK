/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version B.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-B.0.txt

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
    m_CDA.clearAll();
    m_CDB.clearAll();

    // Get geometry properties
    Vec3d sAP = m_sphereA->getPosition();
    double rA = m_sphereA->getRadius() * m_sphereA->getScaling();
    Vec3d sBP = m_sphereB->getPosition();
    double rB = m_sphereB->getRadius() * m_sphereB->getScaling();

    // Compute direction vector
    Vec3d dirBToA = sAP - sBP;

    // Compute shortest distance
    double d = dirBToA.norm();

    // Return if no penetration
    double penetrationDepth = rA+rB-d;
    if ( penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    dirBToA.normalize();
    Vec3d sAC = sAP - dirBToA*rA;
    Vec3d sBC = sBP + dirBToA*rB;

    // Set collisionData
    m_CDA.PDColData.push_back({sAC, dirBToA, penetrationDepth});
    m_CDB.PDColData.push_back({sBC, -dirBToA, penetrationDepth});
}

}
