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

#include "imstkPbdObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPBDCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdObjectCollision::PbdObjectCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                       std::string cdType) : CollisionPair(obj1, obj2)
{
    std::shared_ptr<PbdModel> pbdModel1 = obj1->getPbdModel();

    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd =
        makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry());
    setCollisionDetection(cd);

    // Setup the handler
    std::shared_ptr<PBDCollisionHandling> ch = std::make_shared<PBDCollisionHandling>();
    ch->setInputObjectA(obj1);
    ch->setInputObjectB(obj2);
    ch->setInputCollisionData(cd->getCollisionData());
    setCollisionHandlingAB(ch);

    // We want to integrate the positions before detecting collisions
    // then afterwards we want to map the physics geometry to collision so collision geometry is updated
    // before doing detection, lastly we do internal solve, then collision constraint solve
    if (auto pbdObj2 = std::dynamic_pointer_cast<PbdObject>(obj2))
    {
        std::shared_ptr<PbdModel> pbdModel2 = pbdObj2->getPbdModel();

        // Define where collision interaction happens
        m_taskNodeInputs.first.push_back(pbdModel1->getIntegratePositionNode());
        m_taskNodeInputs.second.push_back(pbdModel2->getIntegratePositionNode());

        m_taskNodeOutputs.first.push_back(pbdModel1->getSolveNode());
        m_taskNodeOutputs.second.push_back(pbdModel2->getSolveNode());
    }
    else
    {
        // Define where collision interaction happens
        m_taskNodeInputs.first.push_back(pbdModel1->getIntegratePositionNode());
        m_taskNodeInputs.second.push_back(obj2->getUpdateGeometryNode());

        m_taskNodeOutputs.first.push_back(pbdModel1->getSolveNode());
        m_taskNodeOutputs.second.push_back(obj2->getTaskGraph()->getSink());
    }

    // Setup compute node for collision solver (true/critical node)
    m_collisionSolveNode = std::make_shared<TaskNode>([ch]() { ch->getCollisionSolver()->solve(); },
        obj1->getName() + "_vs_" + obj2->getName() + "_CollisionSolver", true);

    m_correctVelocitiesNode = std::make_shared<TaskNode>([ch]() { ch->correctVelocities(); },
        obj1->getName() + "_vs_" + obj2->getName() + "_VelocityCorrect", true);
}

void
PbdObjectCollision::setRestitution(const double restitution)
{
    std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->setRestitution(restitution);
}

const double
PbdObjectCollision::getRestitution() const
{
    return std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->getRestitution();
}

void
PbdObjectCollision::setFriction(const double friction)
{
    std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->setFriction(friction);
}

const double
PbdObjectCollision::getFriction() const
{
    return std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->getFriction();
}

void
PbdObjectCollision::apply()
{
    // Add the collision solve step (which happens after internal constraint solve)
    std::shared_ptr<TaskGraph> taskGraphA = m_objects.first->getTaskGraph();
    std::shared_ptr<TaskGraph> taskGraphB = m_objects.second->getTaskGraph();

    auto                         pbdObj1 = std::dynamic_pointer_cast<PbdObject>(m_objects.first);
    std::shared_ptr<SceneObject> obj2    = m_objects.second;

    std::shared_ptr<CollisionDetectionAlgorithm> cd = m_colDetect;
    std::shared_ptr<CollisionHandling>           ch = m_colHandlingA; // A=B=AB

    // Solve collision constraints AFTER solving internal constraints of each object
    // but before updating velocities at the end
    taskGraphA->addNode(m_collisionSolveNode);
    taskGraphA->addNode(m_collisionGeometryUpdateNode);
    taskGraphA->addNode(ch->getTaskNode());
    taskGraphA->addNode(m_correctVelocitiesNode);
    taskGraphA->addNode(m_colDetect->getTaskNode());

    // Update Collision Geometry -> Collision Detect -> Collision Handle -> Solve Collision -> Update Pbd Velocity -> Correct Velocity
    taskGraphA->addEdge(pbdObj1->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
    taskGraphA->addEdge(m_collisionGeometryUpdateNode, m_colDetect->getTaskNode());
    taskGraphA->addEdge(m_colDetect->getTaskNode(), ch->getTaskNode());
    taskGraphA->addEdge(ch->getTaskNode(), m_collisionSolveNode);
    taskGraphA->addEdge(m_collisionSolveNode, pbdObj1->getPbdModel()->getUpdateVelocityNode());
    taskGraphA->addEdge(pbdObj1->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
    taskGraphA->addEdge(m_correctVelocitiesNode, pbdObj1->getPbdModel()->getTaskGraph()->getSink());

    if (auto pbdObj2 = std::dynamic_pointer_cast<PbdObject>(obj2))
    {
        taskGraphB->addNode(m_collisionSolveNode);
        taskGraphB->addNode(m_collisionGeometryUpdateNode);
        taskGraphB->addNode(ch->getTaskNode());
        taskGraphB->addNode(m_correctVelocitiesNode);
        taskGraphB->addNode(m_colDetect->getTaskNode());

        taskGraphB->addEdge(pbdObj2->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
        taskGraphB->addEdge(m_collisionGeometryUpdateNode, m_colDetect->getTaskNode());
        taskGraphB->addEdge(m_colDetect->getTaskNode(), ch->getTaskNode());
        taskGraphB->addEdge(ch->getTaskNode(), m_collisionSolveNode);
        taskGraphB->addEdge(m_collisionSolveNode, pbdObj2->getPbdModel()->getUpdateVelocityNode());
        taskGraphB->addEdge(pbdObj2->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
        taskGraphB->addEdge(m_correctVelocitiesNode, pbdObj2->getPbdModel()->getTaskGraph()->getSink());
    }
    else
    {
        taskGraphB->addNode(m_colDetect->getTaskNode());

        taskGraphB->addEdge(obj2->getUpdateGeometryNode(), m_colDetect->getTaskNode());
        taskGraphB->addEdge(m_colDetect->getTaskNode(), obj2->getTaskGraph()->getSink());
    }
}
}