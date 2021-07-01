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

#pragma once

#include "imstkPbdRigidObjectCollision.h"
#include "imstkTetraToLineMeshCD.h"

#include "NeedlePbdCH.h"
#include "NeedleRigidBodyCH.h"
#include "NeedleEmbeddedCH.h"

#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"

using namespace imstk;

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public PbdRigidObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject> tissueObj, std::shared_ptr<NeedleObject> needleObj) : PbdRigidObjectCollision(tissueObj, needleObj)
    {
        if (std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) == nullptr)
        {
            LOG(WARNING) << "NeedleInteraction only works with LineMesh collision geometry on NeedleObject";
        }

        // This handler consumes collision data to resolve the tool from the tissue
        imstkNew<NeedleRigidBodyCH> needleRbdCH;
        needleRbdCH->setInputRigidObjectA(needleObj);
        needleRbdCH->setInputCollidingObjectB(tissueObj);
        needleRbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needleRbdCH->setBeta(0.001);
        needleRbdCH->getTaskNode()->m_isCritical = true;
        setCollisionHandlingB(needleRbdCH);

        // This handler consumes the collision data to resolve the tissue from the tool
        imstkNew<NeedlePbdCH> needlePbdCH;
        needlePbdCH->setInputObjectA(tissueObj);
        needlePbdCH->setInputObjectB(needleObj);
        needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
        needlePbdCH->getCollisionSolver()->setCollisionIterations(1);
        needlePbdCH->getTaskNode()->m_isCritical = true;
        setCollisionHandlingA(needlePbdCH);

        // This handler consumes collision data to produce PBD and RBD constraints when the tool
        // is embedded in the tissue (ie: when NeedleObject::inserted is on)

        // Assumes usage of physics geometry for this
        tetMeshCD = std::make_shared<TetraToLineMeshCD>();
        tetMeshCD->setInputGeometryA(tissueObj->getPhysicsGeometry());
        tetMeshCD->setInputGeometryB(needleObj->getCollidingGeometry());

        embeddedCH = std::make_shared<NeedleEmbeddedCH>();
        embeddedCH->setInputCollisionData(tetMeshCD->getCollisionData());
        embeddedCH->setInputObjectA(tissueObj);
        embeddedCH->setInputObjectB(needleObj);
        embeddedCH->getTaskNode()->m_isCritical = true;
    }

    ~NeedleInteraction() override = default;

public:
    ///
    /// \brief Set the force threshold for the needle
    ///
    void setForceThreshold(const double forceThreshold)
    {
        if (auto needleRbdCh = std::dynamic_pointer_cast<NeedleRigidBodyCH>(getCollisionHandlingB()))
        {
            needleRbdCh->setNeedleForceThreshold(forceThreshold);
        }
    }

protected:
    void apply() override
    {
        PbdRigidObjectCollision::apply();

        std::shared_ptr<TaskGraph> taskGraphA = m_objects.first->getTaskGraph();  // Pbd
        std::shared_ptr<TaskGraph> taskGraphB = m_objects.second->getTaskGraph(); // Rbd

        std::shared_ptr<CollisionDetectionAlgorithm> cd = m_colDetect;

        auto                               pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objects.first);
        std::shared_ptr<CollisionHandling> pbdCH  = m_colHandlingA;

        auto                               rbdObj = std::dynamic_pointer_cast<RigidObject2>(m_objects.second);
        std::shared_ptr<CollisionHandling> rbdCH  = m_colHandlingB;

        // Detection collision with the tet mesh
        auto needleEmbeddedCD =
            std::make_shared<TaskNode>([&]() { tetMeshCD->update(); }, "NeedleEmbeddingCD", true);
        taskGraphA->addNode(needleEmbeddedCD);
        taskGraphB->addNode(needleEmbeddedCD);

        taskGraphA->addEdge(m_colDetect->getTaskNode(), needleEmbeddedCD);
        taskGraphA->addEdge(needleEmbeddedCD, pbdCH->getTaskNode());

        taskGraphB->addEdge(m_colDetect->getTaskNode(), needleEmbeddedCD);
        taskGraphB->addEdge(needleEmbeddedCD, pbdCH->getTaskNode());

        // Consume the collision data after main handler executes but before collisions are solved for
        auto embeddingCHNode =
            std::make_shared<TaskNode>([&]() { embeddedCH->update(); }, "NeedleEmbeddingCH", true);
        taskGraphA->addNode(embeddingCHNode);
        taskGraphB->addNode(embeddingCHNode);

        // pbdCH -> embeddingCH -> collision solve (moves vertices)
        taskGraphA->addEdge(pbdCH->getTaskNode(), embeddingCHNode);
        taskGraphA->addEdge(embeddingCHNode, m_pbdCollisionSolveNode);

        taskGraphB->addEdge(pbdCH->getTaskNode(), embeddingCHNode);
        taskGraphB->addEdge(embeddingCHNode, m_pbdCollisionSolveNode);

        // Add a solver step for the embedding constraints solver
        // Should happen after collision to ensure convergence on final constraints
        auto embeddingSolveNode =
            std::make_shared<TaskNode>([&]() { embeddedCH->solve(); }, "NeedleEmbeddingSolve", true);
        auto correctNeedleVelocitiesNode =
            std::make_shared<TaskNode>([&]() { embeddedCH->correctVelocities(); }, "NeedleEmbeddingCorrectVelocities");
        taskGraphA->addNode(embeddingSolveNode);
        taskGraphA->addNode(correctNeedleVelocitiesNode);
        taskGraphB->addNode(embeddingSolveNode);
        taskGraphB->addNode(correctNeedleVelocitiesNode);

        taskGraphA->addEdge(m_pbdCollisionSolveNode, embeddingSolveNode);
        taskGraphA->addEdge(embeddingSolveNode, pbdObj->getPbdModel()->getUpdateVelocityNode());

        taskGraphA->addEdge(m_correctVelocitiesNode, correctNeedleVelocitiesNode);
        taskGraphA->addEdge(correctNeedleVelocitiesNode, pbdObj->getPbdModel()->getTaskGraph()->getSink());

        taskGraphB->addEdge(m_pbdCollisionSolveNode, embeddingSolveNode);
        taskGraphB->addEdge(embeddingSolveNode, rbdObj->getRigidBodyModel2()->getIntegrateNode());
    }

protected:
    std::shared_ptr<TetraToLineMeshCD> tetMeshCD;
    std::shared_ptr<NeedleEmbeddedCH>  embeddedCH;
};