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
SphereToSphereCD::computeCollisionData(std::shared_ptr<CollidingObject> objA,
                                       std::shared_ptr<CollidingObject> objB,
                                       CollisionData& colDataA,
                                       CollisionData& colDataB)
{
    auto sphereGeomA = std::dynamic_pointer_cast<Sphere>(objA->getCollidingGeometry());
    auto sphereGeomB = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

    // Geometries check
    if (sphereGeomA == nullptr || sphereGeomB == nullptr)
    {
        LOG(WARNING) << "SphereToSphereCD::computeCollisionData error: invalid geometries.";
        return;
    }

    // Get geometry properties
    Vec3d sAP = sphereGeomA->getPosition();
    double rA = sphereGeomA->getRadius() * sphereGeomA->getScaling();
    Vec3d sBP = sphereGeomB->getPosition();
    double rB = sphereGeomB->getRadius() * sphereGeomB->getScaling();

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
    colDataA.PDColData.push_back({sAC, dirBToA, penetrationDepth});
    colDataB.PDColData.push_back({sBC, -dirBToA, penetrationDepth});
}

}
