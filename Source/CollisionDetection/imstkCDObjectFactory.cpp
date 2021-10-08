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
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"
#include "imstkTetraToPointSetCD.h"
#include "imstkUnidirectionalPlaneToSphereCD.h"

namespace imstk
{
// Manually register creation functions
std::unordered_map<std::string, CDObjectFactory::CDMakeFunc> CDObjectFactory::cdObjCreationMap =
    std::unordered_map<std::string, CDObjectFactory::CDMakeFunc>();

RegisterCollisionDetection(BidirectionalPlaneToSphereCD);
RegisterCollisionDetection(ImplicitGeometryToPointSetCD);
RegisterCollisionDetection(ImplicitGeometryToPointSetCCD);
RegisterCollisionDetection(MeshToMeshBruteForceCD);
RegisterCollisionDetection(PointSetToCapsuleCD);
RegisterCollisionDetection(PointSetToPlaneCD);
RegisterCollisionDetection(PointSetToSphereCD);
RegisterCollisionDetection(PointSetToOrientedBoxCD);
RegisterCollisionDetection(SphereToCylinderCD);
RegisterCollisionDetection(SphereToSphereCD);
RegisterCollisionDetection(SurfaceMeshToSurfaceMeshCD);
RegisterCollisionDetection(SurfaceMeshToCapsuleCD);
RegisterCollisionDetection(SurfaceMeshToSphereCD);
RegisterCollisionDetection(TetraToPointSetCD);
RegisterCollisionDetection(TetraToLineMeshCD);
RegisterCollisionDetection(UnidirectionalPlaneToSphereCD);

std::shared_ptr<CollisionDetectionAlgorithm>
CDObjectFactory::makeCollisionDetection(const std::string collisionTypeName)
{
    if (cdObjCreationMap.count(collisionTypeName) == 0)
    {
        LOG(WARNING) << "No collision detection type named: " << collisionTypeName;
        return nullptr;
    }
    else
    {
        return cdObjCreationMap.at(collisionTypeName)();
    }
}
}