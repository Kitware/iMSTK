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

#include "imstkCollisionPair.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionHandling.h"
#include "imstkTaskGraph.h"

namespace imstk
{
CollisionPair::CollisionPair(std::shared_ptr<CollidingObject> objA,
                             std::shared_ptr<CollidingObject> objB) : ObjectInteractionPair(objA, objB)
{
}

CollisionPair::CollisionPair(std::shared_ptr<CollidingObject>    objA,
                             std::shared_ptr<CollidingObject>    objB,
                             std::shared_ptr<CollisionDetection> cd,
                             std::shared_ptr<CollisionHandling>  chA,
                             std::shared_ptr<CollisionHandling>  chB) : ObjectInteractionPair(objA, objB)
{
    setCollisionDetection(cd);

    if (chA != nullptr)
    {
        setCollisionHandlingA(chA);
    }
    if (chB != nullptr)
    {
        setCollisionHandlingB(chB);
    }
}

CollisionPair::CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB,
                             std::shared_ptr<CollisionDetection> cd,
                             std::shared_ptr<CollisionHandling> chAB) : ObjectInteractionPair(objA, objB)
{
    setCollisionDetection(cd);

    if (chAB != nullptr)
    {
        setCollisionHandlingAB(chAB);
    }
}

void
CollisionPair::setCollisionDetection(std::shared_ptr<CollisionDetection> colDetect)
{
    m_colDetect = colDetect;
    m_collisionDetectionNode = m_interactionFunction = m_colDetect->getTaskNode();
    m_collisionDetectionNode->m_name = getObjectsPair().first->getName() + "_" + getObjectsPair().second->getName() + "_CollisionDetection";
    m_colData = m_colDetect->getCollisionData();
}

void
CollisionPair::setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA)
{
    m_colHandlingA = colHandlingA;
    m_collisionHandleANode = m_colHandlingA->getTaskNode();
    m_collisionHandleANode->m_name = getObjectsPair().first->getName() + "_CollisionHandling";
}

void
CollisionPair::setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB)
{
    m_colHandlingB = colHandlingB;
    m_collisionHandleBNode = m_colHandlingB->getTaskNode();
    m_collisionHandleBNode->m_name = getObjectsPair().second->getName() + "_CollisionHandling";
}

void
CollisionPair::setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB)
{
    m_colHandlingA = m_colHandlingB = colHandlingAB;
    m_collisionHandleANode = m_collisionHandleBNode = colHandlingAB->getTaskNode();
    m_collisionHandleANode->m_name = getObjectsPair().first->getName() + '_' + getObjectsPair().second->getName() + "_CollisionHandling";
}

void
CollisionPair::apply()
{
    std::shared_ptr<TaskGraph> computeGraphA = m_objects.first->getTaskGraph();
    std::shared_ptr<TaskGraph> computeGraphB = m_objects.second->getTaskGraph();

    // If nothing was added to the input/output list use default collision location
    if ((m_taskNodeInputs.first.size() == 0) && (m_taskNodeInputs.second.size() == 0)
        && (m_taskNodeOutputs.first.size() == 0) && (m_taskNodeOutputs.second.size() == 0))
    {
        m_taskNodeInputs.first.clear();
        m_taskNodeInputs.second.clear();
        m_taskNodeOutputs.first.clear();
        m_taskNodeOutputs.second.clear();

        // Default location is inbetween the source->updateNode
        m_taskNodeInputs.first.push_back(computeGraphA->getSource());
        m_taskNodeInputs.second.push_back(computeGraphB->getSource());

        m_taskNodeOutputs.first.push_back(m_objects.first->getUpdateNode());
        m_taskNodeOutputs.second.push_back(m_objects.second->getUpdateNode());
    }

    // Add all the nodes to the graph
    computeGraphA->addNode(m_collisionDetectionNode);
    computeGraphB->addNode(m_collisionDetectionNode);
    if (m_collisionHandleANode != nullptr)
    {
        computeGraphA->addNode(m_collisionHandleANode);
    }
    if (m_collisionHandleBNode != nullptr)
    {
        computeGraphB->addNode(m_collisionHandleBNode);
    }

    // Add the edges
    {
        // Connect inputA's->CD
        for (size_t i = 0; i < m_taskNodeInputs.first.size(); i++)
        {
            computeGraphA->addEdge(m_taskNodeInputs.first[i], m_collisionDetectionNode);
        }

        // Connect inputB's->CD
        for (size_t i = 0; i < m_taskNodeInputs.second.size(); i++)
        {
            computeGraphA->addEdge(m_taskNodeInputs.second[i], m_collisionDetectionNode);
        }
    }

    // Now connect CD to CHA/CHB/CHAB
    // This also works for the case CHA = CHB = CHAB
    {
        if (m_collisionHandleANode != nullptr)
        {
            computeGraphA->addEdge(m_collisionDetectionNode, m_collisionHandleANode);
        }
        if (m_collisionHandleBNode != nullptr)
        {
            computeGraphB->addEdge(m_collisionDetectionNode, m_collisionHandleBNode);
        }
    }

    // Connect either CD or CHA/CHAB to outputA's
    for (size_t i = 0; i < m_taskNodeOutputs.first.size(); i++)
    {
        if (m_collisionHandleANode != nullptr)
        {
            computeGraphA->addEdge(m_collisionHandleANode, m_taskNodeOutputs.first[i]);
        }
        else
        {
            computeGraphA->addEdge(m_collisionDetectionNode, m_taskNodeOutputs.first[i]);
        }
    }
    // Connect eitehr CD or CHB/CHAB to outputB's
    for (size_t i = 0; i < m_taskNodeOutputs.second.size(); i++)
    {
        if (m_collisionHandleBNode != nullptr)
        {
            computeGraphB->addEdge(m_collisionHandleBNode, m_taskNodeOutputs.second[i]);
        }
        else
        {
            computeGraphB->addEdge(m_collisionDetectionNode, m_taskNodeOutputs.second[i]);
        }
    }
}
}
