/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLineMeshToSphereCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkSphere.h"
#include "imstkLineMesh.h"

namespace imstk
{
LineMeshToSphereCD::LineMeshToSphereCD()
{
    setRequiredInputType<LineMesh>(0);
    setRequiredInputType<Sphere>(1);
}

void
LineMeshToSphereCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<LineMesh> lineMesh = std::dynamic_pointer_cast<LineMesh>(geomA);
    std::shared_ptr<Sphere>   sphere   = std::dynamic_pointer_cast<Sphere>(geomB);

    const Vec3d& spherePos    = sphere->getPosition();
    const double sphereRadius = sphere->getRadius();

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

            // This approach does a built in sphere sweep
            // \todo: Spatial accelerators need to be abstracted
            const Vec3d centroid = (x1 + x2) / 2.0;

            // Find the maximal point from centroid for radius
            const double rSqr1 = (centroid - x1).squaredNorm();

            const double lineBoundingRadius = std::sqrt(rSqr1);

            const double distSqr = (centroid - spherePos).squaredNorm();
            const double rSum    = lineBoundingRadius + sphereRadius;

            if (distSqr < rSum * rSum)
            {
                Vec3d lineContactPt;
                int caseType = -1;

                lineContactPt = CollisionUtils::closestPointOnSegment(
                        spherePos,
                        x1, x2,
                        caseType);

                const double sphereDist = (lineContactPt - spherePos).squaredNorm();
                if (sphereDist <= sphereRadius * sphereRadius)
                {
                    // Sphere contact with x1
                    if (caseType == 0)
                    {
                        Vec3d contactNormal = (spherePos - lineContactPt);
                        const double dist   = contactNormal.norm();
                        const double penetrationDepth = sphereRadius - dist;
                        contactNormal /= dist;

                                                        // Point contact
                        PointIndexDirectionElement elemA;
                        elemA.ptIndex = cell[0];        // Point on line
                        elemA.dir     = -contactNormal; // Direction to resolve point on line
                        elemA.penetrationDepth = penetrationDepth;

                        PointDirectionElement elemB;
                        elemB.pt  = spherePos - sphereRadius * contactNormal;    // Contact point on sphere
                        elemB.dir = contactNormal;                               // Direction to resolve point on sphere
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                    // Sphere contact with x2
                    else if (caseType == 1)
                    {
                        Vec3d contactNormal = (spherePos - lineContactPt);
                        const double dist   = contactNormal.norm();
                        const double penetrationDepth = sphereRadius - dist;
                        contactNormal /= dist;

                                                        // Point contact
                        PointIndexDirectionElement elemA;
                        elemA.ptIndex = cell[1];        // Point on line
                        elemA.dir     = -contactNormal; // Direction to resolve point on line
                        elemA.penetrationDepth = penetrationDepth;

                        PointDirectionElement elemB;
                        elemB.pt  = spherePos - sphereRadius * contactNormal;    // Contact point on sphere
                        elemB.dir = contactNormal;                               // Direction to resolve point on sphere
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                    // Sphere contact between x1 and x2
                    else if (caseType == 2)
                    {
                        CellIndexElement elemA;
                        elemA.ids[0]   = cell[0];
                        elemA.ids[1]   = cell[1];
                        elemA.idCount  = 2;
                        elemA.parentId = i; // line id
                        elemA.cellType = IMSTK_EDGE;

                        Vec3d contactNormal = (spherePos - lineContactPt);
                        const double dist   = contactNormal.norm();
                        const double penetrationDepth = sphereRadius - dist;
                        contactNormal /= dist;

                        PointDirectionElement elemB;
                        elemB.dir = contactNormal;                              // Direction to resolve sphere
                        elemB.pt  = spherePos - sphereRadius * contactNormal;   // Contact point on sphere
                        elemB.penetrationDepth = penetrationDepth;

                        lock.lock();
                        elementsA.push_back(elemA);
                        elementsB.push_back(elemB);
                        lock.unlock();
                    }
                }
            }
            }, indices.size() > 500);
}
} // namespace imstk