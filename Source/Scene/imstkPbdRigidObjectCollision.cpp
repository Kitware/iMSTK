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
#include "imstkPBDCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"
#include "imstkRigidBodyModel2.h"

namespace imstk
{
PbdRigidObjectCollision::PbdRigidObjectCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<RigidObject2> obj2,
                                                 std::string cdType) : CollisionPair(obj1, obj2)
{
    std::shared_ptr<PbdModel> pbdModel1 = obj1->getPbdModel();

    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd =
        makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry());
    setCollisionDetection(cd);

    // Setup the handler to resolve obj1
    auto pbdCH = std::make_shared<PBDCollisionHandling>();
    pbdCH->setInputObjectA(obj1);
    pbdCH->setInputObjectB(obj2);
    pbdCH->setInputCollisionData(cd->getCollisionData());
    pbdCH->getTaskNode()->m_isCritical = true;
    setCollisionHandlingA(pbdCH);

    auto rbdCH = std::make_shared<RigidBodyCH>();
    rbdCH->setInputRigidObjectA(obj2);
    rbdCH->setInputCollidingObjectB(obj1);
    rbdCH->setInputCollisionData(cd->getCollisionData());
    rbdCH->setBeta(0.05);
    rbdCH->getTaskNode()->m_isCritical = true;
    setCollisionHandlingB(rbdCH);

    // Setup compute node for collision solver (true/critical node)
    m_pbdCollisionSolveNode = std::make_shared<TaskNode>([&]()
        {
            std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->getCollisionSolver()->solve();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_PBDCollisionSolver", true);

    m_correctVelocitiesNode = std::make_shared<TaskNode>([&]()
        {
            std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->correctVelocities();
        },
        obj1->getName() + "_vs_" + obj2->getName() + "_PBDVelocityCorrect", true);
}

void
PbdRigidObjectCollision::setRestitution(const double restitution)
{
    std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->setRestitution(restitution);
}

const double
PbdRigidObjectCollision::getRestitution() const
{
    return std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->getRestitution();
}

void
PbdRigidObjectCollision::setFriction(const double friction)
{
    std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->setFriction(friction);
}

const double
PbdRigidObjectCollision::getFriction() const
{
    return std::dynamic_pointer_cast<PBDCollisionHandling>(getCollisionHandlingA())->getFriction();
}

void
PbdRigidObjectCollision::apply()
{
    // Add the collision solve step (which happens after internal constraint solve)
    std::shared_ptr<TaskGraph> computeGraphA = m_objects.first->getTaskGraph();  // Pbd
    std::shared_ptr<TaskGraph> computeGraphB = m_objects.second->getTaskGraph(); // Rbd

    std::shared_ptr<CollisionDetectionAlgorithm> cd = m_colDetect;

    // Because pbd solves directly on positions it would cause a race condition
    // if we were to solve rbd and pbd at the same time. Pbd won't write to the
    // rigid body positions, but it will read them.
    // We solve rigid body before pbd, this way pbd has the most up to date positions
    // semi-implicit

    auto                               pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objects.first);
    std::shared_ptr<CollisionHandling> pbdCH  = m_colHandlingA;
    {
        // Add all our collision step nodes to pbd pipeline
        computeGraphA->addNode(m_pbdCollisionSolveNode);
        computeGraphA->addNode(m_collisionGeometryUpdateNode);
        computeGraphA->addNode(pbdCH->getTaskNode());
        computeGraphA->addNode(m_correctVelocitiesNode);
        computeGraphA->addNode(m_colDetect->getTaskNode());

        // InternalConstraint Solve -> Update Collision Geometry ->
        // Collision Detect -> Collision Handle -> Solve Collision ->
        // Update Pbd Velocity -> Correct Velocity -> PbdModelSink
        computeGraphA->addEdge(pbdObj->getPbdModel()->getSolveNode(), m_collisionGeometryUpdateNode);
        computeGraphA->addEdge(m_collisionGeometryUpdateNode, m_colDetect->getTaskNode());
        computeGraphA->addEdge(m_colDetect->getTaskNode(), pbdCH->getTaskNode());
        computeGraphA->addEdge(pbdCH->getTaskNode(), m_pbdCollisionSolveNode);
        computeGraphA->addEdge(m_pbdCollisionSolveNode, pbdObj->getPbdModel()->getUpdateVelocityNode());
        computeGraphA->addEdge(pbdObj->getPbdModel()->getUpdateVelocityNode(), m_correctVelocitiesNode);
        computeGraphA->addEdge(m_correctVelocitiesNode, pbdObj->getPbdModel()->getTaskGraph()->getSink());
    }

    auto                               rbdObj = std::dynamic_pointer_cast<RigidObject2>(m_objects.second);
    std::shared_ptr<CollisionHandling> rbdCH  = m_colHandlingB;
    {
        //computeGraphB->addNode(m_collisionGeometryUpdateNode);
        // Add all the nodes we'll need to rbd pipeline
        computeGraphB->addNode(rbdCH->getTaskNode());
        computeGraphB->addNode(m_colDetect->getTaskNode());
        computeGraphB->addNode(m_pbdCollisionSolveNode);
        computeGraphB->addNode(m_collisionGeometryUpdateNode);
        computeGraphB->addNode(pbdCH->getTaskNode());

        // Compute Tentative Velocities -> Collision Detect ->
        // Collision Handle -> Constraint Solve
        computeGraphB->addEdge(rbdObj->getRigidBodyModel2()->getComputeTentativeVelocitiesNode(), m_collisionGeometryUpdateNode);
        computeGraphB->addEdge(m_collisionGeometryUpdateNode, m_colDetect->getTaskNode());
        computeGraphB->addEdge(m_colDetect->getTaskNode(), rbdCH->getTaskNode());
        computeGraphB->addEdge(m_colDetect->getTaskNode(), pbdCH->getTaskNode());
        computeGraphB->addEdge(pbdCH->getTaskNode(), rbdObj->getRigidBodyModel2()->getSolveNode()); // Ensure we aren't handling PBD whilst solving RBD
        computeGraphB->addEdge(rbdCH->getTaskNode(), rbdObj->getRigidBodyModel2()->getSolveNode());
        computeGraphB->addEdge(rbdObj->getRigidBodyModel2()->getSolveNode(), m_pbdCollisionSolveNode);
        computeGraphB->addEdge(m_pbdCollisionSolveNode, rbdObj->getRigidBodyModel2()->getIntegrateNode());
    }
}
}