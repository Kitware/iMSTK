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