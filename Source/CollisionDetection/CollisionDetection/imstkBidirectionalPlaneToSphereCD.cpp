/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkSphere.h"
#include "imstkPlane.h"

namespace imstk
{
BidirectionalPlaneToSphereCD::BidirectionalPlaneToSphereCD()
{
    setRequiredInputType<Plane>(0);
    setRequiredInputType<Sphere>(1);
}

void
BidirectionalPlaneToSphereCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Plane>  plane  = std::dynamic_pointer_cast<Plane>(geomA);
    std::shared_ptr<Sphere> sphere = std::dynamic_pointer_cast<Sphere>(geomB);

    // Get geometry properties
    const Vec3d  spherePos = sphere->getPosition();
    const double r = sphere->getRadius();
    const Vec3d  planePos = plane->getPosition();
    const Vec3d  n = plane->getNormal();

    Vec3d  planeContactPt, sphereContactPt;
    Vec3d  planeContactNormal, sphereContactNormal;
    double depth;
    if (CollisionUtils::testBidirectionalPlaneToSphere(
                planePos, n,
                spherePos, r,
                planeContactPt, planeContactNormal,
                sphereContactPt, sphereContactNormal,
                depth))
    {
        PointDirectionElement elemA;
        elemA.dir = planeContactNormal;         // Direction to resolve plane
        elemA.pt  = planeContactPt;
        elemA.penetrationDepth = depth;

        PointDirectionElement elemB;
        elemB.dir = sphereContactNormal;         // Direction to resolve sphere
        elemB.pt  = sphereContactPt;
        elemB.penetrationDepth = depth;

        elementsA.push_back(CollisionElement(elemA));
        elementsB.push_back(elemB);
    }
}
} // namespace imstk