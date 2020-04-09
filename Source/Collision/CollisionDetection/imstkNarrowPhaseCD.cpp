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

#include "imstkNarrowPhaseCD.h"
#include "imstkCollisionUtils.h"

#include "imstkLogger.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
namespace NarrowPhaseCD
{
void
bidirectionalPlaneToSphere(Geometry* const planeGeo, Geometry* const sphereGeo,
                           const std::shared_ptr<CollisionData>& colData)
{
    const auto plane  = static_cast<Plane*>(planeGeo);
    const auto sphere = static_cast<Sphere*>(sphereGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Plane*>(planeGeo) == nullptr
                   || dynamic_cast<Sphere*>(sphereGeo) == nullptr)) << "Invalid geometries";
#endif
    bidirectionalPlaneToSphere(plane, sphere, colData);
}

void
bidirectionalPlaneToSphere(Plane* const planeA, Sphere* const sphereB,
                           const std::shared_ptr<CollisionData>& colData)
{
    // Get geometry properties
    Vec3d  sphereBPos = sphereB->getPosition();
    double r = sphereB->getRadius() * sphereB->getScaling();
    Vec3d  planeAPos = planeA->getPosition();
    Vec3d  n = planeA->getNormal();

    // Compute shortest distance
    double d = (sphereBPos - planeAPos).dot(n);

    // Define sphere to plane direction
    Vec3d dirAToB = n;
    if (d < 0)
    {
        d       = -d;
        dirAToB = -n;
    }

    // Return if no penetration
    double penetrationDepth = r - d;
    if (penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    Vec3d planeAColPt  = sphereBPos - dirAToB * d;
    Vec3d sphereBColPt = sphereBPos - dirAToB * r;

    // Set collisionData
    colData->PDColData.safeAppend({ planeAColPt,
                                    sphereBColPt,
                                    dirAToB, penetrationDepth });
}

void
unidirectionalPlaneToSphere(Geometry* const planeGeo, Geometry* const sphereGeo,
                            const std::shared_ptr<CollisionData>& colData)
{
    const auto plane  = static_cast<Plane*>(planeGeo);
    const auto sphere = static_cast<Sphere*>(sphereGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Plane*>(planeGeo) == nullptr
                   || dynamic_cast<Sphere*>(sphereGeo) == nullptr)) << "Invalid geometries";
#endif
    unidirectionalPlaneToSphere(plane, sphere, colData);
}

void
unidirectionalPlaneToSphere(Plane* const plane, Sphere* const sphere,
                            const std::shared_ptr<CollisionData>& colData)
{
    // Get geometry properties
    const Vec3d  sphereBPos = sphere->getPosition();
    const double r = sphere->getRadius();
    const Vec3d  planeAPos = plane->getPosition();
    const Vec3d  n = plane->getNormal();

    // Compute shortest distance
    double d = (sphereBPos - planeAPos).dot(n);

    // Compute penetration depth
    // Half-space defined by the normal of the plane is considered as "outside".
    double penetrationDepth = r - d;
    if (penetrationDepth <= 0.0)
    {
        return;
    }

    // Compute collision points
    Vec3d planeAColPt  = sphereBPos - n * d;
    Vec3d sphereBColPt = sphereBPos - n * r;

    // Set collisionData
    colData->PDColData.safeAppend({ planeAColPt,
                                    sphereBColPt,
                                    n, penetrationDepth });
}

void
sphereToCylinder(Geometry* const sphereGeo, Geometry* const cylinderGeo,
                 const std::shared_ptr<CollisionData>& colData)
{
    const auto sphere   = static_cast<Sphere*>(sphereGeo);
    const auto cylinder = static_cast<Cylinder*>(cylinderGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Sphere*>(sphereGeo) == nullptr
                   || dynamic_cast<Cylinder*>(cylinderGeo) == nullptr)) << "Invalid geometries";
#endif
    sphereToCylinder(sphere, cylinder, colData);
}

