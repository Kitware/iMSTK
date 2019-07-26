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

#include "imstkCollisionDetection.h"
#include "imstkCollisionData.h"

// Points to objects
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSurfaceMeshCD.h"
#include "imstkPointSetToSpherePickingCD.h"

// Mesh to mesh
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCCD.h"
#include "imstkTetraToTetraCD.h"

// Analytical object to analytical object
#include "imstkUnidirectionalPlaneToSphereCD.h"
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"

#include "imstkCollidingObject.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCylinder.h"
#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <g3log/g3log.hpp>

#define IMSTK_CHECK_FOR_VALID_GEOMETRIES(obj1, obj2)                                           \
    LOG_IF(FATAL, (obj1 == nullptr || obj2 == nullptr)) <<                                     \
        "CollisionDetection::makeCollisionDetectionObject() error: " <<                        \
        "Invalid object geometries were provided for the given collision detection type.\n" << \
        "    Line: " << __LINE__ << ", File: " << __FILE__;

namespace imstk
{
std::shared_ptr<CollisionDetection>
CollisionDetection::makeCollisionDetectionObject(const Type                       type,
                                                 std::shared_ptr<CollidingObject> objA,
                                                 std::shared_ptr<CollidingObject> objB,
                                                 std::shared_ptr<CollisionData>   colData)
{
    switch (type)
    {
    // Points to objects
    case Type::PointSetToSphere:
    {
        auto pointset = std::dynamic_pointer_cast<PointSet>(objA->getCollidingGeometry());
        auto sphere   = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(pointset, sphere)
        return std::make_shared<PointSetToSphereCD>(pointset, sphere, colData);
    }
    case Type::PointSetToPlane:
    {
        auto pointset = std::dynamic_pointer_cast<PointSet>(objA->getCollidingGeometry());
        auto plane    = std::dynamic_pointer_cast<Plane>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(pointset, plane)
        return std::make_shared<PointSetToPlaneCD>(pointset, plane, colData);
    }
    case Type::PointSetToCapsule:
    {
        auto pointset = std::dynamic_pointer_cast<PointSet>(objA->getCollidingGeometry());
        auto capsule  = std::dynamic_pointer_cast<Capsule>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(pointset, capsule)
        return std::make_shared<PointSetToCapsuleCD>(pointset, capsule, colData);
    }
    case Type::PointSetToSpherePicking:
    {
        auto pointset = std::dynamic_pointer_cast<PointSet>(objA->getCollidingGeometry());
        auto sphere   = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(pointset, sphere)
        return std::make_shared<PointSetToSpherePickingCD>(pointset, sphere, colData);
    }
    case Type::PointSetToSurfaceMesh:
    {
        auto pointset = std::dynamic_pointer_cast<PointSet>(objA->getCollidingGeometry());
        auto triMesh  = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(pointset, triMesh)
        return std::make_shared<PointSetToSurfaceMeshCD>(pointset, triMesh, colData);
    }
    // Mesh to mesh
    case Type::SurfaceMeshToSurfaceMesh:
    {
        auto meshA = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry());
        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(meshA, meshB)
        return std::make_shared<SurfaceMeshToSurfaceMeshCD>(meshA, meshB, colData);
    }
    case Type::SurfaceMeshToSurfaceMeshCCD:
    {
        auto meshA = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry());
        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(meshA, meshB)
        return std::make_shared<SurfaceMeshToSurfaceMeshCCD>(meshA, meshB, colData);
    }

    case Type::VolumeMeshToVolumeMesh:
    {
        auto tet1 = std::dynamic_pointer_cast<TetrahedralMesh>(objA->getCollidingGeometry());
        auto tet2 = std::dynamic_pointer_cast<TetrahedralMesh>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(tet1, tet2)
        return std::make_shared<TetraToTetraCD>(tet1, tet2, colData);
    }

    // Analytical object to analytical object
    case Type::UnidirectionalPlaneToSphere:
    {
        auto plane  = std::dynamic_pointer_cast<Plane>(objA->getCollidingGeometry());
        auto sphere = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(plane, sphere)
        return std::make_shared<UnidirectionalPlaneToSphereCD>(plane, sphere, colData);
    }
    case Type::BidirectionalPlaneToSphere:
    {
        auto plane  = std::dynamic_pointer_cast<Plane>(objA->getCollidingGeometry());
        auto sphere = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(plane, sphere)
        return std::make_shared<BidirectionalPlaneToSphereCD>(plane, sphere, colData);
    }
    case Type::SphereToSphere:
    {
        auto sphereA = std::dynamic_pointer_cast<Sphere>(objA->getCollidingGeometry());
        auto sphereB = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(sphereA, sphereB)
        return std::make_shared<SphereToSphereCD>(sphereA, sphereB, colData);
    }
    case Type::SphereToCylinder:
    {
        auto sphere   = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());
        auto cylinder = std::dynamic_pointer_cast<Cylinder>(objA->getCollidingGeometry());
        IMSTK_CHECK_FOR_VALID_GEOMETRIES(sphere, cylinder)
        return std::make_shared<SphereToCylinderCD>(sphere, cylinder, colData);
    }

    default:
    {
        LOG(FATAL) << "CollisionDetection::makeCollisionDetectionObject error: type not implemented.";
        return nullptr;
    }
    }
}

CollisionDetection::CollisionDetection(const CollisionDetection::Type& type, std::shared_ptr<CollisionData> colData) : m_type(type)
{
    m_colData = (colData == nullptr) ? std::make_shared<CollisionData>() : colData;
}
}
