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

#include "imstkPointSetToCylinderCD.h"
#include "imstkCylinder.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSetToCylinderCD::PointSetToCylinderCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<Cylinder>(1);
}

void
PointSetToCylinderCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Cylinder> cylinder = std::dynamic_pointer_cast<Cylinder>(geomB);

    const Vec3d  cylinderPos    = cylinder->getPosition();
    const Vec3d  cylinderAxis   = cylinder->getOrientation().toRotationMatrix().col(1);
    const double cylinderLength = cylinder->getLength();
    const double cylinderRadius = cylinder->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cylinderContactPt;
            Vec3d cylinderContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCylinderToPoint(
                                cylinderPos, cylinderAxis, cylinderLength, cylinderRadius,
                                vertices[idx],
                                cylinderContactPt, cylinderContactNormal, pointContactNormal, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = pointContactNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                PointDirectionElement elemB;
                elemB.dir = cylinderContactNormal; // Direction to resolve cylinder
                elemB.pt  = cylinderContactPt;     // Contact point on surface of cylinder
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}

void
PointSetToCylinderCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Cylinder> cylinder = std::dynamic_pointer_cast<Cylinder>(geomB);

    const Vec3d  cylinderPos    = cylinder->getPosition();
    const Vec3d  cylinderAxis   = cylinder->getOrientation().toRotationMatrix().col(1);
    const double cylinderLength = cylinder->getLength();
    const double cylinderRadius = cylinder->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cylinderContactPt;
            Vec3d cylinderContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCylinderToPoint(
                                cylinderPos, cylinderAxis, cylinderLength, cylinderRadius,
                                vertices[idx],
                                cylinderContactPt, cylinderContactNormal, pointContactNormal, depth))
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
PointSetToCylinderCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Cylinder> cylinder = std::dynamic_pointer_cast<Cylinder>(geomB);

    const Vec3d  cylinderPos    = cylinder->getPosition();
    const Vec3d  cylinderAxis   = cylinder->getOrientation().toRotationMatrix().col(1);
    const double cylinderLength = cylinder->getLength();
    const double cylinderRadius = cylinder->getRadius();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;
    ParallelUtils::SpinLock                  lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cylinderContactPt;
            Vec3d cylinderContactNormal, pointContactNormal;
            double depth;
            if (CollisionUtils::testCylinderToPoint(
                                cylinderPos, cylinderAxis, cylinderLength, cylinderRadius,
                                vertices[idx],
                                cylinderContactPt, cylinderContactNormal, pointContactNormal, depth))
            {
                PointDirectionElement elemB;
                elemB.dir = cylinderContactNormal; // Direction to resolve cylinder
                elemB.pt  = cylinderContactPt;     // Contact point on surface of cylinder
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
                }, vertices.size() > 100);
}
}