void
sphereToCylinder(Sphere* const sphere, Cylinder* const cylinder,
                 const std::shared_ptr<CollisionData>& colData)
{
    // Get geometry properties
    const Vec3d  spherePos = sphere->getPosition();
    const double rSphere   = sphere->getRadius();

    const Vec3d  cylinderPos  = cylinder->getPosition();
    const Vec3d  cylinderAxis = cylinder->getOrientationAxis();
    const double rCylinder    = cylinder->getRadius();

    // Compute shortest distance
    Vec3d distVec = (spherePos - cylinderPos) - cylinderAxis * (spherePos - cylinderPos).dot(cylinderAxis);
    Vec3d n       = -distVec / distVec.norm();

    // Compute penetration depth
    double penetrationDepth = distVec.norm() - rSphere - rCylinder;
    if (penetrationDepth > 0.0)
    {
        return;
    }

    // Compute collision points
    Vec3d sphereColPt   = spherePos + rSphere * n;
    Vec3d cylinderColPt = cylinderPos + cylinderAxis * (spherePos - cylinderPos).dot(cylinderAxis) +
                          n * rCylinder;

    // Set collisionData
    colData->PDColData.safeAppend({ sphereColPt,
                                    cylinderColPt,
                                    n, penetrationDepth });
}

void
sphereToSphere(Geometry* const sphereGeoA, Geometry* const sphereGeoB,
               const std::shared_ptr<CollisionData>& colData)
{
    const auto sphereA = static_cast<Sphere*>(sphereGeoA);
    const auto sphereB = static_cast<Sphere*>(sphereGeoB);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Sphere*>(sphereGeoA) == nullptr
                   || dynamic_cast<Sphere*>(sphereGeoB) == nullptr)) << "Invalid geometries";
#endif
    sphereToSphere(sphereA, sphereB, colData);
}

void
sphereToSphere(Sphere* const sphereA, Sphere* const sphereB,
               const std::shared_ptr<CollisionData>& colData)
{
    // Get geometry properties
    Vec3d  sphereAPos = sphereA->getPosition();
    double rA = sphereA->getRadius();
    Vec3d  sphereBPos = sphereB->getPosition();
    double rB = sphereB->getRadius();

    // Compute direction vector
    Vec3d dirAToB = sphereBPos - sphereAPos;

    // Compute shortest distance
    double d = dirAToB.norm();

    // Return if no penetration
    double penetrationDepth = rA + rB - d;
    if (penetrationDepth <= 0)
    {
        return;
    }

    // Compute collision points
    dirAToB.normalize();
    Vec3d sphereAColPt = sphereAPos + dirAToB * rA;
    Vec3d sphereBColPt = sphereBPos - dirAToB * rB;

    // Set collisionData
    colData->PDColData.safeAppend({ sphereAColPt,
                                    sphereBColPt,
                                    dirAToB, penetrationDepth });
}

void
pointToCapsule(const Vec3r& point, uint32_t pointIdx, Geometry* const capsuleGeo,
               const std::shared_ptr<CollisionData>& colData)
{
    const auto capsule = static_cast<Capsule*>(capsuleGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Capsule*>(capsuleGeo) == nullptr)) << "Invalid geometry";
#endif
    pointToCapsule(point, pointIdx, capsule, colData);
}

void
pointToCapsule(const Vec3r& point, uint32_t pointIdx, Capsule* const capsule,
               const std::shared_ptr<CollisionData>& colData)
{
    const Vec3d capsulePos = capsule->getPosition();
    const auto  length     = capsule->getLength();
    const auto  radius     = capsule->getRadius();

    // Get position of end points of the capsule
    // TODO: Fix this issue of extra computation in future
    const Vec3d p0     = capsulePos;
    const Vec3d p1     = p0 + capsule->getOrientationAxis() * length;
    const Vec3d mid    = 0.5 * (p0 + p1);
    const Vec3d p      = p1 - p0;
    const auto  pDotp  = p.dot(p);
    const auto  pDotp0 = p.dot(p0);

    // First, check collision with bounding sphere
    if ((mid - point).norm() > (radius + length * 0.5))
    {
        return;
    }

    // Do the actual check
    const auto  alpha = (point.dot(p) - pDotp0) / pDotp;
    const Vec3d closestPoint = p0 + p * alpha;

    // If the point is inside the bounding sphere then the closest point
    // should be inside the capsule
    const auto dist = (closestPoint - point).norm();
    if (dist < radius)
    {
        const auto  direction      = (closestPoint - point) / dist;
        const Vec3d pointOnCapsule = closestPoint - radius * direction;
        colData->MAColData.safeAppend({ pointIdx, p - pointOnCapsule });
    }
}

void
pointToPlane(const Vec3r& point, uint32_t pointIdx, Geometry* const planeGeo,
             const std::shared_ptr<CollisionData>& colData)
{
    const auto plane = static_cast<Plane*>(planeGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Plane*>(planeGeo) == nullptr)) << "Invalid geometry";
#endif
    pointToPlane(point, pointIdx, plane, colData);
}

