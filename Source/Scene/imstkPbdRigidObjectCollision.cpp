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

#include "imstkPbdRigidObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdRigidObjectCollision::PbdRigidObjectCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<RigidObject2> obj2,
                                                 std::string cdType) :
    CollisionInteraction("PbdRigidObjectCollision" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2)
{
    std::shared_ptr<PbdModel> pbdModel1 = obj1->getPbdModel();

    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    cd->setInput(obj1->getCollidingGeometry(), 0);
    cd->setInput(obj2->getCollidingGeometry(), 1);
    setCollisionDetection(cd);

    // Setup the handler to resolve obj1
    auto pbdCH = std::make_shared<PbdCollisionHandling>();
    pbdCH->setInputObjectA(obj1);
    pbdCH->setInputObjectB(obj2);
    pbdCH->setInputCollisionData(cd->getCollisionData());
    setCollisionHandlingA(pbdCH);

    auto rbdCH = std::make_shared<RigidBodyCH>();
    rbdCH->setInputRigidObjectA(obj2);
    rbdCH->setInputCollidingObjectB(obj1);
    rbdCH->setInputCollisionData(cd->getCollisionData());
    rbdCH->setBeta(0.1);
    setCollisionHandlingB(rbdCH);

    // Setup compute node for collision solver (true/critical node)
    m_pbdCollisionSolveNode = std::make_shared<TaskNode>([&]()
        {
            std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->getCollisionSolver()->solve();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_PBDCollisionSolver", true);
    m_taskGraph->addNode(m_pbdCollisionSolveNode);

    m_correctVelocitiesNode = std::make_shared<TaskNode>([&]()
        {
            std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->correctVelocities();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_PBDVelocityCorrect", true);
    m_taskGraph->addNode(m_correctVelocitiesNode);

    // Nodes from objectA
    auto pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objA);
    m_taskGraph->addNode(pbdObj->getPbdModel()->getTaskGraph()->getSource());
    m_taskGraph->addNode(pbdObj->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(pbdObj->getPbdModel()->getUpdateVelocityNode());
    m_taskGraph->addNode(pbdObj->getPbdModel()->getTaskGraph()->getSink());

    // Nodes from objectB
    auto rbdObj = std::dynamic_pointer_cast<RigidObject2>(m_objB);
    m_taskGraph->addNode(rbdObj->getRigidBodyModel2()->getTaskGraph()->getSource());
    m_taskGraph->addNode(rbdObj->getRigidBodyModel2()->getComputeTentativeVelocitiesNode());
    m_taskGraph->addNode(rbdObj->getRigidBodyModel2()->getSolveNode());
    m_taskGraph->addNode(rbdObj->getRigidBodyModel2()->getIntegrateNode());
    m_taskGraph->addNode(rbdObj->getRigidBodyModel2()->getTaskGraph()->getSink());
}

void
PbdRigidObjectCollision::setRestitution(const double restitution)
{
    std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->setRestitution(restitution);
}

const double
PbdRigidObjectCollision::getRestitution() const
{
    return std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->getRestitution();
}

void
PbdRigidObjectCollision::setFriction(const double friction)
{
    std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->setFriction(friction);
}

const double
PbdRigidObjectCollision::getFriction() const
{
    return std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA())->getFriction();
}

void
PbdRigidObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Add the collision solve step (which happens after internal constraint solve)
    std::shared_ptr<TaskNode> pbdHandlerNode = m_collisionHandleANode;
    std::shared_ptr<TaskNode> rbdHandlerNode = m_collisionHandleBNode;

    // Because pbd solves directly on positions it would cause a race condition
    // if we were to solve rbd and pbd at the same time. Pbd won't write to the
    // rigid body positions, but it will read them.
    // We solve rigid body before pbd, this way pbd has the most up to date positions
    // semi-implicit

    auto pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objA);
    auto rbdObj = std::dynamic_pointer_cast<RigidObject2>(m_objB);

    m_taskGraph->addEdge(source, pbdObj->getPbdModel()->getTaskGraph()->getSource());
    m_taskGraph->addEdge(source, rbdObj->getRigidBodyModel2()->getTaskGraph()->getSource());
    m_taskGraph->addEdge(pbdObj->getPbdModel()->getTaskGraph()->getSink(), sink);
    m_taskGraph->addEdge(rbdObj->getRigidBodyModel2()->getTaskGraph()->getSink(), sink);

    std::shared_ptr<CollisionHandling> pbdCH = m_colHandlingA;
    {
        // InternalConstraint Solve -> Update Collision Geometry ->
        // Collision Detect -> Collision Handle -> Solve Collision ->
        // Update Pbd Velocity -> Correct Velocity -> PbdModelSink
        m_taskGraph->addEdge(pbdObj->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
        m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
        m_taskGraph->addEdge(m_collisionDetectionNode, pbdHandlerNode);
        m_taskGraph->addEdge(pbdHandlerNode, m_pbdCollisionSolveNode);
        m_taskGraph->addEdge(m_pbdCollisionSolveNode, pbdObj->getPbdModel()->getUpdateVelocityNode());
        m_taskGraph->addEdge(pbdObj->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
        m_taskGraph->addEdge(m_correctVelocitiesNode, pbdObj->getPbdModel()->getTaskGraph()->getSink());
    }

    std::shared_ptr<CollisionHandling> rbdCH = m_colHandlingB;
    {
        // Compute Tentative Velocities -> Collision Detect ->
        // Collision Handle -> Constraint Solve
        m_taskGraph->addEdge(rbdObj->getRigidBodyModel2()->getComputeTentativeVelocitiesNode(), m_collisionGeometryUpdateNode);
        m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
        m_taskGraph->addEdge(m_collisionDetectionNode, rbdHandlerNode);
        m_taskGraph->addEdge(m_collisionDetectionNode, pbdHandlerNode);
        m_taskGraph->addEdge(pbdHandlerNode, rbdObj->getRigidBodyModel2()->getSolveNode()); // Ensure we aren't handling PBD whilst solving RBD
        m_taskGraph->addEdge(rbdHandlerNode, rbdObj->getRigidBodyModel2()->getSolveNode());
        m_taskGraph->addEdge(rbdObj->getRigidBodyModel2()->getSolveNode(), m_pbdCollisionSolveNode);
        m_taskGraph->addEdge(m_pbdCollisionSolveNode, rbdObj->getRigidBodyModel2()->getIntegrateNode());
    }
}
} // namespace imstk