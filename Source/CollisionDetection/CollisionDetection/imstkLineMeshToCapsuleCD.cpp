/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLineMeshToCapsuleCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkCapsule.h"
#include "imstkLineMesh.h"

namespace imstk
{
LineMeshToCapsuleCD::LineMeshToCapsuleCD()
{
    setRequiredInputType<LineMesh>(0);
    setRequiredInputType<Capsule>(1);
}

void
LineMeshToCapsuleCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<LineMesh> lineMesh = std::dynamic_pointer_cast<LineMesh>(geomA);
    std::shared_ptr<Capsule>  capsule  = std::dynamic_pointer_cast<Capsule>(geomB);

    const Vec3d& capsulePos         = capsule->getPosition();
    const double capsuleRadius      = capsule->getRadius();
    const double capsuleLength      = capsule->getLength();
    const Quatd& capsuleOrientation = capsule->getOrientation();
    const Vec3d& capsulePosA        = capsulePos - 0.5 * capsuleLength * capsuleOrientation.toRotationMatrix().col(1);
    const Vec3d& capsulePosB        = capsulePos + (capsulePos - capsulePosA);

    std::shared_ptr<VecDataArray<int, 2>>    indicesPtr  = lineMesh->getCells();
    const VecDataArray<int, 2>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = lineMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;

    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(indices.size(), [&](int i)
        {
            const Vec2i& cell = indices[i];
            const Vec3d& x1   = vertices[cell[0]];
            const Vec3d& x2   = vertices[cell[1]];

            double capBoundingRaidus = (capsuleLength / 2.0) + capsuleRadius;

            // This approach does a built in sphere sweep
            // \todo: Spatial accelerators need to be abstracted
            const Vec3d centroid = (x1 + x2) / 2.0;

            // Find the maximal point from centroid for radius
            const double rSqr1 = (centroid - x1).squaredNorm();

            const double lineBoundingRadius = std::sqrt(rSqr1);

            const double distSqr = (centroid - capsulePos).squaredNorm();
            const double rSum    = lineBoundingRadius + capBoundingRaidus;

            // Test for intersection of bounding sphere
            if (distSqr < rSum * rSum)
            {
                // Choose the closest point on capsule axis to create a virtual sphere for CD
                int unusedCaseType = 0;
                Vec3d capClosestPt = Vec3d::Zero();
                Vec3d segClosestPt = Vec3d::Zero();

                unusedCaseType = CollisionUtils::edgeToEdgeClosestPoints(
                    capsulePosA, capsulePosB,
                    x1, x2,
                    capClosestPt, segClosestPt);

                double seperationDistance = (capClosestPt - segClosestPt).norm();

                // test if closest point is less than capsule raidus
                if (seperationDistance <= capsuleRadius)
                {
                    // Get type (point-point or point-edge)
                    int caseType = -1;
                    if ((x1 - segClosestPt).norm() <= 1E-12)
                    {
                        caseType = 0;
                    }
                    else if ((x2 - segClosestPt).norm() <= 1E-12)
                    {
                        caseType = 1;
                    }
                    else if (seperationDistance <= 1E-12)
                    {
                        caseType = 3;
                    }
                    else
                    {
                        caseType = 2;
                    }

                    Vec3d contactNormal = (capClosestPt - segClosestPt);

                    // Capsule contact with x1
                    if (caseType == 0)
                    {
                        const double penetrationDepth = capsuleRadius - seperationDistance;
                        contactNormal /= seperationDistance;

                        PointIndexDirectionElement elemA;
                        elemA.ptIndex = cell[0];        // Point on line
                        elemA.dir     = -contactNormal; // Direction to resolve point on line
                        elemA.penetrationDepth = penetrationDepth;

                        PointDirectionElement elemB;
                        elemB.pt  = capClosestPt - capsuleRadius * contactNormal; // Contact point on capsule
                        elemB.dir = contactNormal;                                // Direction to resolve point on capsuel
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                    // Capsule contact with x2
                    else if (caseType == 1)
                    {
                        const double penetrationDepth = capsuleRadius - seperationDistance;
                        contactNormal /= seperationDistance;

                        PointIndexDirectionElement elemA;
                        elemA.ptIndex = cell[1];        // Point on line
                        elemA.dir     = -contactNormal; // Direction to resolve point on line
                        elemA.penetrationDepth = penetrationDepth;

                        PointDirectionElement elemB;
                        elemB.pt  = capClosestPt - capsuleRadius * contactNormal; // Contact point on capsule
                        elemB.dir = contactNormal;                                // Direction to resolve point on capsuel
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                    // Capsule contact between x1 and x2
                    else if (caseType == 2)
                    {
                        CellIndexElement elemA;
                        elemA.ids[0]   = cell[0];
                        elemA.ids[1]   = cell[1];
                        elemA.idCount  = 2;
                        elemA.parentId = i; // Edge id
                        elemA.cellType = IMSTK_EDGE;

                        const double penetrationDepth = capsuleRadius - seperationDistance;
                        contactNormal /= seperationDistance;

                        PointDirectionElement elemB;
                        elemB.dir = contactNormal;                                // Direction to resolve capsule
                        elemB.pt  = capClosestPt - capsuleRadius * contactNormal; // Contact point on capsule
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                    // Capsule centerline coincident with segment
                    else if (caseType == 3)
                    {
                        Vec3d segVec = x2 - x1;
                        Vec3d capVec = capsulePosB - capsulePosA;
                        Vec3d escapeDirection = capVec.cross(segVec).normalized();

                        CellIndexElement elemA;
                        elemA.ids[0]   = cell[0];
                        elemA.ids[1]   = cell[1];
                        elemA.idCount  = 2;
                        elemA.parentId = i; // Edge id
                        elemA.cellType = IMSTK_EDGE;

                        const double penetrationDepth = capsuleRadius - seperationDistance;
                        contactNormal /= seperationDistance;

                        PointDirectionElement elemB;
                        elemB.dir = escapeDirection;                                // Direction to resolve sphere
                        elemB.pt  = capClosestPt - capsuleRadius * escapeDirection; // Contact point on sphere
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                }
            }
            }, indices.size() > 100);
}
} // namespace imstk