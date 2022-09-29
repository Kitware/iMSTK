/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkUnidirectionalPlaneToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkPlane.h"

namespace imstk
{
UnidirectionalPlaneToCapsuleCD::UnidirectionalPlaneToCapsuleCD()
{
    setRequiredInputType<Plane>(0);
    setRequiredInputType<Capsule>(1);
}

void
UnidirectionalPlaneToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Plane>   plane   = std::dynamic_pointer_cast<Plane>(geomA);
    std::shared_ptr<Capsule> capsule = std::dynamic_pointer_cast<Capsule>(geomB);

    // Get geometry properties
    const Vec3d planePos = plane->getPosition();
    const Vec3d n = plane->getNormal();

    const Vec3d  capsule0Pos        = capsule->getPosition();
    const Vec3d  capsule0Axis       = capsule->getOrientation().toRotationMatrix().col(1).normalized();
    const double capsule0HalfLength = capsule->getLength() * 0.5;
    const Vec3d  diff0 = capsule0Axis * capsule0HalfLength;
    const double capsule0Radius = capsule->getRadius();

    // Find closest point on edge to plane

    const Vec3d p0 = capsule0Pos - diff0;
    const Vec3d p1 = capsule0Pos + diff0;

    const double signedDist0 = plane->getFunctionValue(p0);
    const double signedDist1 = plane->getFunctionValue(p1);

    // Find the deepest point
    Vec3d  deepestPtOnEdge = p0;
    double deepestDist     = signedDist0;
    if (signedDist1 < signedDist0)
    {
        deepestPtOnEdge = p1;
        deepestDist     = signedDist1;
    }

    // If collision
    if (deepestDist - capsule0Radius < 0.0)
    {
        // If nearly coplanar use midpoint
        if ((signedDist1 - signedDist0) < IMSTK_DOUBLE_EPS)
        {
            deepestPtOnEdge = (p0 + p1) * 0.5;
        }

        const double depth = std::abs(deepestDist - capsule0Radius);

        PointDirectionElement elemA;
        elemA.dir = -n; // Direction to resolve plane
        elemA.pt  = deepestPtOnEdge + n * std::abs(deepestDist);
        elemA.penetrationDepth = depth;

        PointDirectionElement elemB;
        elemB.dir = n;                                    // Direction to resolve capsule
        elemB.pt  = deepestPtOnEdge - n * capsule0Radius; // Contact point on capsule
        elemB.penetrationDepth = depth;

        elementsA.push_back(elemA);
        elementsB.push_back(elemB);
    }
}
} // namespace imstk