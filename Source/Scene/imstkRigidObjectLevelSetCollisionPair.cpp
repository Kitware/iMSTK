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

#include "imstkRigidObjectLevelSetCollisionPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"

namespace imstk
{
namespace expiremental
{
RigidObjectLevelSetCollisionPair::RigidObjectLevelSetCollisionPair(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<LevelSetDeformableObject> obj2) :
    CollisionPair(obj1, obj2)
{
    std::shared_ptr<RigidBodyModel2> rbdModel    = obj1->getRigidBodyModel2();
    std::shared_ptr<LevelSetModel>   lvlSetModel = obj2->getLevelSetModel();

    if (rbdModel == nullptr || lvlSetModel == nullptr)
    {
        LOG(WARNING) << "RigidObjectCollisionPair, could not create";
        return;
    }

    // Graph of explicit interaction
    // [compute tentative velocities]  [lvlSetSource]
    //                           [CD]
    //     [Add rigid constraints]    [Apply impulses]
    //       [Solve rbd system]       [Evolve levelset]
    // Here the CH's adds constraints to the system on the LHS, and impulses to the levelset RHS

    // Define where collision interaction happens
    m_taskNodeInputs.first.push_back(rbdModel->getComputeTentativeVelocitiesNode());
    m_taskNodeInputs.second.push_back(lvlSetModel->getTaskGraph()->getSource());

    m_taskNodeOutputs.first.push_back(rbdModel->getSolveNode());
    m_taskNodeOutputs.second.push_back(lvlSetModel->getQuantityEvolveNode(0));

    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(CollisionDetection::Type::PointSetToImplicit, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handlers for each side
    setCollisionHandlingA(std::make_shared<RigidBodyCH>(CollisionHandling::Side::A, m_colData, obj1, nullptr, 0.0, 0.0));
    setCollisionHandlingB(std::make_shared<LevelSetCH>(CollisionHandling::Side::B, m_colData, obj2, obj1));
}
}
}