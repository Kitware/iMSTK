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

#include "imstkObjectInteractionFactory.h"
#include "imstkBoneDrillingCH.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkFeDeformableObject.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollisionPair.h"
#include "imstkPbdObjectPickingPair.h"
#include "imstkPBDPickingCH.h"
#include "imstkPenaltyCH.h"
#include "imstkPickingCH.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkSPHCollisionHandling.h"
#include "imstkSPHObject.h"

namespace imstk
{
// Cast type check
template<typename ObjectType>
static bool
isType(std::shared_ptr<SceneObject> obj)
{
    return std::dynamic_pointer_cast<ObjectType>(obj) != nullptr;
}

std::shared_ptr<ObjectInteractionPair>
makeObjectInteractionPair(std::shared_ptr<CollidingObject> obj1, std::shared_ptr<CollidingObject> obj2,
                          InteractionType intType, CollisionDetection::Type cdType)
{
    std::shared_ptr<ObjectInteractionPair> results = nullptr;
    if (intType == InteractionType::PbdObjToPbdObjCollision && isType<PbdObject>(obj1) && isType<PbdObject>(obj2))
    {
        results = std::make_shared<PbdObjectCollisionPair>(std::dynamic_pointer_cast<PbdObject>(obj1), std::dynamic_pointer_cast<PbdObject>(obj2), cdType);
    }
    else if (intType == InteractionType::PbdObjToCollidingObjPicking && isType<PbdObject>(obj1))
    {
        results = std::make_shared<PbdObjectPickingPair>(std::dynamic_pointer_cast<PbdObject>(obj1), std::dynamic_pointer_cast<CollidingObject>(obj2), cdType);
    }
    //else if (intType == InteractionType::PbdObjToCollidingObjCollision && isType<PbdObject>(obj1))
    //{
    //    results = std::make_shared<PbdCollidingObjCollisionPair>(
    //        std::dynamic_pointer_cast<PbdObject>(obj1),
    //        std::dynamic_pointer_cast<PbdObject>(obj2), cdType);
    //}
    else if (intType == InteractionType::SphObjToCollidingObjCollision && isType<SPHObject>(obj1))
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

        // Setup the handler
        std::shared_ptr<SPHCollisionHandling> colHandler =
            std::make_shared<SPHCollisionHandling>(CollisionHandling::Side::A, colData, std::dynamic_pointer_cast<SPHObject>(obj1));

        results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
    }
    else if (intType == InteractionType::SphObjToCollidingObjSDFCollision && isType<SPHObject>(obj1))
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

        // Setup the handler
        std::shared_ptr<SPHCollisionHandling> colHandler = std::make_shared<SPHCollisionHandling>(CollisionHandling::Side::B, colData, std::dynamic_pointer_cast<SPHObject>(obj1));
        results = std::make_shared<CollisionPair>(obj2, obj1, colDetect, colHandler, nullptr);
    }
    else if (intType == InteractionType::FemObjToCollidingObjNodalPicking && isType<FeDeformableObject>(obj1))
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

        // Setup the handler
        std::shared_ptr<PickingCH> colHandler =
            std::make_shared<PickingCH>(CollisionHandling::Side::A, colData, std::dynamic_pointer_cast<FeDeformableObject>(obj1));

        results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
    }
    else if (intType == InteractionType::FemObjToCollidingObjPenaltyForce && isType<FeDeformableObject>(obj1))
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

        // Setup the handler
        std::shared_ptr<PenaltyCH> colHandler =
            std::make_shared<PenaltyCH>(CollisionHandling::Side::A, colData, obj1);
    }
    else if (intType == InteractionType::CollidingObjToCollidingObjBoneDrilling)
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

        // Setup the handler
        std::shared_ptr<BoneDrillingCH> colHandler =
            std::make_shared<BoneDrillingCH>(CollisionHandling::Side::A, colData, obj1, obj2);

        results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
    }

    if (results == nullptr)
    {
        LOG(WARNING) << "Invalid SceneObjects for requested interaction.";
    }
    return results;
}
}