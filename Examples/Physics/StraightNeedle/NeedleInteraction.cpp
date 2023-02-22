/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleInteraction.h"
#include "imstkLineMesh.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystem.h"
#include "imstkPuncturable.h"
#include "imstkStraightNeedle.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraToLineMeshCD.h"
#include "NeedleEmbedder.h"

using namespace imstk;

NeedleInteraction::NeedleInteraction(std::shared_ptr<Entity> tissueObj,
                                     std::shared_ptr<Entity> needleObj,
                                     const std::string&      collisionName) :
    PbdObjectCollision(tissueObj, needleObj, collisionName)
{
}

bool
NeedleInteraction::initialize()
{
    PbdObjectCollision::initialize();

    auto tissueObj = m_objA;
    auto needleObj = m_objB;
    if (tissueObj->containsComponent<StraightNeedle>())
    {
        std::swap(tissueObj, needleObj);
    }

    CHECK(needleObj->containsComponent<StraightNeedle>())
        << "NeedleInteraction only works with objects that have a StraightNeedle component";
    CHECK(tissueObj->containsComponent<Puncturable>())
        << "NeedleInteraction only works with objects that have a Puncturable component";
    CHECK(tissueObj->getComponent<PbdMethod>() != nullptr
        && std::dynamic_pointer_cast<TetrahedralMesh>(
        tissueObj->getComponent<PbdMethod>()->getPhysicsGeometry()) != nullptr)
        << "NeedleInteraction only works with TetrahedralMesh physics geometry on pbd tissueObj";

    // Assumes usage of physics geometry for this
    m_embedder = std::make_shared<NeedleEmbedder>();
    m_embedder->setCollisionHandleNode(m_collisionHandleANode);
    m_embedder->setCollisionData(getCollisionDetection()->getCollisionData());
    m_embedder->setTissueObject(tissueObj);
    m_embedder->setNeedleObject(needleObj);

    // Needle interaction introduces its own collision detection step, handling, solve, and velocity correction
    m_embedderNode =
        std::make_shared<TaskNode>([&]() { m_embedder->update(); }, "NeedleEmbedding", true);
    m_taskGraph->addNode(m_embedderNode);

    return true;
}

void
NeedleInteraction::setFriction(const double friction)
{
    m_embedder->setFriction(friction);
}

double
NeedleInteraction::getFriction() const
{
    return m_embedder->getFriction();
}

void
NeedleInteraction::setNeedleCompliance(const double compliance)
{
    m_embedder->setCompliance(compliance);
}

double
NeedleInteraction::getNeedleCompliance() const
{
    return m_embedder->getCompliance();
}

void
NeedleInteraction::setStaticFrictionForceThreshold(const double force)
{
    m_embedder->setStaticFrictionForceThreshold(force);
}

const double
NeedleInteraction::getStaticFrictionForceThreshold() const
{
    return m_embedder->getStaticFrictionForceThreshold();
}

void
NeedleInteraction::setPunctureForceThreshold(const double forceThreshold)
{
    m_embedder->setPunctureForceThreshold(forceThreshold);
}

const double
NeedleInteraction::getPunctureForceThreshold() const
{
    return m_embedder->getPunctureForceThreshold();
}

void
NeedleInteraction::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup the usual collision interaction in the graph
    // which adds contact constraints before the end of the pbd solve
    PbdObjectCollision::initGraphEdges(source, sink);

    // Collision detection should be done before so we can tell if touching or not
    // This way state can transition Removed -> Touching -> Punctured in one step
    m_taskGraph->addEdge(m_collisionDetectionNode, m_embedderNode);
    m_taskGraph->addEdge(m_embedderNode, m_collisionHandleANode);
}