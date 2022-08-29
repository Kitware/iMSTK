/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkParallelUtils.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
SurfaceMeshToSphereCD::SurfaceMeshToSphereCD()
{
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<Sphere>(1);
}

void
SurfaceMeshToSphereCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    std::shared_ptr<SurfaceMesh> surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
    std::shared_ptr<Sphere>      sphere   = std::dynamic_pointer_cast<Sphere>(geomB);

    const Vec3d& spherePos    = sphere->getPosition();
    const double sphereRadius = sphere->getRadius();

    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMesh->getCells();
    const VecDataArray<int, 3>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;

    // \todo: Doesn't remove duplicate contacts (shared edges), refer to SurfaceMeshCD for easy method to do so
    ParallelUtils::SpinLock lock;
    ParallelUtils::parallelFor(indices.size(), [&](int i)
        {
            const Vec3i& cell = indices[i];
            const Vec3d& x1   = vertices[cell[0]];
            const Vec3d& x2   = vertices[cell[1]];
            const Vec3d& x3   = vertices[cell[2]];

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
                int pointContact;
                int caseType = CollisionUtils::testSphereToTriangle(
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
                    elemA.parentId = i; // Triangle id
                    elemA.cellType = IMSTK_EDGE;

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
                else if (caseType == 2) // Triangle vs point on sphere
                {
                                        // Face contact
                    CellIndexElement elemA;
                    elemA.ids[0]   = cell[0];
                    elemA.ids[1]   = cell[1];
                    elemA.ids[2]   = cell[2];
                    elemA.idCount  = 3;
                    elemA.parentId = i; // Triangle id
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
                else if (caseType == 3)
                {
                    Vec3d contactNormal = (spherePos - triangleContactPt);
                    const double dist   = contactNormal.norm();
                    const double penetrationDepth = sphereRadius - dist;
                    contactNormal /= dist;

                                                    // Point contact
                    PointIndexDirectionElement elemA;
                    elemA.ptIndex = pointContact;   // Point on triangle
                    elemA.dir     = -contactNormal; // Direction to resolve point on triangle
                    elemA.penetrationDepth = penetrationDepth;

                    PointDirectionElement elemB;
                    elemB.pt  = spherePos - sphereRadius * contactNormal; // Contact point on sphere
                    elemB.dir = contactNormal;                            // Direction to resolve point on sphere
                    elemB.penetrationDepth = penetrationDepth;

                    lock.lock();
                    elementsA.push_back(elemA);
                    elementsB.push_back(elemB);
                    lock.unlock();
                }
            }
        });
}
} // namespace imstk