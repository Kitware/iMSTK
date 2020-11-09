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

#include "imstkPbdObjectPickingPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkPBDPickingCH.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
// Pbd Collision will be tested before any step of pbd, then resolved after the solve steps of the two objects
PbdObjectPickingPair::PbdObjectPickingPair(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                           CollisionDetection::Type cdType) : CollisionPair(obj1, obj2)
{
    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handler
    std::shared_ptr<PBDPickingCH> ch = std::make_shared<PBDPickingCH>(CollisionHandling::Side::A, m_colData, obj1, obj2);
    setCollisionHandlingA(ch);
}

void
PbdObjectPickingPair::apply()
{
    // Add the collision interaction
    CollisionPair::apply();
}
}