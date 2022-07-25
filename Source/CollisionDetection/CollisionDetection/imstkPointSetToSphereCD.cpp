/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointSetToSphereCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkSphere.h"

namespace imstk
{
PointSetToSphereCD::PointSetToSphereCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<Sphere>(1);
}

void
PointSetToSphereCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Sphere>   sphere   = std::dynamic_pointer_cast<Sphere>(geomB);

    const Vec3d& spherePos = sphere->getPosition();
    const double r = sphere->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d sphereContactPt, sphereContactNormal;
            double depth;

            if (CollisionUtils::testSphereToPoint(
                                spherePos, r, vertices[idx],
                                sphereContactPt, sphereContactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = sphereContactNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                PointDirectionElement elemB;
                elemB.dir = -sphereContactNormal; // Direction to resolve sphere
                elemB.pt  = sphereContactPt;
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}

void
PointSetToSphereCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Sphere>   sphere   = std::dynamic_pointer_cast<Sphere>(geomB);

    const Vec3d& spherePos = sphere->getPosition();
    const double r = sphere->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d sphereContactPt, sphereContactNormal;
            double depth;

            if (CollisionUtils::testSphereToPoint(
                                spherePos, r, vertices[idx],
                                sphereContactPt, sphereContactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = sphereContactNormal;             // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                lock.unlock();
            }
                }, vertices.size() > 100);
}

void
PointSetToSphereCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Sphere>   sphere   = std::dynamic_pointer_cast<Sphere>(geomB);

    const Vec3d& spherePos = sphere->getPosition();
    const double r = sphere->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d sphereContactPt, sphereContactNormal;
            double depth;

            if (CollisionUtils::testSphereToPoint(
                                spherePos, r, vertices[idx],
                                sphereContactPt, sphereContactNormal, depth))
            {
                PointDirectionElement elemB;
                elemB.dir = -sphereContactNormal;                 // Direction to resolve sphere
                elemB.pt  = sphereContactPt;
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}
} // namespace imstk