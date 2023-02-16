/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCollider.h"
#include "imstkCCDAlgorithm.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSolver.h"
#include "imstkPbdSystem.h"
#include "imstkSceneObject.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdObjectCollision::PbdObjectCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                                       std::string cdType) :
    CollisionInteraction("PbdObjectCollision_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2, cdType)
{
}

namespace
{
std::shared_ptr<TaskGraph>
extractTaskGraph(std::shared_ptr<Entity> entity)
{
    // \todo: remove later when SceneObject class is no longer used.
    if (auto sceneObject = std::dynamic_pointer_cast<SceneObject>(entity))
    {
        return sceneObject->getTaskGraph();
    }
    else if (auto sceneBehaviour = std::dynamic_pointer_cast<SceneBehaviour>(entity->getComponent<PbdMethod>()))
    {
        sceneBehaviour->getTaskGraph();
    }
    LOG(FATAL) << "Cannot find a corresponding task graph for the provided entity.";
    return nullptr;
}
} // namespace anonymous

bool
PbdObjectCollision::initialize()
{
    CollisionInteraction::initialize();

    m_objectA.method   = m_objA->getComponent<PbdMethod>();
    m_objectA.collider = m_objA->getComponent<Collider>();
    m_objectB.method   = m_objB->getComponent<PbdMethod>();
    m_objectB.collider = m_objB->getComponent<Collider>();

    CHECK(m_objectA.method != nullptr || m_objectB.method != nullptr) << "At least one input object to PbdObjectCollision" <<
        "should have a PbdMethod.";

    m_objectA.taskGraph = extractTaskGraph(m_objA);
    m_objectB.taskGraph = extractTaskGraph(m_objB);

    m_objectA.system = m_objectA.method ? m_objectA.method->getPbdSystem() : nullptr;
    m_objectB.system = m_objectB.method ? m_objectB.method->getPbdSystem() : nullptr;

    // Swap so that objectA is the one that is guaranteed to have a PbdMethod.
    // ObjectB is always the object which may or may not have a PbdMethod.
    // Both objects are guaranteed to have a collider if they pass initialize().
    if (!m_objectA.method)
    {
        std::swap(m_objectA, m_objectB);
    }

    CHECK(m_objectA.method && m_objectA.system) << "At this point, ObjectA should have a PbdMethod and a PbdSystem.";

    CHECK(m_objectA.collider && m_objectB.collider) << "Both input objects should have a Collider.";

    CHECK(m_objectB.method == nullptr || m_objectA.system == m_objectB.system) <<
        "PbdObjectCollision may only be used with PbdObjects that share the same PbdSystem";

    setupConnections();
    return true;
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

    std::shared_ptr<TaskNode> chNodeAB = m_collisionHandleANode;

    // Ensure a complete graph
    m_taskGraph->addEdge(source, m_objectA.taskGraph->getSource());
    m_taskGraph->addEdge(m_objectA.taskGraph->getSink(), sink);
    m_taskGraph->addEdge(source, m_objectB.taskGraph->getSource());
    m_taskGraph->addEdge(m_objectB.taskGraph->getSink(), sink);

    // -------------------------------------------------------------------------
    // Internal Constraint Solve -> Collision Geometry Update -> Collision Detection ->
    // PbdHandlerAB -> Collision Constraint Solve -> CCD Update Prev Geometry ->
    // Update Pbd Velocity -> Correct Velocities for Collision (restitution+friction)
    // -------------------------------------------------------------------------
    m_taskGraph->addEdge(m_objectA.system->getIntegratePositionNode(), m_collisionGeometryUpdateNode);
    m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, chNodeAB); // A=AB=B
    m_taskGraph->addEdge(chNodeAB, m_objectA.system->getSolveNode());
    m_taskGraph->addEdge(m_objectA.system->getSolveNode(), m_updatePrevGeometryCCDNode);
    m_taskGraph->addEdge(m_updatePrevGeometryCCDNode, m_objectA.system->getUpdateVelocityNode());

    m_taskGraph->addEdge(m_collisionDetectionNode, m_objectB.taskGraph->getSink());
}

void
PbdObjectCollision::setupConnections()
{
    // Setup the handler
    std::shared_ptr<PbdCollisionHandling> ch = std::make_shared<PbdCollisionHandling>();
    ch->setInputObjectA(m_objectA.collider, m_objectA.method);
    ch->setInputObjectB(m_objectB.collider, m_objectB.method);
    ch->setInputCollisionData(m_colDetect->getCollisionData());

    m_updatePrevGeometryCCDNode = std::make_shared<TaskNode>([&]()
        {
            // Confirm if the collision detection algorithm is a CCD algorithm,
            // and update the cached geometry accordingly.
            if (auto pbdCCD = std::dynamic_pointer_cast<CCDAlgorithm>(getCollisionDetection()))
            {
                // \todo: These inputs could be flipped in the algorithm
                pbdCCD->updatePreviousTimestepGeometry(pbdCCD->getInput(0), pbdCCD->getInput(1));
            }
        });
    m_taskGraph->addNode(m_updatePrevGeometryCCDNode);

    setCollisionHandlingAB(ch);

    m_taskGraph->addNode(m_objectA.taskGraph->getSource());
    m_taskGraph->addNode(m_objectA.taskGraph->getSink());

    m_taskGraph->addNode(m_objectB.taskGraph->getSource());
    m_taskGraph->addNode(m_objectB.taskGraph->getSink());

    std::shared_ptr<PbdSystem> pbdSystem = m_objectA.method->getPbdSystem();
    m_taskGraph->addNode(pbdSystem->getSolveNode());
    m_taskGraph->addNode(pbdSystem->getIntegratePositionNode());
    m_taskGraph->addNode(pbdSystem->getUpdateVelocityNode());
    // updateGeometryNode of a SceneObject is empty. We do not expect a bug
    // to occur by removing the following line:
    // m_taskGraph->addNode(obj2AsSceneObject->getUpdateGeometryNode());
}
} // namespace imstk
