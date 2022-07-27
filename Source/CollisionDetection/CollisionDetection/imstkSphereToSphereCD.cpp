/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Sphere> sphereA = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Sphere> sphereB = std::dynamic_pointer_cast<Sphere>(geomB);

    // Get geometry properties
    const Vec3d  sphereAPos = sphereA->getPosition();
    const double rA = sphereA->getRadius();
    const Vec3d  sphereBPos = sphereB->getPosition();
    const double rB = sphereB->getRadius();

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

        elementsA.push_back(elemA);
        elementsB.push_back(elemB);
    }
}
} // namespace imstk