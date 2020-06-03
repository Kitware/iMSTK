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

#include "imstkObjectInteractionFactory.h"
#include "imstkBoneDrillingCH.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetection.h"
#include "imstkDeformableObject.h"
#include "imstkLogger.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollisionPair.h"
#include "imstkPenaltyCH.h"
#include "imstkPickingCH.h"
#include "imstkSPHCollisionHandling.h"
#include "imstkSPHObject.h"

#include <memory>

namespace imstk
{
// Predefined standard/types of interaction from imstk
enum class InteractionType
{
    PbdObjToPbdObj_Collision,

    PbdObjToCollidingObj_Collision,
    SphObjToCollidingObj_Collision,
    FemObjToCollidingObj_Collision,
    //RigidObjToCollidingObj_Collision,

    //RigidObjToRigidObj_Collision,

    FemObjToCollidingObj_PenaltyForce,
    FemObjToCollidingObj_BoneDrilling,
    FemObjToCollidingObj_NodalPicking
};

///
/// \brief Factory for InteractionPairs
///
template<typename ObjectType1, typename ObjectType2>
extern std::shared_ptr<ObjectInteractionPair>
makeObjectInteractionPair(
    std::shared_ptr<ObjectType1> obj1, std::shared_ptr<ObjectType2> obj2,
    InteractionType intType, CollisionDetection::Type cdType)
{
    std::shared_ptr<ObjectInteractionPair> results = nullptr;
    if (intType == InteractionType::PbdObjToPbdObj_Collision)
    {
        if (std::is_base_of<PbdObject, ObjectType1>::value
            && std::is_base_of<PbdObject, ObjectType2>::value)
        {
            results = std::make_shared<PbdObjectCollisionPair>(
                std::dynamic_pointer_cast<PbdObject>(obj1),
                std::dynamic_pointer_cast<PbdObject>(obj2), cdType);
        }
    }
    /*else if (intType == InteractionType::PbdObjToCollidingObj_Collision)
    {
        if (std::is_base_of<PbdObject, ObjectType1>::value &&
            std::is_base_of<CollidingObject, ObjectType2>::value)
        {
            results = std::make_shared<PbdCollidingObjCollisionPair>(
                std::dynamic_pointer_cast<PbdObject>(obj1),
                std::dynamic_pointer_cast<PbdObject>(obj2), cdType);
        }
    }*/
    else if (intType == InteractionType::SphObjToCollidingObj_Collision)
    {
        if (std::is_base_of<SPHObject, ObjectType1>::value
            && std::is_base_of<CollidingObject, ObjectType2>::value)
        {
            // Setup CD, and collision data
            std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
            std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

            // Setup the handler
            std::shared_ptr<SPHCollisionHandling> colHandler =
                std::make_shared<SPHCollisionHandling>(CollisionHandling::Side::A, colData, std::dynamic_pointer_cast<SPHObject>(obj1));

            results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
        }
    }
    else if (intType == InteractionType::FemObjToCollidingObj_NodalPicking)
    {
        if (std::is_base_of<FeDeformableObject, ObjectType1>::value
            && std::is_base_of<CollidingObject, ObjectType2>::value)
        {
            // Setup CD, and collision data
            std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
            std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

            // Setup the handler
            std::shared_ptr<PickingCH> colHandler =
                std::make_shared<PickingCH>(CollisionHandling::Side::A, colData, std::dynamic_pointer_cast<FeDeformableObject>(obj1));

            results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
        }
    }
    else if (intType == InteractionType::FemObjToCollidingObj_PenaltyForce)
    {
        if ((std::is_base_of<CollidingObject, ObjectType1>::value)
            && std::is_base_of<CollidingObject, ObjectType2>::value)
        {
            // Setup CD, and collision data
            std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
            std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

            // Setup the handler
            std::shared_ptr<PenaltyCH> colHandler =
                std::make_shared<PenaltyCH>(CollisionHandling::Side::A, colData, obj1);

            results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
        }
    }
    else if (intType == InteractionType::FemObjToCollidingObj_BoneDrilling)
    {
        if (std::is_base_of<FeDeformableObject, ObjectType1>::value
            && std::is_base_of<CollidingObject, ObjectType2>::value)
        {
            // Setup CD, and collision data
            std::shared_ptr<CollisionData>      colData   = std::make_shared<CollisionData>();
            std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), colData);

            // Setup the handler
            std::shared_ptr<BoneDrillingCH> colHandler =
                std::make_shared<BoneDrillingCH>(CollisionHandling::Side::A, colData, obj1, obj2);

            results = std::make_shared<CollisionPair>(obj1, obj2, colDetect, colHandler, nullptr);
        }
    }

    if (results == nullptr)
    {
        LOG(WARNING) << "Invalid SceneObjects for requested interaction.";
    }
    return results;
}
}