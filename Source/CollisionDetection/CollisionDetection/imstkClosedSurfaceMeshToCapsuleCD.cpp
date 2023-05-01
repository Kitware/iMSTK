/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkClosedSurfaceMeshToCapsuleCD.h"
#include "imstkCapsule.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
ClosedSurfaceMeshToCapsuleCD::ClosedSurfaceMeshToCapsuleCD()
{
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<Capsule>(1);
}

void
ClosedSurfaceMeshToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<SurfaceMesh> surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
    std::shared_ptr<Capsule>     capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d& capsulePos         = capsule->getPosition();
    const double capsuleRadius      = capsule->getRadius();
    const double capsuleLength      = capsule->getLength();
    const Quatd& capsuleOrientation = capsule->getOrientation();

    // PosA and PosB are the end points of the line used to represent the cylindrical section of the capsule
    const Vec3d& capsulePosA = capsulePos - 0.5 * capsuleLength * capsuleOrientation.toRotationMatrix().col(1);
    const Vec3d& capsulePosB = capsulePos + (capsulePos - capsulePosA);

    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMesh->getCells();
    const VecDataArray<int, 3>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;

    // \todo: Doesn't remove duplicate contacts (shared edges), refer to SurfaceMeshCD for easy method to do so
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(indices.size(), [&](int cellId)
        {
            const Vec3i& cell = indices[cellId];
            const Vec3d& x1   = vertices[cell[0]];
            const Vec3d& x2   = vertices[cell[1]];
            const Vec3d& x3   = vertices[cell[2]];

            // Use signed distance value to filter triangles
            double minSDV = std::numeric_limits<double>::max();
            for (int vertId = 0; vertId < 3; vertId++)
            {
                auto SDV = capsule->getFunctionValue(vertices[cell[vertId]]);
                minSDV   = std::min(minSDV, SDV);
            }
            // Only generate CD if capsule is sufficiently close to triangle
            if (minSDV <= capsuleRadius * 8.0)
            {
                // Choose the closest point on capsule axis to create a virtual sphere for CD
                int unusedCaseType = 0;
                const Vec3d trianglePointA = CollisionUtils::closestPointOnTriangle(capsulePosA, x1, x2, x3, unusedCaseType);
                const Vec3d trianglePointB = CollisionUtils::closestPointOnTriangle(capsulePosB, x1, x2, x3, unusedCaseType);

                const auto segmentPointA = CollisionUtils::closestPointOnSegment(trianglePointA, capsulePosA, capsulePosB, unusedCaseType);
                const auto segmentPointB = CollisionUtils::closestPointOnSegment(trianglePointB, capsulePosA, capsulePosB, unusedCaseType);

                const auto distanceA = (segmentPointA - trianglePointA).norm();
                const auto distanceB = (segmentPointB - trianglePointB).norm();

                const double sphereRadius = capsuleRadius;
                Vec3d spherePos(0, 0, 0);

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
                    spherePos = (segmentPointA + segmentPointB) / 2.0;
                }

                // Create possible contact points
                // These are set by testSphereToTriangle depending on
                // what geometry is collided
                Vec3d triangleContactPt;

                int caseType = CollisionUtils::testSphereToTriangle(
                    spherePos, sphereRadius,
                    x1, x2, x3,
                    triangleContactPt);

                // Test if capsule centerline intersects with surface
                Vec3d uvw;
                const bool inserted = CollisionUtils::testSegmentTriangle(
                    capsulePosA, capsulePosB,
                    x1, x2, x3,
                    uvw);

                // If capsule segment is inside of triangle, find nearest segment tip and
                // create constraint to move the capsule out using that position + radius
                if (inserted)
                {
                    caseType = 2;
                }

                // Contact with triangle face
                if (caseType == 1)
                {
                    CellIndexElement elemA;
                    elemA.ids[0]   = cell[0];
                    elemA.ids[1]   = cell[1];
                    elemA.ids[2]   = cell[2];
                    elemA.idCount  = 3;
                    elemA.parentId = cellId;
                    elemA.cellType = IMSTK_TRIANGLE;

                    Vec3d contactNormal = (spherePos - triangleContactPt);
                    const double dist   = contactNormal.norm();
                    const double penetrationDepth = sphereRadius - dist;
                    contactNormal /= dist;

                    PointDirectionElement elemB;
                    elemB.dir = contactNormal;                            // Direction to resolve sphere
                    elemB.pt  = spherePos - sphereRadius * contactNormal; // Contact point on sphere
                    elemB.penetrationDepth = penetrationDepth;

                    lock.lock();
                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                    lock.unlock();
                }
                // Capsule body intersecting triangle
                else if (caseType == 2)
                {
                    CellIndexElement elemA;
                    elemA.ids[0]   = cell[0];
                    elemA.ids[1]   = cell[1];
                    elemA.ids[2]   = cell[2];
                    elemA.idCount  = 3;
                    elemA.parentId = cellId;
                    elemA.cellType = IMSTK_TRIANGLE;

                    Vec3d contactNormal     = (spherePos - triangleContactPt);
                    const double dist       = contactNormal.norm();
                    double penetrationDepth = sphereRadius - dist;
                    contactNormal /= dist;

                    contactNormal = (x2 - x1).cross(x3 - x1).normalized();

                    penetrationDepth = sphereRadius * 2.0;

                    PointDirectionElement elemB;
                    elemB.dir = contactNormal;                            // Direction to resolve sphere
                    elemB.pt  = spherePos - sphereRadius * contactNormal; // Contact point on sphere
                    elemB.penetrationDepth = penetrationDepth;

                    lock.lock();
                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                    lock.unlock();
                }
            }
        }, false);
}
} // namespace imstk