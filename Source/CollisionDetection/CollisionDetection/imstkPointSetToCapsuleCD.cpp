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

#include "imstkPointSetToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionData.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"
#include "imstkCollisionUtils.h"

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
}