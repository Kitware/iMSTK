/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkSphCollisionHandling.h"
#include "imstkSphModel.h"
#include "imstkSphObject.h"
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

    auto sphObjA = std::dynamic_pointer_cast<SphObject>(m_objA);
    auto sphObjB = std::dynamic_pointer_cast<SphObject>(m_objB);
    CHECK(sphObjA || sphObjB) << "At least one input Entity should be an SphObject.";

    // Swap so that the first object (m_objA) is always the SPH object.
    if (sphObjB)
    {
        std::swap(m_objA, m_objB);
        std::swap(sphObjA, sphObjB);
    }

    // Setup the handler
    std::shared_ptr<SphCollisionHandling> ch = std::make_shared<SphCollisionHandling>();
    ch->setInputSphObject(sphObjA);
    ch->setInputCollisionData(m_colDetect->getCollisionData());
    ch->setDetection(m_colDetect);
    setCollisionHandlingA(ch);
    // Since CollisingHandling classes are currently neither of Component, Behaviour, or SceneObject types,
    // they will require explicit initialization inside the Interaction classes where they are instantiated.
    ch->initialize();

    auto obj2AsSceneObject = std::dynamic_pointer_cast<SceneObject>(m_objB);
    CHECK(obj2AsSceneObject != nullptr) << "Expected obj2 to be a SceneObject.";
    // Collision should happen after positions and velocities are computed
    m_taskGraph->addNode(sphObjA->getUpdateGeometryNode());
    m_taskGraph->addNode(obj2AsSceneObject->getUpdateGeometryNode());

    m_taskGraph->addNode(sphObjA->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2AsSceneObject->getTaskGraph()->getSink());

    return true;
}

void
SphObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto sphObj1 = std::dynamic_pointer_cast<SphObject>(m_objA);

    //
    // ...Sph steps...
    // Update Geometry A                  Update Geometry B
    //                 Collision Detection
    //                 Collision Handling A
    //    objA Sink                          objB Sink
    //
    m_taskGraph->addEdge(sphObj1->getUpdateGeometryNode(), m_collisionDetectionNode);
    auto objBAsSceneObject = std::dynamic_pointer_cast<SceneObject>(m_objB);
    m_taskGraph->addEdge(objBAsSceneObject->getUpdateGeometryNode(), m_collisionDetectionNode);

    m_taskGraph->addEdge(m_collisionDetectionNode, m_collisionHandleANode);

    m_taskGraph->addEdge(m_collisionHandleANode, sphObj1->getTaskGraph()->getSink());
    m_taskGraph->addEdge(m_collisionHandleANode, objBAsSceneObject->getTaskGraph()->getSink());
}
} // namespace imstk