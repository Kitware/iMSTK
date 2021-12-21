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
}