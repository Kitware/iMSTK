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

#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionUtils.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
SurfaceMeshToCapsuleCD::SurfaceMeshToCapsuleCD()
{
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<Capsule>(1);
}

void
SurfaceMeshToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<SurfaceMesh> surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
    std::shared_ptr<Capsule>     capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d& capsulePos         = capsule->getPosition();
    const double capsuleRadius      = capsule->getRadius();
    const double capsuleLength      = capsule->getLength();
    const Quatd& capsuleOrientation = capsule->getOrientation();
    const Vec3d& capsulePosA        = capsulePos - 0.5 * capsuleLength * capsuleOrientation.toRotationMatrix().transpose().col(1);
    const Vec3d& capsulePosB        = capsulePos + (capsulePos - capsulePosA);

    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMesh->getTriangleIndices();
    const VecDataArray<int, 3>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;

    // \todo: Doesn't remove duplicate contacts (shared edges), refer to SurfaceMeshCD for easy method to do so
    for (int i = 0; i < indices.size(); i++)
    {
        const Vec3i& cell = indices[i];
        const Vec3d& x1   = vertices[cell[0]];
        const Vec3d& x2   = vertices[cell[1]];
        const Vec3d& x3   = vertices[cell[2]];

        // Choose the closest point on capsule axis to create a virtual sphere for CD
        int         unusedCaseType = 0;
        const Vec3d trianglePointA = CollisionUtils::closestPointOnTriangle(capsulePosA, x1, x2, x3, unusedCaseType);
        const Vec3d trianglePointB = CollisionUtils::closestPointOnTriangle(capsulePosB, x1, x2, x3, unusedCaseType);
        const auto  segmentPointA  = CollisionUtils::closestPointOnSegment(trianglePointA, capsulePosA, capsulePosB, unusedCaseType);
        const auto  segmentPointB  = CollisionUtils::closestPointOnSegment(trianglePointB, capsulePosA, capsulePosB, unusedCaseType);
        const auto  distanceA      = (segmentPointA - trianglePointA).norm();
        const auto  distanceB      = (segmentPointB - trianglePointB).norm();

        const double sphereRadius = capsuleRadius;
        Vec3d        spherePos(0, 0, 0);
        if (distanceA < distanceB)
        {
            spherePos = segmentPointA;
        }
        else if (distanceA > distanceB)
        {
            spherePos = segmentPointB;
        }
        else // parallel
        {
            spherePos = (segmentPointA + segmentPointB) / 2;
        }

        // This approach does a built in sphere sweep
        // \todo: Spatial accelerators need to be abstracted
        const Vec3d centroid = (x1 + x2 + x3) / 3.0;

        // Find the maximal point from centroid for radius
        const double rSqr1 = (centroid - x1).squaredNorm();
        const double rSqr2 = (centroid - x2).squaredNorm();
        const double rSqr3 = (centroid - x3).squaredNorm();
        const double triangleBoundingRadius = std::sqrt(std::max(rSqr1, std::max(rSqr2, rSqr3)));

        const double distSqr = (centroid - spherePos).squaredNorm();
        const double rSum    = triangleBoundingRadius + sphereRadius;
        if (distSqr < rSum * rSum)
        {
            Vec3d triangleContactPt;
            Vec2i edgeContact;
            int   pointContact;
            int   caseType = CollisionUtils::testSphereToTriangle(
                spherePos, sphereRadius,
                cell, x1, x2, x3,
                triangleContactPt,
                edgeContact, pointContact);
            if (caseType == 1) // Edge vs point on sphere
            {
                // Edge contact
                CellIndexElement elemA;
                elemA.ids[0]   = edgeContact[0];
                elemA.ids[1]   = edgeContact[1];
                elemA.idCount  = 2;
                elemA.cellType = IMSTK_EDGE;

                Vec3d        contactNormal = (spherePos - triangleContactPt);
                const double dist = contactNormal.norm();
                const double penetrationDepth = sphereRadius - dist;
                contactNormal /= dist;

                PointDirectionElement elemB;
                elemB.dir = contactNormal;                            // Direction to resolve sphere
                elemB.pt  = spherePos - sphereRadius * contactNormal; // Contact point on sphere
                elemB.penetrationDepth = penetrationDepth;

                elementsA.unsafeAppend(elemA);
                elementsB.unsafeAppend(elemB);
            }
            else if (caseType == 2) // Triangle vs point on sphere
            {
                // Face contact
                CellIndexElement elemA;
                elemA.ids[0]   = cell[0];
                elemA.ids[1]   = cell[1];
                elemA.ids[2]   = cell[2];
                elemA.idCount  = 3;
                elemA.cellType = IMSTK_TRIANGLE;

                Vec3d        contactNormal = (spherePos - triangleContactPt);
                const double dist = contactNormal.norm();
                const double penetrationDepth = sphereRadius - dist;
                contactNormal /= dist;

                PointDirectionElement elemB;
                elemB.dir = contactNormal;                            // Direction to resolve sphere
                elemB.pt  = spherePos - sphereRadius * contactNormal; // Contact point on sphere
                elemB.penetrationDepth = penetrationDepth;

                elementsA.unsafeAppend(elemA);
                elementsB.unsafeAppend(elemB);
            }
            else if (caseType == 3)
            {
                Vec3d        contactNormal = (spherePos - triangleContactPt);
                const double dist = contactNormal.norm();
                const double penetrationDepth = sphereRadius - dist;
                contactNormal /= dist;

                // Point contact
                PointIndexDirectionElement elemA;
                elemA.ptIndex = pointContact;
                elemA.dir     = -contactNormal; // Direction to resolve point
                elemA.penetrationDepth = penetrationDepth;

                PointDirectionElement elemB;
                elemB.pt  = triangleContactPt; // Point on sphere
                elemB.dir = contactNormal;     // Direction to resolve point
                elemB.penetrationDepth = penetrationDepth;

                elementsA.unsafeAppend(elemA);
                elementsB.unsafeAppend(elemB);
            }
        }
    }
}
}