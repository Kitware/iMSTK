/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    const std::vector<PbdParticleId>& ptsA,
    const std::vector<double>& weightsA,
    const std::vector<PbdParticleId>& ptsB,
    const std::vector<double>& weightsB,
    double stiffnessA, double stiffnessB)
{
    // Create constraint
    auto constraint = std::make_shared<PbdRigidBaryPointToPointConstraint>(m_rbdObj->getRigidBody());
    constraint->initConstraint(
        ptsA, weightsA,
        ptsB, weightsB,
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
