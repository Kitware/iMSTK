/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCDObjectFactory.h"
#include "imstkCollider.h"
#include "imstkCollisionInteraction.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCollisionHandling.h"
#include "imstkEntity.h"
#include "imstkSceneObject.h"
#include "imstkTaskGraph.h"

namespace imstk
{
CollisionInteraction::CollisionInteraction(
    std::string             objName,
    std::shared_ptr<Entity> objA,
    std::shared_ptr<Entity> objB,
    std::string             cdType = "") : SceneObject(objName),
    m_objA(objA), m_objB(objB)
{
    CHECK(objA != nullptr) << "CollisionInteraction requires an Entity objA";
    CHECK(objB != nullptr) << "CollisionInteraction requires an Entity objB";

    m_collisionDetectionNode = std::make_shared<TaskNode>(std::bind(&CollisionInteraction::updateCD, this),
        objA->getName() + "_vs_" + objB->getName() + "_CollisionDetection");
    m_taskGraph->addNode(m_collisionDetectionNode);

    m_collisionHandleANode = std::make_shared<TaskNode>(std::bind(&CollisionInteraction::updateCHA, this),
        objA->getName() + "_vs_" + objB->getName() + "_CollisionHandlingA", true);
    m_taskGraph->addNode(m_collisionHandleANode);

    m_collisionHandleBNode = std::make_shared<TaskNode>(std::bind(&CollisionInteraction::updateCHB, this),
        objA->getName() + "_vs_" + objB->getName() + "_CollisionHandlingB", true);
    m_taskGraph->addNode(m_collisionHandleBNode);

    // Setup a step to update geometries before detecting collision
    m_collisionGeometryUpdateNode = std::make_shared<TaskNode>(std::bind(&CollisionInteraction::updateCollisionGeometry, this),
        objA->getName() + "_vs_" + objB->getName() + "_CollisionGeometryUpdate", true);
    m_taskGraph->addNode(m_collisionGeometryUpdateNode);

    // Get default cdType if one not provided
    auto colliderA = objA->getComponent<Collider>();
    auto colliderB = objB->getComponent<Collider>();

    if (colliderA && colliderB)
    {
        auto collidingGeomA = objA->getComponent<Collider>()->getGeometry();
        auto collidingGeomB = objB->getComponent<Collider>()->getGeometry();

        if (cdType.empty() && collidingGeomA && collidingGeomB)
        {
            // cdType = getCDType(*objA, *objB);
            cdType = CDObjectFactory::getCDType(*collidingGeomA, *collidingGeomB);
        }

        if (!cdType.empty())
        {
            // Setup the CD
            m_colDetect = CDObjectFactory::makeCollisionDetection(cdType);
            m_colDetect->setInput(collidingGeomA, 0);
            m_colDetect->setInput(collidingGeomB, 1);
            setCollisionDetection(m_colDetect);
        }
        else
        {
            LOG(FATAL) << "Failed to identify collision detection method to use.";
        }
    }
    else
    {
        LOG(FATAL) << "Failed to find Colliders.";
    }
}

void
CollisionInteraction::setCollisionDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect)
{
    m_colDetect = colDetect;
}

void
CollisionInteraction::setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA)
{
    m_colHandlingA = colHandlingA;
}

void
CollisionInteraction::setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB)
{
    m_colHandlingB = colHandlingB;
}

void
CollisionInteraction::setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB)
{
    m_colHandlingA = m_colHandlingB = colHandlingAB;
}

void
CollisionInteraction::updateCD()
{
    if (m_colDetect != nullptr)
    {
        m_colDetect->update();
    }
}

void
CollisionInteraction::updateCHA()
{
    if (m_colHandlingA != nullptr)
    {
        m_colHandlingA->update();
    }
}

void
CollisionInteraction::updateCHB()
{
    if (m_colHandlingB != nullptr)
    {
        m_colHandlingB->update();
    }
}

void
CollisionInteraction::updateCollisionGeometry()
{
    // Ensure the collision geometry is updatedbefore checking collision
    // this could involve a geometry map or something, ex: simulated
    // tet mesh mapped to a collision surface mesh
}

void
CollisionInteraction::setEnabled(const bool enabled)
{
    m_collisionDetectionNode->setEnabled(enabled);
    if (m_colDetect != nullptr)
    {
        // Clear the data (since CD clear is only run before CD is performed)
        m_colDetect->getCollisionData()->elementsA.resize(0);
        m_colDetect->getCollisionData()->elementsB.resize(0);
    }
    else
    {
        LOG(FATAL) << "Tried to enable/disable collision, but no CD method was provided";
    }
}

bool
CollisionInteraction::getEnabled() const
{
    return m_collisionDetectionNode->m_enabled;
}
} // namespace imstk