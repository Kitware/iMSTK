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

#include "imstkPbdObjectCollisionPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkComputeGraph.h"
#include "imstkComputeNode.h"
#include "imstkPBDCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"

namespace imstk
{
// Pbd Collision will be tested before any step of pbd, then resolved after the solve steps of the two objects
PbdObjectCollisionPair::PbdObjectCollisionPair(std::shared_ptr<PbdObject> obj1, std::shared_ptr<PbdObject> obj2,
                                               CollisionDetection::Type cdType) : CollisionPair(obj1, obj2)
{
    std::shared_ptr<PbdModel> pbdModel1 = obj1->getPbdModel();
    std::shared_ptr<PbdModel> pbdModel2 = obj2->getPbdModel();

    // Define where collision interaction happens
    m_computeNodeInputs.first.push_back(pbdModel1->getUpdateCollisionGeometryNode());
    m_computeNodeInputs.second.push_back(pbdModel2->getUpdateCollisionGeometryNode());

    m_computeNodeOutputs.first.push_back(pbdModel1->getSolveNode());
    m_computeNodeOutputs.second.push_back(pbdModel2->getSolveNode());

    // Define where solver interaction happens
    m_solveNodeInputs.first.push_back(pbdModel1->getSolveNode());
    m_solveNodeInputs.second.push_back(pbdModel2->getSolveNode());

    m_solveNodeOutputs.first.push_back(pbdModel1->getUpdateVelocityNode());
    m_solveNodeOutputs.second.push_back(pbdModel2->getUpdateVelocityNode());

    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handler
    std::shared_ptr<PBDCollisionHandling> ch = std::make_shared<PBDCollisionHandling>(CollisionHandling::Side::AB, m_colData, obj1, obj2);
    setCollisionHandlingAB(ch);

    // Setup compute node for collision solver (true/critical node)
    m_collisionSolveNode = std::make_shared<ComputeNode>([ch]() { ch->getCollisionSolver()->solve(); },
                obj1->getName() + "_vs_" + obj2->getName() + "_CollisionSolver", true);
}

void
PbdObjectCollisionPair::modifyComputeGraph()
{
    // Add the collision interaction
    CollisionPair::modifyComputeGraph();

    // Add a secondary interaction
    m_objects.first->getComputeGraph()->addNode(m_collisionSolveNode);
    m_objects.second->getComputeGraph()->addNode(m_collisionSolveNode);

    // Add the solver interaction
    for (size_t i = 0; i < m_solveNodeInputs.first.size(); i++)
    {
        m_objects.first->getComputeGraph()->addEdge(m_solveNodeInputs.first[i], m_collisionSolveNode);
    }
    for (size_t i = 0; i < m_solveNodeInputs.second.size(); i++)
    {
        m_objects.second->getComputeGraph()->addEdge(m_solveNodeInputs.second[i], m_collisionSolveNode);
    }

    for (size_t i = 0; i < m_solveNodeOutputs.first.size(); i++)
    {
        m_objects.first->getComputeGraph()->addEdge(m_collisionSolveNode, m_solveNodeOutputs.first[i]);
    }
    for (size_t i = 0; i < m_solveNodeOutputs.second.size(); i++)
    {
        m_objects.second->getComputeGraph()->addEdge(m_collisionSolveNode, m_solveNodeOutputs.second[i]);
    }
}
}