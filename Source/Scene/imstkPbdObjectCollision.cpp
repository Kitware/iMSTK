/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCCDAlgorithm.h"
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
    CollisionInteraction("PbdObjectCollision_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2, cdType)
{
    setupConnections(obj1, obj2, cdType);
}

void
PbdObjectCollision::setRestitution(const double restitution)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setRestitution(restitution);
}

double
PbdObjectCollision::getRestitution() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getRestitution();
}

void
PbdObjectCollision::setFriction(const double friction)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setFriction(friction);
}

double
PbdObjectCollision::getFriction() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getFriction();
}

bool
PbdObjectCollision::getUseCorrectVelocity() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getUseCorrectVelocity();
}

void
PbdObjectCollision::setUseCorrectVelocity(const bool useCorrectVelocity)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setUseCorrectVelocity(useCorrectVelocity);
}

void
PbdObjectCollision::setRigidBodyCompliance(const double compliance)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setRigidBodyCompliance(compliance);
}

double
PbdObjectCollision::getRigidBodyCompliance() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getRigidBodyCompliance();
}

void
PbdObjectCollision::setDeformableStiffnessA(const double stiffness)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setDeformableStiffnessA(stiffness);
}

double
PbdObjectCollision::getDeformableStiffnessA() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getDeformableStiffnessA();
}

void
PbdObjectCollision::setDeformableStiffnessB(const double stiffness)
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    pbdCH->setDeformableStiffnessB(stiffness);
}

double
PbdObjectCollision::getDeformableStiffnessB() const
{
    auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(getCollisionHandlingA());
    CHECK(pbdCH != nullptr) << "No PbdCollisionHandling set";
    return pbdCH->getDeformableStiffnessB();
}

void
PbdObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                         pbdObj1 = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<SceneObject> obj2    = m_objB;

    std::shared_ptr<TaskNode> chNodeAB = m_collisionHandleANode;

    // Ensure a complete graph
    m_taskGraph->addEdge(source, pbdObj1->getTaskGraph()->getSource());
    m_taskGraph->addEdge(pbdObj1->getTaskGraph()->getSink(), sink);
    m_taskGraph->addEdge(source, obj2->getTaskGraph()->getSource());
    m_taskGraph->addEdge(obj2->getTaskGraph()->getSink(), sink);

    // -------------------------------------------------------------------------
    // Internal Constraint Solve -> Collision Geometry Update -> Collision Detection ->
    // PbdHandlerAB -> Collision Constraint Solve -> CCD Update Prev Geometry ->
    // Update Pbd Velocity -> Correct Velocities for Collision (restitution+friction)
    // -------------------------------------------------------------------------
    m_taskGraph->addEdge(pbdObj1->getPbdModel()->getIntegratePositionNode(), m_collisionGeometryUpdateNode);
    m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, chNodeAB); // A=AB=B
    m_taskGraph->addEdge(chNodeAB, pbdObj1->getPbdModel()->getSolveNode());

    m_taskGraph->addEdge(pbdObj1->getPbdModel()->getSolveNode(), m_updatePrevGeometryCCDNode);
    m_taskGraph->addEdge(m_updatePrevGeometryCCDNode, pbdObj1->getPbdModel()->getUpdateVelocityNode());

    if (std::dynamic_pointer_cast<PbdObject>(obj2) == nullptr)
    {
        m_taskGraph->addEdge(obj2->getUpdateGeometryNode(), m_collisionGeometryUpdateNode);
        m_taskGraph->addEdge(m_collisionDetectionNode, obj2->getTaskGraph()->getSink());
    }
}

void
PbdObjectCollision::setupConnections(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2, std::string cdType /*= ""*/)
{
    // Setup the handler
    std::shared_ptr<PbdCollisionHandling> ch = std::make_shared<PbdCollisionHandling>();
    ch->setInputObjectA(obj1);
    ch->setInputObjectB(obj2);
    if (m_colDetect->getCollisionVectorData() != nullptr)
    {
        ch->setInputCollisionData(m_colDetect->getCollisionVectorData());
    }
    else
    {
        ch->setInputCollisionData(m_colDetect->getCollisionData());
    }

    m_updatePrevGeometryCCDNode = std::make_shared<TaskNode>([&]()
        {
            // Confirm if the collision detection algorithm is a CCD algorithm,
            // and update the cached geometry accordingly.
            if (auto pbdCCD = std::dynamic_pointer_cast<CCDAlgorithm>(getCollisionDetection()))
            {
                // \todo: These inputs could be flipped in the algorithm
                std::dynamic_pointer_cast<CollidingObject>(m_objA)->updateGeometries();
                std::dynamic_pointer_cast<CollidingObject>(m_objB)->updateGeometries();
                pbdCCD->updatePreviousTimestepGeometry(pbdCCD->getInput(0), pbdCCD->getInput(1));
            }
        });
    m_taskGraph->addNode(m_updatePrevGeometryCCDNode);

    setCollisionHandlingAB(ch);

    m_taskGraph->addNode(obj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSink());

    std::shared_ptr<PbdModel> pbdModel = obj1->getPbdModel();
    m_taskGraph->addNode(pbdModel->getSolveNode());
    m_taskGraph->addNode(pbdModel->getIntegratePositionNode());
    m_taskGraph->addNode(pbdModel->getUpdateVelocityNode());

    if (auto pbdObj2 = std::dynamic_pointer_cast<PbdObject>(obj2))
    {
        CHECK(pbdModel == pbdObj2->getPbdModel()) << "PbdObjectCollision may only be used with PbdObjects that share the same PbdModel";
    }
    else
    {
        m_taskGraph->addNode(obj2->getUpdateGeometryNode());
    }
}
} // namespace imstk