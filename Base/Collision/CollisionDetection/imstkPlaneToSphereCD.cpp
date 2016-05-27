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
PlaneToSphereCD::computeCollisionData(std::shared_ptr<CollidingObject> objA,
                                      std::shared_ptr<CollidingObject> objB,
                                      CollisionData& colDataA,
                                      CollisionData& colDataB)
{
    auto planeGeom = std::dynamic_pointer_cast<Plane>(objA->getCollidingGeometry());
    auto sphereGeom = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

    // Geometries check
    if (planeGeom == nullptr || sphereGeom == nullptr)
    {
        LOG(WARNING) << "PlaneToSphereCD::computeCollisionData error: invalid geometries.";
        return;
    }

    // Get geometry properties
    Vec3d sP = sphereGeom->getPosition();
    double r = sphereGeom->getRadius();
    Vec3d pP = planeGeom->getPosition();
    Vec3d n = planeGeom->getNormal();

    // Compute shortest distance
    double d = (sP-pP).dot(n);

    // Define sphere to plane direction
    Vec3d dir = -n;
    if( d < 0 )
    {
        d = -d;
        dir = n;
    }

    // Return if no penetration
    double penetrationDepth = r-d;
    if ( penetrationDepth < 0)
    {
        return;
    }

    // Compute collision points
    Vec3d pC = sP + dir*d;
    Vec3d sC = sP + dir*r;

    // Set collisionData
    PositionDirectionCollisionData cdA = {pC, -dir, penetrationDepth};
    PositionDirectionCollisionData cdB = {sC, dir, penetrationDepth};
    colDataA.PDColData.push_back({pC, dir, penetrationDepth});
    colDataB.PDColData.push_back(cdB);

    LOG(DEBUG) << "Collision between "
               << objA->getName() << " & " << objB->getName()
               << " : " << penetrationDepth;

}

}
