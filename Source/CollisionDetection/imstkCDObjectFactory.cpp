/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/
#include "imstkCDObjectFactory.h"

#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkCapsuleToCapsuleCD.h"
#include "imstkClosedSurfaceMeshToCapsuleCD.h"
#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkCompoundCD.h"
#include "imstkGeometry.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkLineMeshToCapsuleCD.h"
#include "imstkLineMeshToLineMeshCCD.h"
#include "imstkLineMeshToSphereCD.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToCylinderCD.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkSphereToCapsuleCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"
#include "imstkTetraToPointSetCD.h"
#include "imstkUnidirectionalPlaneToCapsuleCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

namespace imstk
{
IMSTK_REGISTER_COLLISION_DETECTION(BidirectionalPlaneToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(CapsuleToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(ClosedSurfaceMeshToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(ClosedSurfaceMeshToMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(ImplicitGeometryToPointSetCD);
IMSTK_REGISTER_COLLISION_DETECTION(ImplicitGeometryToPointSetCCD);
IMSTK_REGISTER_COLLISION_DETECTION(LineMeshToLineMeshCCD);
IMSTK_REGISTER_COLLISION_DETECTION(LineMeshToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(LineMeshToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToCylinderCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToPlaneCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToOrientedBoxCD);
IMSTK_REGISTER_COLLISION_DETECTION(SphereToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(SphereToCylinderCD);
IMSTK_REGISTER_COLLISION_DETECTION(SphereToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToSurfaceMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(TetraToPointSetCD);
IMSTK_REGISTER_COLLISION_DETECTION(TetraToLineMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(UnidirectionalPlaneToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(UnidirectionalPlaneToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(CompoundCD);

// Map types so order does not matter
#define IMSTK_MAP_TYPES(geomA, geomB, cdType)                            \
    { std::string(#geomA) + std::string(#geomB), std::string(#cdType) }, \
    { std::string(#geomB) + std::string(#geomA), std::string(#cdType) }  \


std::shared_ptr<CollisionDetectionAlgorithm>
CDObjectFactory::makeCollisionDetection(const std::string collisionTypeName)
{
    if (collisionTypeName == "MeshToMeshBruteForceCD")
    {
        LOG(WARNING) << "MeshToMeshBruteForceCD deprecated. Using ClosedSurfaceMeshToMeshCD instead.";
        return create("ClosedSurfaceMeshToMeshCD");
    }
    if (!contains(collisionTypeName))
    {
        LOG(FATAL) << "No collision detection type named: " << collisionTypeName;
        return nullptr;
    }
    else
    {
        return create(collisionTypeName);
    }
}

std::string
CDObjectFactory::getCDType(
    const Geometry& obj1,
    const Geometry& obj2)
{
    std::string type1 = obj1.getTypeName();
    std::string type2 = obj2.getTypeName();

    // Map defining collision detection for different geometry types, default options
    // NOTE HS 20221221 changed some AnalyticalGeometry to Mesh CD from
    // ImplicitGeometryToPointSet to PointSetTo<Geometry> due to issues with
    // ImplicitGeoemtryToPointSetCD

    static std::unordered_map<std::string, std::string> cdTypeMap = {
        IMSTK_MAP_TYPES(Plane, Capsule, UnidirectionalPlaneToCapsuleCD),
        IMSTK_MAP_TYPES(Plane, Sphere, BidirectionalPlaneToSphereCD),
        IMSTK_MAP_TYPES(Plane, LineMesh, PointSetToPlaneCD),
        IMSTK_MAP_TYPES(Plane, PointSet, PointSetToPlaneCD),
        IMSTK_MAP_TYPES(Plane, SurfaceMesh, PointSetToPlaneCD),
        IMSTK_MAP_TYPES(Plane, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Plane, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Sphere, Sphere, SphereToSphereCD),
        IMSTK_MAP_TYPES(Sphere, Capsule, SphereToCapsuleCD),
        IMSTK_MAP_TYPES(Sphere, PointSet, PointSetToSphereCD),
        IMSTK_MAP_TYPES(Sphere, LineMesh, LineMeshToSphereCD),
        IMSTK_MAP_TYPES(Sphere, SurfaceMesh, SurfaceMeshToSphereCD),
        IMSTK_MAP_TYPES(Sphere, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Sphere, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Capsule, Capsule, CapsuleToCapsuleCD),
        IMSTK_MAP_TYPES(Capsule, PointSet, PointSetToCapsuleCD),
        IMSTK_MAP_TYPES(Capsule, LineMesh, LineMeshToCapsuleCD),
        IMSTK_MAP_TYPES(Capsule, SurfaceMesh, SurfaceMeshToCapsuleCD),
        // IMSTK_MAP_TYPES(Capsule, SurfaceMesh, ClosedSurfaceMeshToCapsuleCD),
        IMSTK_MAP_TYPES(Capsule, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Capsule, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Cylinder, Sphere, SphereToCylinderCD),
        IMSTK_MAP_TYPES(Cylinder, PointSet, PointSetToCylinderCD),
        IMSTK_MAP_TYPES(Cylinder, LineMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Cylinder, SurfaceMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Cylinder, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(Cylinder, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(OrientedBox, PointSet, PointSetToOrientedBoxCD),
        IMSTK_MAP_TYPES(OrientedBox, LineMesh, PointSetToOrientedBoxCD),
        IMSTK_MAP_TYPES(OrientedBox, SurfaceMesh, PointSetToOrientedBoxCD),
        IMSTK_MAP_TYPES(OrientedBox, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(OrientedBox, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(PointSet, SurfaceMesh, ClosedSurfaceMeshToMeshCD),
        IMSTK_MAP_TYPES(LineMesh, LineMesh, LineMeshToLineMeshCCD),
        IMSTK_MAP_TYPES(LineMesh, SurfaceMesh, ClosedSurfaceMeshToMeshCD),
        IMSTK_MAP_TYPES(SurfaceMesh, SurfaceMesh, ClosedSurfaceMeshToMeshCD),
        IMSTK_MAP_TYPES(TetrahedralMesh, SurfaceMesh, ClosedSurfaceMeshToMeshCD),
        IMSTK_MAP_TYPES(HexahedralMesh, SurfaceMesh, ClosedSurfaceMeshToMeshCD),
        IMSTK_MAP_TYPES(SignedDistanceField, PointSet, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(SignedDistanceField, LineMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(SignedDistanceField, SurfaceMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(SignedDistanceField, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(SignedDistanceField, HexahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(CompositeImplicitGeometry, PointSet, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(CompositeImplicitGeometry, LineMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(CompositeImplicitGeometry, SurfaceMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(CompositeImplicitGeometry, TetrahedralMesh, ImplicitGeometryToPointSetCD),
        IMSTK_MAP_TYPES(CompositeImplicitGeometry, HexahedralMesh, ImplicitGeometryToPointSetCD),

        // CompoundGeometry, uses CompoundCD for all types,
        // the check if CD actually exists is done at runtime
        IMSTK_MAP_TYPES(CompoundGeometry, Plane, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, Sphere, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, Capsule, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, Cylinder, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, OrientedBox, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, PointSet, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, HexahedralMesh, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, LineMesh, CompoundCD),
        IMSTK_MAP_TYPES(CompoundGeometry, SurfaceMesh, CompoundCD)
    };

    if (cdTypeMap.find(type1 + type2) == cdTypeMap.end())
    {
        LOG(INFO) << "No valid collision detection type for : " << type1 + type2;
        return std::string("");
    }

    return cdTypeMap[type1 + type2];
}
} // namespace imstk