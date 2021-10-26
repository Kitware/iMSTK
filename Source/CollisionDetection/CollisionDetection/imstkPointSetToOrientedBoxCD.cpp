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

#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkOrientedBox.h"
#include "imstkParallelUtils.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PointSetToOrientedBoxCD::PointSetToOrientedBoxCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<OrientedBox>(1);
}

void
PointSetToOrientedBoxCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet>    pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<OrientedBox> box      = std::dynamic_pointer_cast<OrientedBox>(geomB);

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

    const Vec3d             boxPos      = box->getPosition();
    const Mat3d             cubeRot     = box->getOrientation().toRotationMatrix();
    const Vec3d             cubeExtents = box->getExtents();
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cubeContactPt;
            Vec3d pointContactNormal;
            double depth;
            if (CollisionUtils::testOBBToPoint(
                boxPos, cubeRot, cubeExtents,
                vertices[idx],
                pointContactNormal, cubeContactPt, depth))
            {
                PointIndexDirectionElement elemA;
                elemA.dir     = pointContactNormal; // Direction to resolve pointset point
                elemA.ptIndex = idx;
                elemA.penetrationDepth = depth;

                PointDirectionElement elemB;
                elemB.dir = -pointContactNormal; // Direction to resolve cube
                elemB.pt  = cubeContactPt;       // Contact point on surface of cube
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}

void
PointSetToOrientedBoxCD::computeCollisionDataA(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet>    pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<OrientedBox> box      = std::dynamic_pointer_cast<OrientedBox>(geomB);

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

    const Vec3d             boxPos      = box->getPosition();
    const Mat3d             cubeRot     = box->getOrientation().toRotationMatrix();
    const Vec3d             cubeExtents = box->getExtents();
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cubeContactPt;
            Vec3d pointContactNormal;
            double depth;
            if (CollisionUtils::testOBBToPoint(
                boxPos, cubeRot, cubeExtents,
                vertices[idx],
                pointContactNormal, cubeContactPt, depth))
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
PointSetToOrientedBoxCD::computeCollisionDataB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet>    pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<OrientedBox> box      = std::dynamic_pointer_cast<OrientedBox>(geomB);

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

    const Vec3d             boxPos      = box->getPosition();
    const Mat3d             cubeRot     = box->getOrientation().toRotationMatrix();
    const Vec3d             cubeExtents = box->getExtents();
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(vertices.size(),
        [&](const int idx)
        {
            Vec3d cubeContactPt;
            Vec3d pointContactNormal;
            double depth;
            if (CollisionUtils::testOBBToPoint(
                boxPos, cubeRot, cubeExtents,
                vertices[idx],
                pointContactNormal, cubeContactPt, depth))
            {
                PointDirectionElement elemB;
                elemB.dir = -pointContactNormal; // Direction to resolve cube
                elemB.pt  = cubeContactPt;       // Contact point on surface of cube
                elemB.penetrationDepth = depth;

                lock.lock();
                elementsB.push_back(elemB);
                lock.unlock();
            }
        }, vertices.size() > 100);
}
}