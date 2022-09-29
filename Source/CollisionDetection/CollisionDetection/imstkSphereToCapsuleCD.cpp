/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphereToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionUtils.h"
#include "imstkSphere.h"

namespace imstk
{
SphereToCapsuleCD::SphereToCapsuleCD()
{
    setRequiredInputType<Sphere>(0);
    setRequiredInputType<Capsule>(1);
}

void
SphereToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Sphere>  sphere0  = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Capsule> capsule1 = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d  sphere0Pos    = sphere0->getPosition();
    const double sphere0Radius = sphere0->getRadius();

    const Vec3d  capsule1Pos        = capsule1->getPosition();
    const Vec3d  capsule1Axis       = capsule1->getOrientation().toRotationMatrix().col(1).normalized();
    const double capsule1HalfLength = capsule1->getLength() * 0.5;
    const Vec3d  diff1 = capsule1Axis * capsule1HalfLength;
    const double capsule1Radius = capsule1->getRadius();

    // Find the closest point on the two segments
    int         caseType  = -1;
    const Vec3d closestPt = CollisionUtils::closestPointOnSegment(sphere0Pos,
        capsule1Pos - diff1, capsule1Pos + diff1, caseType);

    // We can interpret spheres around these two points of capsule radius
    // The point of contact should be between these two spheres
    Vec3d  sphereAContactPt, sphereBContactPt;
    Vec3d  sphereAContactNormal, sphereBContactNormal;
    double depth;
    if (CollisionUtils::testSphereToSphere(
        sphere0Pos, sphere0Radius, closestPt, capsule1Radius,
        sphereAContactPt, sphereAContactNormal,
        sphereBContactPt, sphereBContactNormal,
        depth))
    {
        PointDirectionElement elemA;
        elemA.dir = sphereAContactNormal; // Direction to resolve sphereA
        elemA.pt  = sphereAContactPt;     // Contact point on sphereB
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