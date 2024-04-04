/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCDObjectFactory.h"
#include "imstkCollisionInteraction.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCollisionHandling.h"
#include "imstkTaskGraph.h"

namespace imstk
{
CollisionInteraction::CollisionInteraction(
    std::string                      objName,
    std::shared_ptr<CollidingObject> objA,
    std::shared_ptr<CollidingObject> objB,
    std::string                      cdType = "") : SceneObject(objName),
    m_objA(objA), m_objB(objB)
{
    CHECK(objA != nullptr) << "CollisionInteraction requires a CollidingObject objA";
    CHECK(objB != nullptr) << "CollisionInteraction requires a CollidingObject objB";

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
    m_collisionGeometryUpdateNode = std::make_shared<TaskNode>([this]() {
            if (m_didUpdateThisFrame)
            {
                return;
            }
            m_didUpdateThisFrame = true;
            updateCollisionGeometry();
        },
        objA->getName() + "_vs_" + objB->getName() + "_CollisionGeometryUpdate", true);
    m_taskGraph->addNode(m_collisionGeometryUpdateNode);

    // Get default cdType if one not provided
    if (cdType.empty())
    {
        cdType = getCDType(*objA, *objB);
    }

    // Setup the CD
    m_colDetect = CDObjectFactory::makeCollisionDetection(cdType);
    m_colDetect->setInput(objA->getCollidingGeometry(), 0);
    m_colDetect->setInput(objB->getCollidingGeometry(), 1);
    setCollisionDetection(m_colDetect);
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

    auto dataVector = m_colDetect->getCollisionDataVector();
    for (const auto& item : *dataVector)
    {
        if (item->elementsA.empty() && item->elementsB.empty())
        {
            continue;
        }
        m_objA->addCollision(m_objB, item);
        m_objB->addCollision(m_objA, item);
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
    // Ensure the collision geometry is updated before checking collision
    // this could involve a geometry map or something, ex: simulated
    // tet mesh mapped to a collision surface mesh
    if (auto colObj1 = std::dynamic_pointer_cast<CollidingObject>(m_objA))
    {
        colObj1->updateGeometries();
    }
    if (auto colObj2 = std::dynamic_pointer_cast<CollidingObject>(m_objB))
    {
        colObj2->updateGeometries();
    }
}

void
CollisionInteraction::setEnabled(const bool enabled)
{
    m_collisionDetectionNode->setEnabled(enabled);

    if (m_colDetect != nullptr)
    {
        for (size_t i = 0; i < m_colDetect->getCollisionDataVectorSize(); ++i)
        {
            if (auto data = m_colDetect->getCollisionData())
            {
                // Clear the data (since CD clear is only run before CD is performed)
                data->elementsA.resize(0);
                data->elementsB.resize(0);
            }
        }
    }
    else
    {
        LOG(WARNING) << "Tried to enable/disable collision, but no CD method was provided";
    }
}

bool
CollisionInteraction::getEnabled() const
{
    return m_collisionDetectionNode->m_enabled;
}

void
CollisionInteraction::visualUpdate()
{
    m_didUpdateThisFrame = false;
}
} // namespace imstk