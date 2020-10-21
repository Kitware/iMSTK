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

#include "imstkCollisionDetection.h"

namespace imstk
{
class CollidingObject;
class ObjectInteractionPair;

// Predefined standard/types of interaction from imstk
enum class InteractionType
{
    PbdObjToPbdObjCollision,

    PbdObjToCollidingObjCollision,
    SphObjToCollidingObjCollision,
    SphObjToCollidingObjSDFCollision,
    FemObjToCollidingObjCollision,
    //RigidObjToCollidingObjCollision,

    //RigidObjToRigidObjCollision,

    FemObjToCollidingObjPenaltyForce,
    FemObjToCollidingObjBoneDrilling,
    FemObjToCollidingObjNodalPicking,
    SphObjToPhysiologyObjCoupling,
    CollidingObjToCollidingObjBoneDrilling
};

///
/// \brief Factory for InteractionPairs, returns nullptr and logs warning if failed
///
std::shared_ptr<ObjectInteractionPair> makeObjectInteractionPair(std::shared_ptr<CollidingObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                                                 InteractionType intType, CollisionDetection::Type cdType);
}
