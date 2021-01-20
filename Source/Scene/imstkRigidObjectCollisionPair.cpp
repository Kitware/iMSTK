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

#include "imstkRigidObjectCollisionPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"
#include "imstkRigidBodyModel2.h"
#include "imstkLogger.h"

namespace imstk
{
namespace expiremental
{
RigidObjectCollisionPair::RigidObjectCollisionPair(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<RigidObject2> obj2,
                                                   CollisionDetection::Type cdType) : CollisionPair(obj1, obj2)
{
    std::shared_ptr<RigidBodyModel2> model1 = obj1->getRigidBodyModel2();
    std::shared_ptr<RigidBodyModel2> model2 = obj2->getRigidBodyModel2();

    if (model1 == nullptr || model2 == nullptr)
    {
        LOG(WARNING) << "RigidObjectCollisionPair, could not create";
        return;
    }

    // Here we use RigidBodyCH which generates constraints for the rigid body model
    // This step is done *after* tentative velocities have been computed but *before*
    // constraints and new velocities are solved.

    // Define where collision interaction happens
    m_taskNodeInputs.first.push_back(model1->getComputeTentativeVelocitiesNode());
    m_taskNodeInputs.second.push_back(model2->getComputeTentativeVelocitiesNode());

    m_taskNodeOutputs.first.push_back(model1->getSolveNode());
    m_taskNodeOutputs.second.push_back(model2->getSolveNode());

    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handler
    CollisionHandling::Side side = CollisionHandling::Side::AB;
    // If both objects use the same model, we need only add one two-way constraint to the system
    if (model1 == model2)
    {
        side = CollisionHandling::Side::AB;
    }
    else
    {
        LOG(WARNING) << "RigidObjectCollisionPair, specified RigidObjects have differing models, one-way constraints not supported yet";
        return;
    }
    auto ch = std::make_shared<RigidBodyCH>(side, m_colData, obj1, obj2);
    setCollisionHandlingAB(ch);
}
}
}