void
pointToPlane(const Vec3r& point, uint32_t pointIdx, Plane* const plane,
             const std::shared_ptr<CollisionData>& colData)
{
    // Get plane properties
    const auto planePos        = plane->getPosition();
    const auto planeNormal     = plane->getNormal();
    const auto penetrationDist = (point - planePos).dot(planeNormal);

    if (penetrationDist < 0.0)
    {
        const auto penetrationDir = planeNormal * penetrationDist;
        colData->MAColData.safeAppend({ pointIdx, penetrationDir });
    }
}

void
pointToSphere(const Vec3r& point, uint32_t pointIdx, Geometry* const sphereGeo,
              const std::shared_ptr<CollisionData>& colData)
{
    const auto sphere = static_cast<Sphere*>(sphereGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Sphere*>(sphereGeo) == nullptr)) << "Invalid geometry";
#endif
    pointToSphere(point, pointIdx, sphere, colData);
}

void
pointToSphere(const Vec3r& point, uint32_t pointIdx, Sphere* const sphere,
              const std::shared_ptr<CollisionData>& colData)
{
    const Vec3r sphereCenter    = sphere->getPosition();
    const auto  sphereRadius    = sphere->getRadius();
    const auto  sphereRadiusSqr = sphereRadius * sphereRadius;

    const Vec3r pc      = sphereCenter - point;
    const auto  distSqr = pc.squaredNorm();
    if (distSqr < sphereRadiusSqr)
    {
        const Vec3r direction      = distSqr > Real(1e-12) ? pc / std::sqrt(distSqr) : Vec3r(0, 0, 0);
        const Vec3r pointOnSphere  = sphereCenter - sphereRadius * direction;
        const Vec3r penetrationDir = point - pointOnSphere;
        colData->MAColData.safeAppend({ pointIdx, penetrationDir });
    }
}

void
pointToSpherePicking(const Vec3r& point, uint32_t pointIdx, Geometry* const sphereGeo,
                     const std::shared_ptr<CollisionData>& colData)
{
    const auto sphere = static_cast<Sphere*>(sphereGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<Sphere*>(sphereGeo) == nullptr)) << "Invalid geometry";
#endif
    pointToSpherePicking(point, pointIdx, sphere, colData);
}

void
pointToSpherePicking(const Vec3r& point, uint32_t pointIdx, Sphere* const sphere,
                     const std::shared_ptr<CollisionData>& colData)
{
    const Vec3r sphereCenter    = sphere->getPosition();
    const auto  sphereRadius    = sphere->getRadius();
    const auto  sphereRadiusSqr = sphereRadius * sphereRadius;

    const Vec3r pc      = sphereCenter - point;
    const auto  distSqr = pc.squaredNorm();
    if (distSqr < sphereRadiusSqr)
    {
        colData->NodePickData.safeAppend({ pc, pointIdx, 0 });
    }
}

void
triangleToTriangle(uint32_t triIdx1, Geometry* const triMeshGeo1, uint32_t triIdx2, Geometry* const triMeshGeo2,
                   const std::shared_ptr<CollisionData>& colData)
{
    const auto mesh1 = static_cast<SurfaceMesh*>(triMeshGeo1);
    const auto mesh2 = static_cast<SurfaceMesh*>(triMeshGeo2);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<SurfaceMesh*>(triMeshGeo1) == nullptr
                   || dynamic_cast<SurfaceMesh*>(triMeshGeo2) == nullptr)) << "Invalid geometries";
#endif
    triangleToTriangle(triIdx1, mesh1, triIdx2, mesh2, colData);
}

