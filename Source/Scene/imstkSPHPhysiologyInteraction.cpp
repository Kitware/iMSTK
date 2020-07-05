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
#include <iostream>

#include "imstkSPHPhysiologyInteraction.h"
#include "imstkPhysiologyModel.h"
#include "imstkPhysiologyObject.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkTaskGraph.h"

namespace imstk
{
  SPHPhysiologyObjectInteractionPair::SPHPhysiologyObjectInteractionPair(std::shared_ptr<SPHObject> obj1, std::shared_ptr<PhysiologyObject> obj2) : ObjectInteractionPair(obj1, obj2)
{
    m_sphModel1 = obj1->getDynamicalSPHModel();
    m_physiologyModel1 = obj2->getPhysiologyModel();

    // Define where the boundary condition happens
    m_bcNode = std::make_shared<TaskNode>([&]() { computeInletBCs(); },
      obj1->getName() + "_vs_" + obj2->getName() + "_boundaryCondition", true);

    // Define where solver interaction happens
    m_solveNodeInputs.first.push_back(m_sphModel1->getTaskGraph()->getSource());
    m_solveNodeInputs.second.push_back(m_physiologyModel1->getTaskGraph()->getSource());

    m_solveNodeOutputs.first.push_back(m_sphModel1->getFindParticleNeighborsNode());
    m_solveNodeOutputs.second.push_back(m_physiologyModel1->getSolveNode());
}

void SPHPhysiologyObjectInteractionPair::computeInletBCs()
{
    // todo input femoral flow rate as inlet boundary condition of SPH model
    double femoralFlowRate = m_physiologyModel1->m_femoralFlowRate;
    std::cout << femoralFlowRate << std::endl;
}

void
SPHPhysiologyObjectInteractionPair::apply()
{
    // Add the SPH physiology interaction node to the task graph
    m_objects.first->getTaskGraph()->addNode(m_bcNode);
    m_objects.second->getTaskGraph()->addNode(m_bcNode);

    // Add the edges to the task graph
    for (size_t i = 0; i < m_solveNodeInputs.first.size(); i++)
    {
        m_objects.first->getTaskGraph()->addEdge(m_solveNodeInputs.first[i], m_bcNode);
    }
    for (size_t i = 0; i < m_solveNodeInputs.second.size(); i++)
    {
        m_objects.second->getTaskGraph()->addEdge(m_solveNodeInputs.second[i], m_bcNode);
    }

    for (size_t i = 0; i < m_solveNodeOutputs.first.size(); i++)
    {
        m_objects.first->getTaskGraph()->addEdge(m_bcNode, m_solveNodeOutputs.first[i]);
    }
    for (size_t i = 0; i < m_solveNodeOutputs.second.size(); i++)
    {
        m_objects.second->getTaskGraph()->addEdge(m_bcNode, m_solveNodeOutputs.second[i]);
    }
}
}