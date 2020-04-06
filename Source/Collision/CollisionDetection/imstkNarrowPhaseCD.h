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

#pragma once

#include <memory>

#include "imstkMath.h"

namespace imstk
{
struct CollisionData;

class Geometry;
class Plane;
class Sphere;
class Capsule;
class Cylinder;
class SurfaceMesh;

///
/// \brief Implement narrow-phase collision detection for various primitives
///
namespace NarrowPhaseCD
{
///
/// \brief BidirectionalPlaneToSphere
///
void bidirectionalPlaneToSphere(Geometry* const planeGeo, Geometry* const sphereGeo,
                                const std::shared_ptr<CollisionData>& colData);
void bidirectionalPlaneToSphere(Plane* const planeA, Sphere* const sphereB,
                                const std::shared_ptr<CollisionData>& colData);
inline void
bidirectionalPlaneToSphere(const std::shared_ptr<Plane>& plane, const std::shared_ptr<Sphere>& sphere,
                           const std::shared_ptr<CollisionData>& colData)
{
    bidirectionalPlaneToSphere(plane.get(), sphere.get(), colData);
}

///
/// \brief UnidirectionalPlaneToSphere
///
void unidirectionalPlaneToSphere(Geometry* const planeGeo, Geometry* const sphereGeo,
                                 const std::shared_ptr<CollisionData>& colData);
void unidirectionalPlaneToSphere(Plane* const plane, Sphere* const sphere,
                                 const std::shared_ptr<CollisionData>& colData);
inline void
unidirectionalPlaneToSphere(const std::shared_ptr<Plane>& plane, const std::shared_ptr<Sphere>& sphere,
                            const std::shared_ptr<CollisionData>& colData)
{
    unidirectionalPlaneToSphere(plane.get(), sphere.get(), colData);
}

///
/// \brief SphereToCylinder
///
void sphereToCylinder(Geometry* const sphereGeo, Geometry* const cylinderGeo,
                      const std::shared_ptr<CollisionData>& colData);
void sphereToCylinder(Sphere* const sphere, Cylinder* const cylinder,
                      const std::shared_ptr<CollisionData>& colData);
inline void
sphereToCylinder(const std::shared_ptr<Sphere>& sphere, const std::shared_ptr<Cylinder>& cylinder,
                 const std::shared_ptr<CollisionData>& colData)
{
    sphereToCylinder(sphere.get(), cylinder.get(), colData);
}

///
/// \brief SphereToSphere
///
void sphereToSphere(Geometry* const sphereGeoA, Geometry* const sphereGeoB,
                    const std::shared_ptr<CollisionData>& colData);
void sphereToSphere(Sphere* const sphereA, Sphere* const sphereB,
                    const std::shared_ptr<CollisionData>& colData);
inline void
sphereToSphere(const std::shared_ptr<Sphere>& sphereA, const std::shared_ptr<Sphere>& sphereB,
               const std::shared_ptr<CollisionData>& colData)
{
    sphereToSphere(sphereA.get(), sphereB.get(), colData);
}

///
/// \brief PointToCapsule
///
void pointToCapsule(const Vec3r& point, uint32_t pointIdx, Geometry* const capsuleGeo,
                    const std::shared_ptr<CollisionData>& colData);
void pointToCapsule(const Vec3r& point, uint32_t pointIdx, Capsule* const capsule,
                    const std::shared_ptr<CollisionData>& colData);
inline void
pointToCapsule(const Vec3r& point, uint32_t pointIdx, const std::shared_ptr<Capsule>& capsule,
               const std::shared_ptr<CollisionData>& colData)
{
    pointToCapsule(point, pointIdx, capsule.get(), colData);
}

///
/// \brief PointToPlane
///
void pointToPlane(const Vec3r& point, uint32_t pointIdx, Geometry* const planeGeo,
                  const std::shared_ptr<CollisionData>& colData);
void pointToPlane(const Vec3r& point, uint32_t pointIdx, Plane* const plane,
                  const std::shared_ptr<CollisionData>& colData);
inline void
pointToPlane(const Vec3r& point, uint32_t pointIdx, const std::shared_ptr<Plane>& plane,
             const std::shared_ptr<CollisionData>& colData)
{
    pointToPlane(point, pointIdx, plane.get(), colData);
}

///
/// \brief PointToSphere
///
void pointToSphere(const Vec3r& point, uint32_t pointIdx, Geometry* const sphereGeo,
                   const std::shared_ptr<CollisionData>& colData);
void pointToSphere(const Vec3r& point, uint32_t pointIdx, Sphere* const sphere,
                   const std::shared_ptr<CollisionData>& colData);
inline void
pointToSphere(const Vec3r& point, uint32_t pointIdx, const std::shared_ptr<Sphere>& sphere,
              const std::shared_ptr<CollisionData>& colData)
{
    pointToSphere(point, pointIdx, sphere.get(), colData);
}

///
/// \brief PointToSpherePicking
///
void pointToSpherePicking(const Vec3r& point, uint32_t pointIdx, Geometry* const sphereGeo,
                          const std::shared_ptr<CollisionData>& colData);
void pointToSpherePicking(const Vec3r& point, uint32_t pointIdx, Sphere* const sphere,
                          const std::shared_ptr<CollisionData>& colData);
inline void
pointToSpherePicking(const Vec3r& point, uint32_t pointIdx, const std::shared_ptr<Sphere>& sphere,
                     const std::shared_ptr<CollisionData>& colData)
{
    pointToSpherePicking(point, pointIdx, sphere.get(), colData);
}

///
/// \brief TriangleToTriangle
///
void triangleToTriangle(uint32_t triIdx1, Geometry* const triMeshGeo1,
                        uint32_t triIdx2, Geometry* const triMeshGeo2,
                        const std::shared_ptr<CollisionData>& colData);
void triangleToTriangle(uint32_t triIdx1, SurfaceMesh* const triMesh1,
                        uint32_t triIdx2, SurfaceMesh* const triMesh2,
                        const std::shared_ptr<CollisionData>& colData);
inline void
triangleToTriangle(uint32_t triIdx1, const std::shared_ptr<SurfaceMesh>& triMesh1,
                   uint32_t triIdx2, const std::shared_ptr<SurfaceMesh>& triMesh2,
                   const std::shared_ptr<CollisionData>& colData)
{
    triangleToTriangle(triIdx1, triMesh1.get(), triIdx2, triMesh2.get(), colData);
}

///
/// \brief PointToTriangle
/// \return true if the point 'penetrated' triangle (a point P penetrates a triangle ABC if dot(P - A, n) < 0, where n is triangle normal)
///
bool pointToTriangle(const Vec3r& point, uint32_t pointIdx,
                     uint32_t triIdx, Geometry* const triMeshGeo,
                     const std::shared_ptr<CollisionData>& colData);
bool pointToTriangle(const Vec3r& point, uint32_t pointIdx,
                     uint32_t triIdx, SurfaceMesh* const triMesh,
                     const std::shared_ptr<CollisionData>& colData);
inline void
pointToTriangle(const Vec3r& point, uint32_t pointIdx, uint32_t triIdx, const std::shared_ptr<SurfaceMesh>& triMesh,
                const std::shared_ptr<CollisionData>& colData)
{
    pointToTriangle(point, pointIdx, triIdx, triMesh.get(), colData);
}
} // namespace NarrowPhaseCD
} // namespace imstk
