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

#include "imstkPbdObject.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkPbdRigidBaryPointToPointConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdRigidObjectGrasping::PbdRigidObjectGrasping(
    std::shared_ptr<PbdObject>    obj1,
    std::shared_ptr<RigidObject2> obj2) :
    PbdObjectGrasping(obj1), m_rbdObj(obj2)
{
    m_taskGraph->addNode(m_pickingNode);
    m_taskGraph->addNode(obj2->getRigidBodyModel2()->getSolveNode());
}

void
PbdRigidObjectGrasping::updatePicking()
{
    PbdObjectGrasping::updatePicking();
    for (int i = 0; i < m_constraints.size(); i++)
    {
        auto constraint = std::dynamic_pointer_cast<PbdRigidBaryPointToPointConstraint>(m_constraints[i]);

        constraint->compute(m_rbdObj->getRigidBodyModel2()->getConfig()->m_dt);
        m_rbdObj->getRigidBodyModel2()->addConstraint(constraint);
    }
}

void
PbdRigidObjectGrasping::addConstraint(
    const std::vector<VertexMassPair>& ptsA,
    const std::vector<double>& weightsA,
    const std::vector<VertexMassPair>& ptsB,
    const std::vector<double>& weightsB,
    double stiffnessA, double stiffnessB)
{
    // Create constraint
    auto constraint = std::make_shared<PbdRigidBaryPointToPointConstraint>(m_rbdObj->getRigidBody());
    constraint->initConstraint(
        ptsA,
        weightsA,
        ptsB,
        weightsB,
        stiffnessA,
        stiffnessB);
    // Add to constraints
    m_constraints.push_back(constraint);
}

void
PbdRigidObjectGrasping::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    PbdObjectGrasping::initGraphEdges(source, sink);

    std::shared_ptr<PbdModel>        pbdModel = m_objectToGrasp->getPbdModel();
    std::shared_ptr<RigidBodyModel2> rbdModel = m_rbdObj->getRigidBodyModel2();

    m_taskGraph->addEdge(m_pickingNode, rbdModel->getSolveNode());
    m_taskGraph->addEdge(rbdModel->getSolveNode(), m_taskGraph->getSink());
}
} // namespace imstk
