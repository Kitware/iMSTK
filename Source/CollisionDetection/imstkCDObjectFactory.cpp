/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCDObjectFactory.h"
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkCapsuleToCapsuleCD.h"
#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkLineMeshToLineMeshCCD.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToCylinderCD.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkSphereToCylinderCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"
#include "imstkTetraToPointSetCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

namespace imstk
{
IMSTK_REGISTER_COLLISION_DETECTION(BidirectionalPlaneToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(CapsuleToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(ClosedSurfaceMeshToMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(ImplicitGeometryToPointSetCD);
IMSTK_REGISTER_COLLISION_DETECTION(ImplicitGeometryToPointSetCCD);
IMSTK_REGISTER_COLLISION_DETECTION(LineMeshToLineMeshCCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToCylinderCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToPlaneCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(PointSetToOrientedBoxCD);
IMSTK_REGISTER_COLLISION_DETECTION(SphereToCylinderCD);
IMSTK_REGISTER_COLLISION_DETECTION(SphereToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToSurfaceMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToCapsuleCD);
IMSTK_REGISTER_COLLISION_DETECTION(SurfaceMeshToSphereCD);
IMSTK_REGISTER_COLLISION_DETECTION(TetraToPointSetCD);
IMSTK_REGISTER_COLLISION_DETECTION(TetraToLineMeshCD);
IMSTK_REGISTER_COLLISION_DETECTION(UnidirectionalPlaneToSphereCD);

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
} // namespace imstk