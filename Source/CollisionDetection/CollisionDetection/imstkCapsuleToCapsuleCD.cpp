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

#include "imstkCapsuleToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
CapsuleToCapsuleCD::CapsuleToCapsuleCD()
{
    setRequiredInputType<Capsule>(0);
    setRequiredInputType<Capsule>(1);
}

void
CapsuleToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Capsule> capsule0 = std::dynamic_pointer_cast<Capsule>(geomA);
    std::shared_ptr<Capsule> capsule1 = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d  capsule0Pos        = capsule0->getPosition();
    const Vec3d  capsule0Axis       = capsule0->getOrientation().toRotationMatrix().col(1).normalized();
    const double capsule0HalfLength = capsule0->getLength() * 0.5;
    const Vec3d  diff0 = capsule0Axis * capsule0HalfLength;
    const double capsule0Radius = capsule0->getRadius();

    const Vec3d  capsule1Pos        = capsule1->getPosition();
    const Vec3d  capsule1Axis       = capsule1->getOrientation().toRotationMatrix().col(1).normalized();
    const double capsule1HalfLength = capsule1->getLength() * 0.5;
    const Vec3d  diff1 = capsule1Axis * capsule1HalfLength;
    const double capsule1Radius = capsule1->getRadius();

    // Find the closest point on the two segments
    Vec3d pt0, pt1;
    CollisionUtils::edgeToEdgeClosestPoints(
        capsule0Pos - diff0, capsule0Pos + diff0,
        capsule1Pos - diff1, capsule1Pos + diff1,
        pt0, pt1);

    // Skip case
    if (pt0 == pt1)
    {
        return;
    }

    // \todo: This gives bad normals

    // We can interpret spheres around these two points of capsule radius
    // The point of contact should be between these two spheres
    Vec3d  sphereAContactPt, sphereBContactPt;
    Vec3d  sphereAContactNormal, sphereBContactNormal;
    double depth;
    if (CollisionUtils::testSphereToSphere(
        pt0, capsule0Radius, pt1, capsule1Radius,
        sphereAContactPt, sphereAContactNormal,
        sphereBContactPt, sphereBContactNormal,
        depth))
    {
        PointDirectionElement elemA;
        elemA.dir = sphereAContactNormal; // Direction to resolve capsuleA
        elemA.pt  = sphereAContactPt;     // Contact point on capsuleA
        elemA.penetrationDepth = depth;

        PointDirectionElement elemB;
        elemB.dir = sphereBContactNormal; // Direction to resolve capsuleB
        elemB.pt  = sphereBContactPt;     // Contact point on capsuleB
        elemB.penetrationDepth = depth;

        elementsA.push_back(elemA);
        elementsB.push_back(elemB);
    }
}
} // namespace imstk