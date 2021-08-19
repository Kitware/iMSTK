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
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkTetraToPointSetCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

#define IMSTK_MAKE_CD_CASE(cdType, geomType1, geomType2)                            \
    { #cdType, [](std::shared_ptr<Geometry> geomA, std::shared_ptr<Geometry> geomB) \
      {                                                                             \
          std::shared_ptr<cdType> cd = std::make_shared<cdType>();                  \
          cd->setInput(geomA, 0);                                                   \
          cd->setInput(geomB, 1);                                                   \
          return cd;                                                                \
      }                                                                             \
    }

namespace imstk
{
// Manually register creation functions
// \todo: (switch to in class allocation's with base class register later)
std::unordered_map<std::string, std::function<std::shared_ptr<CollisionDetectionAlgorithm>(std::shared_ptr<Geometry>, std::shared_ptr<Geometry>)>> cdMakeFunctions =
{
    IMSTK_MAKE_CD_CASE(BidirectionalPlaneToSphereCD, Plane, Sphere),
    IMSTK_MAKE_CD_CASE(ImplicitGeometryToPointSetCD, ImplicitGeometry, PointSet),
    IMSTK_MAKE_CD_CASE(ImplicitGeometryToPointSetCCD, ImplicitGeometry, PointSet),
    IMSTK_MAKE_CD_CASE(MeshToMeshBruteForceCD, PointSet, SurfaceMesh),
    IMSTK_MAKE_CD_CASE(PointSetToCapsuleCD, PointSet, Capsule),
    IMSTK_MAKE_CD_CASE(PointSetToPlaneCD, PointSet, Plane),
    IMSTK_MAKE_CD_CASE(PointSetToSphereCD, PointSet, Sphere),
    IMSTK_MAKE_CD_CASE(PointSetToOrientedBoxCD, PointSet, OrientedBox),
    IMSTK_MAKE_CD_CASE(SphereToCylinderCD, Sphere, Cylinder),
    IMSTK_MAKE_CD_CASE(SphereToSphereCD, Sphere, Sphere),
    IMSTK_MAKE_CD_CASE(SurfaceMeshToSurfaceMeshCD, SurfaceMesh, SurfaceMesh),
    IMSTK_MAKE_CD_CASE(SurfaceMeshToCapsuleCD, SurfaceMesh, Capsule),
    IMSTK_MAKE_CD_CASE(SurfaceMeshToSphereCD, SurfaceMesh, Sphere),
    IMSTK_MAKE_CD_CASE(TetraToPointSetCD, TetrahedralMesh, PointSet),
    IMSTK_MAKE_CD_CASE(UnidirectionalPlaneToSphereCD, Plane, Sphere)
};

std::shared_ptr<CollisionDetectionAlgorithm>
makeCollisionDetectionObject(const std::string         collisionTypeName,
                             std::shared_ptr<Geometry> collidingGeometryA,
                             std::shared_ptr<Geometry> collidingGeometryB)
{
    if (collidingGeometryA == nullptr)
    {
        LOG(FATAL) << "Collision Geometry A nullptr";
    }
    if (collidingGeometryB == nullptr)
    {
        LOG(FATAL) << "Collision Geometry B nullptr";
    }

    if (cdMakeFunctions.count(collisionTypeName) == 0)
    {
        LOG(WARNING) << "No collision detection type named: " << collisionTypeName;
        return nullptr;
    }
    else
    {
        return cdMakeFunctions[collisionTypeName](collidingGeometryA, collidingGeometryB);
    }
}
}