/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCapsuleToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionUtils.h"

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