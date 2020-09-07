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

#include "imstkRigidObjectCollidingCollisionPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"

namespace imstk
{
namespace expiremental
{
RigidObjectCollidingCollisionPair::RigidObjectCollidingCollisionPair(
    std::shared_ptr<RigidObject2> obj1, std::shared_ptr<CollidingObject> obj2,
    CollisionDetection::Type cdType, const double stiffness, const double frictionalCoefficient) :
    CollisionPair(obj1, obj2)
{
    std::shared_ptr<RigidBodyModel2> rbdModel = obj1->getRigidBodyModel2();

    if (rbdModel == nullptr)
    {
        LOG(WARNING) << "RigidCollidingObjectCollisionPair, could not create";
        return;
    }

    // Define where collision interaction happens
    m_taskNodeInputs.first.push_back(rbdModel->getComputeTentativeVelocitiesNode());
    m_taskNodeInputs.second.push_back(obj2->getTaskGraph()->getSource());

    m_taskNodeOutputs.first.push_back(rbdModel->getSolveNode());
    m_taskNodeOutputs.second.push_back(obj2->getUpdateNode());

    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handlers for each side
    setCollisionHandlingA(std::make_shared<RigidBodyCH>(CollisionHandling::Side::A, m_colData, obj1, nullptr, stiffness, frictionalCoefficient));
}
}
}