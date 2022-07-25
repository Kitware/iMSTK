/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointSetToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSetToCapsuleCD::PointSetToCapsuleCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<Capsule>(1);
}

void
PointSetToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Capsule>  capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d  capsulePos    = capsule->getPosition();
    const Vec3d  capsuleAxis   = capsule->getOrientation().toRotationMatrix().col(1);
    const double capsuleLength = capsule->getLength();
    const double capsuleRadius = capsule->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d capsuleContactPt;
            Vec3d capsuleContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCapsuleToPoint(
                                capsulePos, capsuleAxis, capsuleLength, capsuleRadius,
                                vertices[idx],
                                capsuleContactPt, capsuleContactNormal, pointContactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = pointContactNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                PointDirectionElement elemB;
                elemB.dir = capsuleContactNormal; // Direction to resolve capsule
                elemB.pt  = capsuleContactPt;     // Contact point on surface of capsule
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}

void
PointSetToCapsuleCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Capsule>  capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d  capsulePos    = capsule->getPosition();
    const Vec3d  capsuleAxis   = capsule->getOrientation().toRotationMatrix().col(1);
    const double capsuleLength = capsule->getLength();
    const double capsuleRadius = capsule->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d capsuleContactPt;
            Vec3d capsuleContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCapsuleToPoint(
                                capsulePos, capsuleAxis, capsuleLength, capsuleRadius,
                                vertices[idx],
                                capsuleContactPt, capsuleContactNormal, pointContactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = pointContactNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                lock.unlock();
            }
                }, vertices.size() > 100);
}

void
PointSetToCapsuleCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Capsule>  capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d  capsulePos    = capsule->getPosition();
    const Vec3d  capsuleAxis   = capsule->getOrientation().toRotationMatrix().col(1);
    const double capsuleLength = capsule->getLength();
    const double capsuleRadius = capsule->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d capsuleContactPt;
            Vec3d capsuleContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCapsuleToPoint(
                                capsulePos, capsuleAxis, capsuleLength, capsuleRadius,
                                vertices[idx],
                                capsuleContactPt, capsuleContactNormal, pointContactNormal, depth))
            {
                PointDirectionElement elemB;
                elemB.dir = capsuleContactNormal; // Direction to resolve capsule
                elemB.pt  = capsuleContactPt;     // Contact point on surface of capsule
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}
} // namespace imstk