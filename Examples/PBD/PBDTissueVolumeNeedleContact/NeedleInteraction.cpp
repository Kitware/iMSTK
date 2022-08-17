/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkLineMesh.h"
#include "imstkPbdModel.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"
#include "NeedleEmbeddedCH.h"
#include "NeedlePbdCH.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                                     std::shared_ptr<NeedleObject> needleObj) : PbdObjectCollision(tissueObj, needleObj)
{
    if (std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with LineMesh collision geometry on rigid NeedleObject";
    }
    if (std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry()) == nullptr)
    {
        LOG(WARNING) << "NeedleInteraction only works with TetrahedralMesh physics geometry on pbd tissueObj";
    }
    CHECK(tissueObj->getPbdModel() == needleObj->getPbdModel()) << "PbdObject's must share a model";

    // Replace the CH, to disable collision upon needle contact
    auto needlePbdCH = std::make_shared<NeedlePbdCH>();
    needlePbdCH->setInputObjectA(tissueObj);
    needlePbdCH->setInputObjectB(needleObj);
    needlePbdCH->setInputCollisionData(getCollisionDetection()->getCollisionData());
    setCollisionHandlingAB(needlePbdCH);

    // Then add a separate scheme for when the needle is embedded

    // Assumes usage of physics geometry for this
    m_tetMeshCD = std::make_shared<TetraToLineMeshCD>();
    m_tetMeshCD->setInputGeometryA(tissueObj->getPhysicsGeometry());
    m_tetMeshCD->setInputGeometryB(needleObj->getCollidingGeometry());

    m_embeddedCH = std::make_shared<NeedleEmbeddedCH>();
    m_embeddedCH->setInputCollisionData(m_tetMeshCD->getCollisionData());
    m_embeddedCH->setInputObjectA(tissueObj);
    m_embeddedCH->setInputObjectB(needleObj);
    m_embeddedCH->setCompliance(0.0001);

    // Needle interaction introduces its own collision detection step, handling, solve, and velocity correction
    m_embeddingCDNode =
        std::make_shared<TaskNode>([&]() { m_tetMeshCD->update(); }, "NeedleEmbeddingCD", true);
    m_taskGraph->addNode(m_embeddingCDNode);
    m_embeddingCHNode =
        std::make_shared<TaskNode>([&]() { m_embeddedCH->update(); }, "NeedleEmbeddingCH", true);
    m_taskGraph->addNode(m_embeddingCHNode);
}

void
NeedleInteraction::setFriction(const double friction)
{
    m_embeddedCH->setFriction(friction);
}

double
NeedleInteraction::getFriction() const
{
    return m_embeddedCH->getFriction();
}

void
NeedleInteraction::setCompliance(const double compliance)
{
    m_embeddedCH->setCompliance(compliance);
}

double
NeedleInteraction::getCompliance() const
{
    return m_embeddedCH->getCompliance();
}

void
NeedleInteraction::setStaticFrictionForceThreshold(const double force)
{
    m_embeddedCH->setStaticFrictionForceThreshold(force);
}

const double
NeedleInteraction::getStaticFrictionForceThreshold() const
{
    return m_embeddedCH->getStaticFrictionForceThreshold();
}

void
NeedleInteraction::setPunctureForceThreshold(const double forceThreshold)
{
    m_embeddedCH->setPunctureForceThreshold(forceThreshold);
}

const double
NeedleInteraction::getPunctureForceThreshold() const
{
    return m_embeddedCH->getPunctureForceThreshold();
}

void
NeedleInteraction::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup the usual collision interaction in the graph
    // which adds contact constraints before the end of the pbd solve
    PbdObjectCollision::initGraphEdges(source, sink);

    auto                               pbdObj = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<CollisionHandling> pbdCH  = m_colHandlingA;

    // Add some extra steps after the collision handling to do embedding handling
    // PBD CH -> EmbeddedCD -> EmbeddedCH -> Collision Solve
    m_taskGraph->addEdge(m_collisionHandleANode, m_embeddingCDNode);
    m_taskGraph->addEdge(m_embeddingCDNode, m_embeddingCHNode);
    m_taskGraph->addEdge(m_embeddingCHNode, pbdObj->getPbdModel()->getCollisionSolveNode());
}