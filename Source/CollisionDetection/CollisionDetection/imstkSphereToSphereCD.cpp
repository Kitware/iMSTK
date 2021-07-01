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
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkSphere.h"

namespace imstk
{
SphereToSphereCD::SphereToSphereCD() : CollisionDetectionAlgorithm()
{
    setRequiredInputType<Sphere>(0);
    setRequiredInputType<Sphere>(1);
}

void
SphereToSphereCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<Sphere> sphereA = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Sphere> sphereB = std::dynamic_pointer_cast<Sphere>(geomB);

    // Get geometry properties
    Vec3d  sphereAPos = sphereA->getPosition();
    double rA = sphereA->getRadius();
    Vec3d  sphereBPos = sphereB->getPosition();
    double rB = sphereB->getRadius();

    Vec3d  sphereAContactPt, sphereBContactPt;
    Vec3d  sphereAContactNormal, sphereBContactNormal;
    double depth;
    if (CollisionUtils::testSphereToSphere(
        sphereAPos, rA, sphereBPos, rB,
        sphereAContactPt, sphereAContactNormal,
        sphereBContactPt, sphereBContactNormal,
        depth))
    {
        PointDirectionElement elemA;
        elemA.dir = sphereAContactNormal; // Direction to resolve sphereA
        elemA.pt  = sphereAContactPt;     // Contact point on sphereA
        elemA.penetrationDepth = depth;

        PointDirectionElement elemB;
        elemB.dir = sphereBContactNormal; // Direction to resolve sphereB
        elemB.pt  = sphereBContactPt;     // Contact point on sphereB
        elemB.penetrationDepth = depth;

        elementsA.unsafeAppend(elemA);
        elementsB.unsafeAppend(elemB);
    }
}

void
SphereToSphereCD::computeCollisionDataA(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA)
{
    std::shared_ptr<Sphere> sphereA = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Sphere> sphereB = std::dynamic_pointer_cast<Sphere>(geomB);

    // Get geometry properties
    Vec3d  sphereAPos = sphereA->getPosition();
    double rA = sphereA->getRadius();
    Vec3d  sphereBPos = sphereB->getPosition();
    double rB = sphereB->getRadius();

    Vec3d  sphereAContactPt, sphereBContactPt;
    Vec3d  sphereAContactNormal, sphereBContactNormal;
    double depth;
    if (CollisionUtils::testSphereToSphere(
        sphereAPos, rA, sphereBPos, rB,
        sphereAContactPt, sphereAContactNormal,
        sphereBContactPt, sphereBContactNormal,
        depth))
    {
        PointDirectionElement elemA;
        elemA.dir = sphereAContactNormal; // Direction to resolve sphereA
        elemA.pt  = sphereAContactPt;
        elemA.penetrationDepth = depth;

        elementsA.unsafeAppend(elemA);
    }
}

void
SphereToSphereCD::computeCollisionDataB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<Sphere> sphereA = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Sphere> sphereB = std::dynamic_pointer_cast<Sphere>(geomB);

    // Get geometry properties
    Vec3d  sphereAPos = sphereA->getPosition();
    double rA = sphereA->getRadius();
    Vec3d  sphereBPos = sphereB->getPosition();
    double rB = sphereB->getRadius();

    Vec3d  sphereAContactPt, sphereBContactPt;
    Vec3d  sphereAContactNormal, sphereBContactNormal;
    double depth;
    if (CollisionUtils::testSphereToSphere(
        sphereAPos, rA, sphereBPos, rB,
        sphereAContactPt, sphereAContactNormal,
        sphereBContactPt, sphereBContactNormal,
        depth))
    {
        PointDirectionElement elemB;
        elemB.dir = sphereBContactNormal; // Direction to resolve sphereB
        elemB.pt  = sphereBContactPt;
        elemB.penetrationDepth = depth;

        elementsB.unsafeAppend(elemB);
    }
}
}