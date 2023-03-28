/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollider.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkSphCollisionHandling.h"
#include "imstkSphMethod.h"
#include "imstkSphSystem.h"
#include "imstkTaskGraph.h"

namespace imstk
{
// Pbd Collision will be tested before any step of pbd, then resolved after the solve steps of the two objects
SphObjectCollision::SphObjectCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                                       std::string cdType) :
    CollisionInteraction("SphObjectCollision_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2, cdType)
{
}

bool
SphObjectCollision::initialize()
{
    CollisionInteraction::initialize();

    auto sphMethodA = m_objA->getComponentUnsafe<SphMethod>();
    auto sphMethodB = m_objB->getComponentUnsafe<SphMethod>();
    CHECK(sphMethodA || sphMethodB) << "At least one input Entity should have an SphMethod.";

    // Swap so that the first object (m_objA) is always the SPH object.
    if (sphMethodB)
    {
        std::swap(m_objA, m_objB);
        std::swap(sphMethodA, sphMethodB);
    }

    auto colliderA = m_objA->getComponent<Collider>();

    // Setup the handler
    auto ch = std::make_shared<SphCollisionHandling>();
    ch->setInputSphObject(sphMethodA, colliderA);
    ch->setInputCollisionData(m_colDetect->getCollisionData());
    ch->setDetection(m_colDetect);
    setCollisionHandlingA(ch);
    // Since CollisingHandling classes are currently neither of Component, Behaviour, or SceneObject types,
    // they will require explicit initialization inside the Interaction classes where they are instantiated.
    ch->initialize();

    // Collision should happen after positions and velocities are computed
    m_taskGraph->addNode(sphMethodA->getUpdateGeometryNode());
    m_taskGraph->addNode(sphMethodA->getTaskGraph()->getSink());

    return true;
}

void
SphObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto sphMethodA = m_objA->getComponent<SphMethod>();

    //
    // ...Sph steps...
    // Update Geometry A                  Update Geometry B
    //                 Collision Detection
    //                 Collision Handling A
    //    objA Sink                          objB Sink
    //
    m_taskGraph->addEdge(sphMethodA->getUpdateGeometryNode(), m_collisionDetectionNode);

    m_taskGraph->addEdge(m_collisionDetectionNode, m_collisionHandleANode);

    m_taskGraph->addEdge(m_collisionHandleANode, sphMethodA->getTaskGraph()->getSink());
}
} // namespace imstk