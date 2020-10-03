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

#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"

// Points to objects
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkPointSetToSurfaceMeshCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"

// Mesh to mesh
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkTetraToTetraCD.h"

// Analytical object to analytical object
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

// Geometry
#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkImplicitGeometry.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#define IMSTK_CHECK_FOR_VALID_GEOMETRIES(obj1, obj2, Type1, Type2) \
    checkValidGeometry<Type1, Type2>(obj1, obj2, #Type1, #Type2);

namespace imstk
{
template<typename T1, typename T2>
void
checkValidGeometry(std::shared_ptr<Geometry> geom1, std::shared_ptr<Geometry> geom2, std::string t1TypeName, std::string t2TypeName)
{
    std::string errorMsg = "Invalid object geometries were provided for the collision detection type.\n";
    bool        fail     = false;
    if (std::dynamic_pointer_cast<T1>(geom1) == nullptr)
    {
        errorMsg += "Geometry 1 expected " + t1TypeName + " but got " + geom1->getTypeName() + ".\n";
        fail      = true;
    }
    if (std::dynamic_pointer_cast<T2>(geom2) == nullptr)
    {
        errorMsg += "Geometry 2 expected " + t2TypeName + " but got " + geom2->getTypeName() + ".\n";
        fail      = true;
    }

    if (fail)
    {
        LOG(FATAL) << errorMsg;
    }
}

std::shared_ptr<CollisionDetection>
makeCollisionDetectionObject(const CollisionDetection::Type type,
                             std::shared_ptr<Geometry>      collidingGeometryA,
                             std::shared_ptr<Geometry>      collidingGeometryB,
                             std::shared_ptr<CollisionData> colData)
{
    if (collidingGeometryA == nullptr)
    {
        LOG(FATAL) << "Collision Geometry A nullptr";
    }
    if (collidingGeometryB == nullptr)
    {
        LOG(FATAL) << "Collision Geometry B nullptr";
    }

    switch (type)
    {
    // Points to objects
    case CollisionDetection::Type::PointSetToSphere:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, PointSet, Sphere);

        auto pointset = std::dynamic_pointer_cast<PointSet>(collidingGeometryA);
        auto sphere   = std::dynamic_pointer_cast<Sphere>(collidingGeometryB);
        return std::make_shared<PointSetToSphereCD>(pointset, sphere, colData);
    }
    case CollisionDetection::Type::PointSetToPlane:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, PointSet, Sphere);

        auto pointset = std::dynamic_pointer_cast<PointSet>(collidingGeometryA);
        auto plane    = std::dynamic_pointer_cast<Plane>(collidingGeometryB);
        return std::make_shared<PointSetToPlaneCD>(pointset, plane, colData);
    }
    case CollisionDetection::Type::PointSetToCapsule:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, PointSet, Capsule);

        auto pointset = std::dynamic_pointer_cast<PointSet>(collidingGeometryA);
        auto capsule  = std::dynamic_pointer_cast<Capsule>(collidingGeometryB);
        return std::make_shared<PointSetToCapsuleCD>(pointset, capsule, colData);
    }
    case CollisionDetection::Type::PointSetToSurfaceMesh:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, PointSet, SurfaceMesh);

        auto pointset = std::dynamic_pointer_cast<PointSet>(collidingGeometryA);
        auto triMesh  = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryB);
        CollisionDetection::addCollisionPairToOctree(collidingGeometryA, collidingGeometryB, type, colData);
        return std::make_shared<PointSetToSurfaceMeshCD>(pointset, triMesh, colData);
    }
    case CollisionDetection::Type::PointSetToImplicit:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, PointSet, ImplicitGeometry);

        auto pointSet     = std::dynamic_pointer_cast<PointSet>(collidingGeometryA);
        auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(collidingGeometryB);
        return std::make_shared<ImplicitGeometryToPointSetCD>(implicitGeom, pointSet, colData);
    }
    // Mesh to mesh
    case CollisionDetection::Type::SurfaceMeshToSurfaceMesh:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, SurfaceMesh, SurfaceMesh);

        auto meshA = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryA);
        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryB);
        CollisionDetection::addCollisionPairToOctree(collidingGeometryA, collidingGeometryB, type, colData);
        return std::make_shared<SurfaceMeshToSurfaceMeshCD>(meshA, meshB, colData);
    }
    case CollisionDetection::Type::SurfaceMeshToSurfaceMeshCCD:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, SurfaceMesh, SurfaceMesh);

        auto meshA = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryA);
        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryB);
        return std::make_shared<SurfaceMeshToSurfaceMeshCCD>(meshA, meshB, colData);
    }

    case CollisionDetection::Type::VolumeMeshToVolumeMesh:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, TetrahedralMesh, TetrahedralMesh);

        auto tet1 = std::dynamic_pointer_cast<TetrahedralMesh>(collidingGeometryA);
        auto tet2 = std::dynamic_pointer_cast<TetrahedralMesh>(collidingGeometryB);
        return std::make_shared<TetraToTetraCD>(tet1, tet2, colData);
    }

    // Analytical object to analytical object
    case CollisionDetection::Type::UnidirectionalPlaneToSphere:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, Plane, Sphere);

        auto plane  = std::dynamic_pointer_cast<Plane>(collidingGeometryA);
        auto sphere = std::dynamic_pointer_cast<Sphere>(collidingGeometryB);
        return std::make_shared<UnidirectionalPlaneToSphereCD>(plane, sphere, colData);
    }
    case CollisionDetection::Type::BidirectionalPlaneToSphere:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, Plane, Sphere);

        auto plane  = std::dynamic_pointer_cast<Plane>(collidingGeometryA);
        auto sphere = std::dynamic_pointer_cast<Sphere>(collidingGeometryB);
        return std::make_shared<BidirectionalPlaneToSphereCD>(plane, sphere, colData);
    }
    case CollisionDetection::Type::SphereToSphere:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, Sphere, Sphere);

        auto sphereA = std::dynamic_pointer_cast<Sphere>(collidingGeometryA);
        auto sphereB = std::dynamic_pointer_cast<Sphere>(collidingGeometryB);
        return std::make_shared<SphereToSphereCD>(sphereA, sphereB, colData);
    }
    case CollisionDetection::Type::SphereToCylinder:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, Cylinder, Sphere);

        auto cylinder = std::dynamic_pointer_cast<Cylinder>(collidingGeometryA);
        auto sphere   = std::dynamic_pointer_cast<Sphere>(collidingGeometryB);
        return std::make_shared<SphereToCylinderCD>(sphere, cylinder, colData);
    }
    case CollisionDetection::Type::MeshToMeshBruteForce:
    {
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(collidingGeometryA, collidingGeometryB, Geometry, SurfaceMesh);

        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(collidingGeometryB);
        return std::make_shared<MeshToMeshBruteForceCD>(collidingGeometryA, meshB, colData);
    }
    default:
    {
        LOG(FATAL) << "makeCollisionDetectionObject error: type not implemented.";
        return nullptr;
    }
    }
}
}