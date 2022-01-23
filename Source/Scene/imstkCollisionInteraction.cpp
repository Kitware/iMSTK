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

#include "imstkCollisionInteraction.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCollisionHandling.h"
#include "imstkDynamicObject.h"
#include "imstkTaskGraph.h"

namespace imstk
{
CollisionInteraction::CollisionInteraction(
    std::string                      objName,
    std::shared_ptr<CollidingObject> objA,
    std::shared_ptr<CollidingObject> objB) : SceneObject(objName),
    m_objA(objA), m_objB(objB)
{
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
    if (auto dynObj1 = std::dynamic_pointer_cast<DynamicObject>(m_objA))
    {
        dynObj1->updateGeometries();
    }
    if (auto dynObj2 = std::dynamic_pointer_cast<DynamicObject>(m_objB))
    {
        dynObj2->updateGeometries();
    }
}
} // namespace imstk