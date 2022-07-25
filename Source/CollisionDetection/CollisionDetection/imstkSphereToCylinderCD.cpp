/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphereToCylinderCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkCylinder.h"
#include "imstkSphere.h"

namespace imstk
{
SphereToCylinderCD::SphereToCylinderCD()
{
    setRequiredInputType<Sphere>(0);
    setRequiredInputType<Cylinder>(1);
}

void
SphereToCylinderCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<Sphere>   sphere   = std::dynamic_pointer_cast<Sphere>(geomA);
    std::shared_ptr<Cylinder> cylinder = std::dynamic_pointer_cast<Cylinder>(geomB);

    // Get geometry properties
    const Vec3d  spherePos = sphere->getPosition();
    const double rSphere   = sphere->getRadius();

    const Vec3d  cylinderPos  = cylinder->getPosition();
    const Vec3d  cylinderAxis = cylinder->getOrientation().toRotationMatrix().col(1);
    const double rCylinder    = cylinder->getRadius();
    const double cylLength    = cylinder->getLength();

    Vec3d  sphereContactPt, cylinderContactPt;
    Vec3d  sphereContactNormal, cylinderContactNormal;
    double depth;
    if (CollisionUtils::testSphereToCylinder(
                spherePos, rSphere,
                cylinderPos, cylinderAxis, rCylinder, cylLength,
                sphereContactPt, sphereContactNormal,
                cylinderContactPt, cylinderContactNormal,
                depth))
    {
        PointDirectionElement elemA;
        elemA.dir = sphereContactNormal;         // Direction to resolve sphere
        elemA.pt  = sphereContactPt;
        elemA.penetrationDepth = depth;

        PointDirectionElement elemB;
        elemB.dir = cylinderContactNormal;         // Direction to resolve cylinder
        elemB.pt  = cylinderContactPt;
        elemB.penetrationDepth = depth;

        elementsA.push_back(elemA);
        elementsB.push_back(elemB);
    }
}
} // namespace imstk