void
triangleToTriangle(uint32_t triIdx1, SurfaceMesh* const triMesh1, uint32_t triIdx2, SurfaceMesh* const triMesh2,
                   const std::shared_ptr<CollisionData>& colData)
{
    const auto tri1Face = triMesh1->getTrianglesVertices()[triIdx1];
    const auto tri2Face = triMesh2->getTrianglesVertices()[triIdx2];

    const std::array<Vec3r, 3> tri1Verts = { triMesh1->getVertexPosition(tri1Face[0]),
                                             triMesh1->getVertexPosition(tri1Face[1]),
                                             triMesh1->getVertexPosition(tri1Face[2]) };

    const std::array<Vec3r, 3> tri2Verts = { triMesh2->getVertexPosition(tri2Face[0]),
                                             triMesh2->getVertexPosition(tri2Face[1]),
                                             triMesh2->getVertexPosition(tri2Face[2]) };

    // Edges of the first triangle
    const std::pair<Vec3r, Vec3r> tri1Edges[3] {
        { tri1Verts[0], tri1Verts[1] },
        { tri1Verts[0], tri1Verts[2] },
        { tri1Verts[1], tri1Verts[2] }
    };

    bool bIntersected[3];
    for (uint32_t i = 0; i < 3; ++i)
    {
        bIntersected[i] = CollisionUtils::segmentIntersectsTriangle(tri1Edges[i].first, tri1Edges[i].second,
                                       tri2Verts[0], tri2Verts[1], tri2Verts[2]);
    }

    // Count number of edge-triangle intersections
    const int numIntersections = bIntersected[0] + bIntersected[1] + bIntersected[2];

    if (numIntersections == 0)
    {
        return;
    }
    else if (numIntersections == 2)
    {
        if (bIntersected[0])
        {
            const auto vertIdx = bIntersected[1] ? tri1Face[0] : tri1Face[1];
            colData->VTColData.safeAppend({ static_cast<uint32_t>(vertIdx), triIdx2, 0 });
        }
        else
        {
            colData->VTColData.safeAppend({ static_cast<uint32_t>(tri1Face[2]), triIdx2, 0 });
        }
    }
    else if (numIntersections == 1)
    {
        std::pair<uint32_t, uint32_t> edgeIdA;
        if (bIntersected[0])
        {
            edgeIdA = { static_cast<uint32_t>(tri1Face[0]), static_cast<uint32_t>(tri1Face[1]) };
        }
        else if (bIntersected[1])
        {
            edgeIdA = { static_cast<uint32_t>(tri1Face[0]), static_cast<uint32_t>(tri1Face[2]) };
        }
        else
        {
            edgeIdA = { static_cast<uint32_t>(tri1Face[1]), static_cast<uint32_t>(tri1Face[2]) };
        }

        const std::pair<Vec3r, Vec3r> tri2Edges[3] {
            { tri2Verts[0], tri2Verts[1] },
            { tri2Verts[0], tri2Verts[2] },
            { tri2Verts[1], tri2Verts[2] }
        };

        bool bFound = false; // Due to numerical round-off errors, the other triangle may not intersect with the current one

        // Find the only edge of triangle2 that intersects with triangle1
        std::pair<uint32_t, uint32_t> edgeIdB;
        for (uint32_t i = 0; i < 3; ++i)
        {
            if (CollisionUtils::segmentIntersectsTriangle(tri2Edges[i].first, tri2Edges[i].second,
                                           tri1Verts[0], tri1Verts[1], tri1Verts[2]))
            {
                if (i == 0)
                {
                    edgeIdB = { static_cast<uint32_t>(tri2Face[0]), static_cast<uint32_t>(tri2Face[1]) };
                }
                else if (i == 1)
                {
                    edgeIdB = { static_cast<uint32_t>(tri2Face[0]), static_cast<uint32_t>(tri2Face[2]) };
                }
                else
                {
                    edgeIdB = { static_cast<uint32_t>(tri2Face[1]), static_cast<uint32_t>(tri2Face[2]) };
                }
                bFound = true;
                break;
            }
        }
        if (bFound)
        {
            colData->EEColData.safeAppend({ edgeIdA, edgeIdB, 0 });
        }
    }
}

bool
pointToTriangle(const Vec3r& point, uint32_t pointIdx, uint32_t triIdx, Geometry* const triMeshGeo,
                const std::shared_ptr<CollisionData>& colData)
{
    const auto triMesh = static_cast<SurfaceMesh*>(triMeshGeo);
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (dynamic_cast<SurfaceMesh*>(triMeshGeo) == nullptr)) << "Invalid geometry";
#endif
    return pointToTriangle(point, pointIdx, triIdx, triMesh, colData);
}

bool
pointToTriangle(const Vec3r& point, uint32_t pointIdx, uint32_t triIdx, SurfaceMesh* const triMesh,
                const std::shared_ptr<CollisionData>& colData)
{
    const auto face   = triMesh->getTrianglesVertices()[triIdx];
    const auto x1     = triMesh->getVertexPosition(face[0]);
    const auto x2     = triMesh->getVertexPosition(face[1]);
    const auto x3     = triMesh->getVertexPosition(face[2]);
    const auto normal = (x2 - x1).cross(x3 - x1);
    const auto pa     = point - x1;
    if (pa.dot(normal) > 0)
    {
        return false;
    }

    const Real closestDistance = CollisionUtils::pointTriangleClosestDistance(point, x1, x2, x3);
    colData->VTColData.safeAppend({ pointIdx, triIdx, closestDistance });
    return true;
}
} // namespace NarrowPhaseCD
} // namespace imstk
