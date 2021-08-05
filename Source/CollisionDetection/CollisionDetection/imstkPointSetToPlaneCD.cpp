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

#include "imstkPointSetToPlaneCD.h"
#include "imstkCollisionData.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"
#include "imstkPlane.h"
#include "imstkCollisionUtils.h"

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
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

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

                PointDirectionElement elemB;
                elemB.dir = -planeNormal; // Direction to resolve plane
                elemB.pt  = vertices[idx];
                elemB.penetrationDepth = depth;

                elementsA.safeAppend(elemA);
                elementsB.safeAppend(elemB);
            }
        }, vertices.size() > 100);
}

void
PointSetToPlaneCD::computeCollisionDataA(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

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

                elementsA.safeAppend(elemA);
            }
        }, vertices.size() > 100);
}

void
PointSetToPlaneCD::computeCollisionDataB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
    std::shared_ptr<Plane>    plane    = std::dynamic_pointer_cast<Plane>(geomB);

    const Vec3d planePt     = plane->getPosition();
    const Vec3d planeNormal = plane->getNormal();

    std::shared_ptr<VecDataArray<double, 3>> vertexData = pointSet->getVertexPositions();
    const VecDataArray<double, 3>&           vertices   = *vertexData;

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
                elemB.dir = -planeNormal; // Direction to resolve plane
                elemB.pt  = vertices[idx];
                elemB.penetrationDepth = depth;

                elementsB.safeAppend(elemB);
            }
        }, vertices.size() > 100);
}
}