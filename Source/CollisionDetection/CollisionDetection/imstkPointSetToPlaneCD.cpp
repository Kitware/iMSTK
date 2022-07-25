/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointSetToPlaneCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSetToPlaneCD::PointSetToPlaneCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<Plane>(1);

    // By default plane cd is not generated
    setGenerateCD(true, false);
}

void
PointSetToPlaneCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(static_cast<unsigned int>(vertices.size()),
        [&](const unsigned int idx)
        {
            Vec3d contactPt, contactNormal;
            double depth;

            if (CollisionUtils::testPlaneToPoint(
                planePt, planeNormal, vertices[idx],
                contactPt, contactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = planeNormal; // Direction to resolve pointset point to
                elemA.ptIndex = idx;         // Point on pointset
                elemA.penetrationDepth = depth;

                PointDirectionElement elemB;
                elemB.dir = -planeNormal;                        // Direction to resolve plane
                elemB.pt  = vertices[idx] + planeNormal * depth; // Point on plane
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}

void
PointSetToPlaneCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(static_cast<unsigned int>(vertices.size()),
        [&](const unsigned int idx)
        {
            Vec3d contactPt, contactNormal;
            double depth;

            if (CollisionUtils::testPlaneToPoint(
                planePt, planeNormal, vertices[idx],
                contactPt, contactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = planeNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                lock.unlock();
            }
        }, vertices.size() > 100);
}

void
PointSetToPlaneCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(static_cast<unsigned int>(vertices.size()),
        [&](const unsigned int idx)
        {
            Vec3d contactPt, contactNormal;
            double depth;

            if (CollisionUtils::testPlaneToPoint(
                planePt, planeNormal, vertices[idx],
                contactPt, contactNormal, depth))
            {
                PointDirectionElement elemB;
                elemB.dir = -planeNormal;                        // Direction to resolve plane
                elemB.pt  = vertices[idx] + planeNormal * depth; // Point on plane
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}
} // namespace imstk