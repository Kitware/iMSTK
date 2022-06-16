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

#include "NeedleInteraction.h"
#include "imstkLineMesh.h"
#include "imstkPbdObject.h"
#include "imstkTaskGraph.h"
#include "imstkTetraToLineMeshCD.h"
#include "NeedleEmbeddedCH.h"
#include "NeedlePbdCH.h"
#include "NeedleRigidBodyCH.h"
#include "imstkTetrahedralMesh.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                                     std::shared_ptr<NeedleObject> needleObj) : PbdRigidObjectCollision(tissueObj, needleObj)
{
    if (std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with LineMesh collision geometry on rigid NeedleObject";
    }
    if (std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with TetrahedralMesh physics geometry on pbd tissueObj";
    }

    // This handler consumes collision data to resolve the tool from the tissue
    // except when the needle is inserted
    auto needleRbdCH = std::make_shared<NeedleRigidBodyCH>();
    needleRbdCH->setInputRigidObjectA(needleObj);
    needleRbdCH->setInputCollidingObjectB(tissueObj);
    needleRbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    needleRbdCH->setBaumgarteStabilization(0.001);
    setCollisionHandlingB(needleRbdCH);

    // This handler consumes the collision data to resolve the tissue from the tool
    // except when the needle is inserted
    auto needlePbdCH = std::make_shared<NeedlePbdCH>();
    needlePbdCH->setInputObjectA(tissueObj);
    needlePbdCH->setInputObjectB(needleObj);
    needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    //needlePbdCH->getCollisionSolver()->setCollisionIterations(1);
    setCollisionHandlingA(needlePbdCH);

    // Then add a separate scheme for when the needle is embedded

    // Assumes usage of physics geometry for this
    tetMeshCD = std::make_shared<TetraToLineMeshCD>();
    tetMeshCD->setInputGeometryA(tissueObj->getPhysicsGeometry());
    tetMeshCD->setInputGeometryB(needleObj->getCollidingGeometry());

    embeddedCH = std::make_shared<NeedleEmbeddedCH>();
    embeddedCH->setInputCollisionData(tetMeshCD->getCollisionData());
    embeddedCH->setCollisionSolver(needlePbdCH->getCollisionSolver());
    embeddedCH->setInputObjectA(tissueObj);
    embeddedCH->setInputObjectB(needleObj);

    // Needle interaction introduces its own collision detection step, handling, solve, and velocity correction
    embeddingCDNode =
        std::make_shared<TaskNode>([&]() { tetMeshCD->update(); }, "NeedleEmbeddingCD", true);
    m_taskGraph->addNode(embeddingCDNode);
    embeddingCHNode =
        std::make_shared<TaskNode>([&]() { embeddedCH->update(); }, "NeedleEmbeddingCH", true);
    m_taskGraph->addNode(embeddingCHNode);
}

void
NeedleInteraction::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    PbdRigidObjectCollision::initGraphEdges(source, sink);

    std::shared_ptr<CollisionDetectionAlgorithm> cd = m_colDetect;

    auto                               pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<CollisionHandling> pbdCH  = m_colHandlingA;

    auto                               rbdObj = std::dynamic_pointer_cast<RigidObject2>(m_objB);
    std::shared_ptr<CollisionHandling> rbdCH  = m_colHandlingB;

    {
        // PBD CH -> EmbeddedCD -> EmbeddedCH -> Collision Solve
        m_taskGraph->addEdge(m_collisionHandleANode, embeddingCDNode);
        m_taskGraph->addEdge(m_collisionHandleBNode, embeddingCDNode);
        m_taskGraph->addEdge(embeddingCDNode, embeddingCHNode);
        m_taskGraph->addEdge(embeddingCHNode, m_pbdCollisionSolveNode);
        m_taskGraph->addEdge(embeddingCHNode, rbdObj->getRigidBodyModel2()->getSolveNode());
    }
}