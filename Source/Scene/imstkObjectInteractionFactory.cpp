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
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectCuttingPair.h"
#include "imstkPbdObjectPicking.h"
#include "imstkPBDPickingCH.h"
#include "imstkPbdRigidObjectCollision.h"
#include "imstkPenaltyCH.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkSPHCollisionHandling.h"
#include "imstkSPHObject.h"
#include "imstkSphObjectCollision.h"

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
                          InteractionType intType, std::string cdType)
{
    std::shared_ptr<ObjectInteractionPair> results = nullptr;
    if (intType == InteractionType::PbdObjCollision)
    {
        if (isType<PbdObject>(obj1) && isType<PbdObject>(obj2))
        {
            results = std::make_shared<PbdObjectCollision>(std::dynamic_pointer_cast<PbdObject>(obj1), std::dynamic_pointer_cast<PbdObject>(obj2), cdType);
        }
        else if (isType<CollidingObject>(obj1) && isType<PbdObject>(obj2))
        {
            results = std::make_shared<PbdObjectCollision>(std::dynamic_pointer_cast<PbdObject>(obj2), obj1, cdType);
        }
        else if (isType<PbdObject>(obj1) && isType<CollidingObject>(obj2))
        {
            results = std::make_shared<PbdObjectCollision>(std::dynamic_pointer_cast<PbdObject>(obj1), obj2, cdType);
        }
    }
    else if (intType == InteractionType::RbdObjCollision)
    {
        if (isType<RigidObject2>(obj1) && isType<CollidingObject>(obj2))
        {
            results = std::make_shared<RigidObjectCollision>(std::dynamic_pointer_cast<RigidObject2>(obj1), obj2, cdType);
        }
        else if (isType<CollidingObject>(obj1) && isType<RigidObject2>(obj2))
        {
            results = std::make_shared<RigidObjectCollision>(std::dynamic_pointer_cast<RigidObject2>(obj2), obj1, cdType);
        }
    }
    else if (intType == InteractionType::RbdObjCollision)
    {
        if (isType<RigidObject2>(obj2) && isType<PbdObject>(obj1))
        {
            results = std::make_shared<PbdRigidObjectCollision>(std::dynamic_pointer_cast<PbdObject>(obj1), std::dynamic_pointer_cast<RigidObject2>(obj2), cdType);
        }
        else if (isType<PbdObject>(obj1) && isType<RigidObject2>(obj2))
        {
            results = std::make_shared<PbdRigidObjectCollision>(std::dynamic_pointer_cast<PbdObject>(obj2), std::dynamic_pointer_cast<RigidObject2>(obj1), cdType);
        }
    }
    else if (intType == InteractionType::PbdObj2dCutting && isType<PbdObject>(obj1))
    {
        results = std::make_shared<PbdObjectCuttingPair>(std::dynamic_pointer_cast<PbdObject>(obj1), obj2);
    }
    else if (intType == InteractionType::PbdObjPicking && isType<PbdObject>(obj1))
    {
        results = std::make_shared<PbdObjectPicking>(std::dynamic_pointer_cast<PbdObject>(obj1), obj2, cdType);
    }
    else if (intType == InteractionType::SphObjCollision)
    {
        if (isType<SPHObject>(obj1) && isType<CollidingObject>(obj2))
        {
            results = std::make_shared<SphObjectCollision>(std::dynamic_pointer_cast<SPHObject>(obj1), obj2, cdType);
        }
        else if (isType<SPHObject>(obj2) && isType<CollidingObject>(obj1))
        {
            results = std::make_shared<SphObjectCollision>(std::dynamic_pointer_cast<SPHObject>(obj2), obj1, cdType);
        }
    }
    else if (intType == InteractionType::BoneDrilling)
    {
        // Setup CD and collision data
        std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
        cd->setInput(obj1->getCollidingGeometry(), 0);
        cd->setInput(obj2->getCollidingGeometry(), 1);

        // Setup the handler
        std::shared_ptr<BoneDrillingCH> ch = std::make_shared<BoneDrillingCH>();
        ch->setInputObjectBone(obj1);
        ch->setInputObjectDrill(std::dynamic_pointer_cast<RigidObject2>(obj2));
        ch->setInputCollisionData(cd->getCollisionData());

        results = std::make_shared<CollisionPair>(obj1, obj2, cd, ch, nullptr);
    }

    if (results == nullptr)
    {
        LOG(WARNING) << "Invalid Interaction.";
    }
    return results;
}
}