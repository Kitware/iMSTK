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
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdObjectCollision::PbdObjectCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                       std::string cdType) :
    CollisionInteraction("PbdObjectCollision_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2)
{
    std::shared_ptr<PbdModel> pbdModel1 = obj1->getPbdModel();

    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    cd->setInput(obj1->getCollidingGeometry(), 0);
    cd->setInput(obj2->getCollidingGeometry(), 1);
    setCollisionDetection(cd);

    // Setup the handler
    std::shared_ptr<PbdCollisionHandling> ch = std::make_shared<PbdCollisionHandling>();
    ch->setInputObjectA(obj1);
    ch->setInputObjectB(obj2);
    ch->setInputCollisionData(cd->getCollisionData());
    setCollisionHandlingAB(ch);

    // Setup collision constraint solve step, should occur after internal constraint solve
    m_collisionSolveNode = std::make_shared<TaskNode>([&]()
        {
            auto pbdCh = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingAB());
            if (pbdCh == nullptr)
            {
                LOG(FATAL) << "Invalid handler type, must be of type PbdCollisionHandling on " <<
                m_name << " collision solve";
            }
            pbdCh->getCollisionSolver()->solve();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_CollisionSolver", true);
    m_taskGraph->addNode(m_collisionSolveNode);

    // Setup a step to correct velocities for restitution & friction after the PBD velocity computation
    m_correctVelocitiesNode = std::make_shared<TaskNode>([&]()
        {
            auto pbdCh = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingAB());
            if (pbdCh == nullptr)
            {
                LOG(FATAL) << "Invalid handler type, must be of type PbdCollisionHandling on " <<
                m_name << " correct velocities";
            }
            pbdCh->correctVelocities();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_VelocityCorrect", true);
    m_taskGraph->addNode(m_correctVelocitiesNode);

    if (auto pbdObj2 = std::dynamic_pointer_cast<PbdObject>(obj2))
    {
        std::shared_ptr<PbdModel> pbdModel2 = pbdObj2->getPbdModel();
        m_taskGraph->addNode(pbdModel2->getIntegratePositionNode());
        m_taskGraph->addNode(pbdModel2->getUpdateVelocityNode());
        m_taskGraph->addNode(pbdModel2->getSolveNode());
        m_taskGraph->addNode(pbdModel2->getTaskGraph()->getSink());
    }
    else
    {
        m_taskGraph->addNode(obj2->getUpdateGeometryNode());
        m_taskGraph->addNode(obj2->getTaskGraph()->getSink());
    }

    m_taskGraph->addNode(pbdModel1->getIntegratePositionNode());
    m_taskGraph->addNode(pbdModel1->getUpdateVelocityNode());
    m_taskGraph->addNode(pbdModel1->getSolveNode());
    m_taskGraph->addNode(pbdModel1->getTaskGraph()->getSink());
}

void
PbdObjectCollision::setRestitution(const double restitution)
{
    std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->setRestitution(restitution);
}

const double
PbdObjectCollision::getRestitution() const
{
    return std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->getRestitution();
}

void
PbdObjectCollision::setFriction(const double friction)
{
    std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->setFriction(friction);
}

const double
PbdObjectCollision::getFriction() const
{
    return std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->getFriction();
}

void
PbdObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                         pbdObj1 = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<SceneObject> obj2    = m_objB;

    std::shared_ptr<TaskNode> chNodeAB = m_collisionHandleANode;

    // -------------------------------------------------------------------------
    // Internal Constraint Solve -> Collision Geometry Update -> Collision Detection ->
    // PbdHandlerAB -> Collision Constraint Solve -> Update Pbd Velocity -> Correct
    // Velocities for Collision (restitution+friction) -> Pbd Sink
    // -------------------------------------------------------------------------
    m_taskGraph->addEdge(pbdObj1->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
    m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, chNodeAB); // A=AB=B
    m_taskGraph->addEdge(chNodeAB, m_collisionSolveNode);
    m_taskGraph->addEdge(m_collisionSolveNode, pbdObj1->getPbdModel()->getUpdateVelocityNode());
    m_taskGraph->addEdge(pbdObj1->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
    m_taskGraph->addEdge(m_correctVelocitiesNode, pbdObj1->getPbdModel()->getTaskGraph()->getSink());

    if (auto pbdObj2 = std::dynamic_pointer_cast<PbdObject>(obj2))
    {
        m_taskGraph->addEdge(pbdObj2->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
        m_taskGraph->addEdge(m_collisionSolveNode, pbdObj2->getPbdModel()->getUpdateVelocityNode());
        m_taskGraph->addEdge(pbdObj2->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
        m_taskGraph->addEdge(m_correctVelocitiesNode, pbdObj2->getPbdModel()->getTaskGraph()->getSink());
    }
    else
    {
        m_taskGraph->addEdge(obj2->getUpdateGeometryNode(), m_collisionGeometryUpdateNode);
        m_taskGraph->addEdge(m_collisionDetectionNode, obj2->getTaskGraph()->getSink());
    }
}
} // namespace imstk