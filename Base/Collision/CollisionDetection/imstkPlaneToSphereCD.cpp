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

#include "imstkPlaneToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

#include <g3log/g3log.hpp>

namespace imstk {

void
PlaneToSphereCD::computeCollisionData()
{
    // Clear collisionData
    m_CDA.clearAll();
    m_CDB.clearAll();

    // Get geometry properties
    Vec3d sP = m_sphereB->getPosition();
    double r = m_sphereB->getRadius() * m_sphereB->getScaling();
    Vec3d pP = m_planeA->getPosition();
    Vec3d n = m_planeA->getNormal();

    // Compute shortest distance
    double d = (sP-pP).dot(n);

    // Define sphere to plane direction
    Vec3d dirBToA = -n;
    if( d < 0 )
    {
        d = -d;
        dirBToA = n;
    }

    // Return if no penetration
    double penetrationDepth = r-d;
    if ( penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    Vec3d pC = sP + dirBToA*d;
    Vec3d sC = sP + dirBToA*r;

    // Set collisionData
    m_CDA.PDColData.push_back({pC, dirBToA, penetrationDepth});
    m_CDB.PDColData.push_back({sC, -dirBToA, penetrationDepth});
}

}
