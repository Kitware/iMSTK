/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRigidObjectLevelSetCollision.h"
#include "imstkCollider.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometry.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkParallelFor.h"
#include "imstkPointSet.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystem.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVecDataArray.h"

namespace imstk
{
RigidObjectLevelSetCollision::RigidObjectLevelSetCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2) :
    CollisionInteraction("RigidObjectLevelSetCollision" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2, ""),
    m_prevVertices(std::make_shared<VecDataArray<double, 3>>())
{
}

bool
RigidObjectLevelSetCollision::initialize()
{
    CollisionInteraction::initialize();

    auto levelSetObjA = std::dynamic_pointer_cast<LevelSetDeformableObject>(m_objA);
    auto levelSetObjB = std::dynamic_pointer_cast<LevelSetDeformableObject>(m_objB);

    CHECK(levelSetObjA != nullptr || levelSetObjB != nullptr) <<
        "One input object is expected to be a LevelSetDeformableObject.";

    if (levelSetObjA)
    {
        std::swap(m_objA, m_objB);
        std::swap(levelSetObjA, levelSetObjB);
    }

    CHECK(levelSetObjA == nullptr) <<
        "At this point, objA is expected to be the rigid object.";

    // Fetch all components and structures required for objectA
    m_objectA.method   = m_objA->getComponent<PbdMethod>();
    m_objectA.collider = m_objA->getComponent<Collider>();
    if (m_objectA.method)
    {
        m_objectA.system    = m_objectA.method->getPbdSystem();
        m_objectA.taskGraph = m_objectA.method->getTaskGraph();

        // Give the point set displacements for CCD, if it doesn't already have them
        if (auto pointSet = std::dynamic_pointer_cast<PointSet>(m_objectA.collider->getGeometry()))
        {
            if (!pointSet->hasVertexAttribute("displacements"))
            {
                auto displacementsPtr = std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices());
                pointSet->setVertexAttribute("displacements", displacementsPtr);
                displacementsPtr->fill(Vec3d::Zero());
            }
            m_objectA.physicsGeometry = pointSet;
            m_objectA.displacements   =
                std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_objectA.physicsGeometry->getVertexAttribute("displacements"));
        }
    }

    // Fetch all components and structures required for objectB
    m_objectB.obj       = levelSetObjB;
    m_objectB.method    = levelSetObjB->getLevelSetModel();
    m_objectB.collider  = m_objB->getComponent<Collider>();
    m_objectB.taskGraph = m_objectB.obj->getTaskGraph();

    if (m_objectA.system == nullptr || m_objectB.method == nullptr)
    {
        LOG(WARNING) << "Cannot create collision pair.";
        return false;
    }
    if (!m_objectA.method->getPbdBody()->isRigid())
    {
        LOG(WARNING) << "PBD object should be rigid.";
        return false;
    }

    // Here the CH's adds constraints to the system on the LHS, and impulses to the levelset RHS
    m_taskGraph->addNode(m_objectA.system->getSolveNode());
    m_taskGraph->addNode(m_objectA.system->getIntegratePositionNode());
    m_taskGraph->addNode(m_objectA.system->getUpdateVelocityNode());

    m_taskGraph->addNode(m_objectB.method->getGenerateVelocitiesBeginNode());
    m_taskGraph->addNode(m_objectB.method->getGenerateVelocitiesEndNode());

    // Setup the rigid body handler to move the rigid body according to collision data
    auto pbdCH = std::make_shared<PbdCollisionHandling>();
    pbdCH->setInputObjectA(m_objectA.collider, m_objectA.method);
    pbdCH->setInputObjectB(m_objectB.collider);

    pbdCH->setInputCollisionData(m_colDetect->getCollisionData());
    pbdCH->setFriction(0.0);
    setCollisionHandlingA(pbdCH);

    // Setup the levelset handler to erode the levelset according to collision data
    auto lvlSetCH = std::make_shared<LevelSetCH>();
    lvlSetCH->setInputLvlSetObj(m_objectB.method, m_objectB.collider);
    lvlSetCH->setInputRigidObj(m_objectA.method, m_objectA.collider);
    lvlSetCH->setInputCollisionData(m_colDetect->getCollisionData());
    lvlSetCH->setLevelSetVelocityScaling(getLevelSetVelocityScaling());
    lvlSetCH->setKernel(getLevelSetKernelSize(), getLevelSetKernelSigma());
    lvlSetCH->setUseProportionalVelocity(getUseProportionalVelocity());
    setCollisionHandlingB(lvlSetCH);

    // Since CollisingHandling classes are currently neither of Component, Behaviour, or SceneObject types,
    // they will require explicit initialization inside the Interaction classes where they are instantiated.
    pbdCH->initialize();
    lvlSetCH->initialize();

    m_copyVertToPrevNode = std::make_shared<TaskNode>([ = ]()
        {
            copyVertsToPrevious();
        }, "CopyVertsToPrevious");
    m_taskGraph->addNode(m_copyVertToPrevNode);

    m_computeDisplacementNode =
        std::make_shared<TaskNode>(std::bind(&RigidObjectLevelSetCollision::measureDisplacementFromPrevious, this),
            "ComputeDisplacements");
    m_taskGraph->addNode(m_computeDisplacementNode);

    m_taskGraph->addNode(m_objectA.method->getUpdateNode());
    m_taskGraph->addNode(m_objectA.method->getUpdateGeometryNode());

    m_taskGraph->addNode(m_objectA.taskGraph->getSource());
    m_taskGraph->addNode(m_objectA.taskGraph->getSink());
    m_taskGraph->addNode(m_objectB.taskGraph->getSource());
    m_taskGraph->addNode(m_objectB.taskGraph->getSink());
    return true;
}

void
RigidObjectLevelSetCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                           lvlSetObj2 = std::dynamic_pointer_cast<LevelSetDeformableObject>(m_objB);
    std::shared_ptr<LevelSetModel> lsmModel   = lvlSetObj2->getLevelSetModel();

    std::shared_ptr<TaskNode> pbdHandlerNode = m_collisionHandleANode;
    std::shared_ptr<TaskNode> lsmHandlerNode = m_collisionHandleBNode;

    // Ensure a complete graph
    m_taskGraph->addEdge(source, m_objectA.taskGraph->getSource());
    m_taskGraph->addEdge(m_objectA.taskGraph->getSink(), sink);
    m_taskGraph->addEdge(source, m_objectB.taskGraph->getSource());
    m_taskGraph->addEdge(m_objectB.taskGraph->getSink(), sink);

    // Add pbd process tasks
    m_taskGraph->addChain({
            m_objectA.system->getIntegratePositionNode(),
            m_collisionGeometryUpdateNode,
            m_collisionDetectionNode,
            pbdHandlerNode,
            m_objectA.system->getSolveNode(),
            m_objectA.system->getUpdateVelocityNode()
    });

    ///                   [pbdObject]                               [lvlSetObject]
    ///                Integrate Positions                   LSM Begin Compute Velocities
    ///                                   Collision Detection
    /// pbd collision handler (add constraints)             LSM Handler (pointset erosion)
    ///              pbd Solve                                LSM End Compute Velocities
    ///

    // Add levelset process tasks
    m_taskGraph->addChain({
            lsmModel->getGenerateVelocitiesBeginNode(),
            m_collisionDetectionNode,
            lsmHandlerNode,
            lsmModel->getGenerateVelocitiesEndNode()
    });

    // The tentative body is never actually computed, it should be good to catch the contact
    // in the next frame
    if (m_objectA.displacements)
    {
        // 1.) Copy the vertices at the start of the frame
        m_taskGraph->addChain({ m_objectA.taskGraph->getSource(), m_copyVertToPrevNode, m_objectA.method->getUpdateNode() });

        // If you were to update to tentative, you'd do it here, then compute displacements

        // 2.) Compute the displacements after updating geometry
        m_taskGraph->addChain({ m_objectA.method->getUpdateGeometryNode(), m_computeDisplacementNode, m_objectA.taskGraph->getSink() });
    }
}

void
RigidObjectLevelSetCollision::copyVertsToPrevious()
{
    auto pointSet = m_objectA.physicsGeometry;

    if (m_objectA.displacements)
    {
        auto vertices = pointSet->getVertexPositions();
        if (m_prevVertices->size() != vertices->size())
        {
            m_prevVertices->resize(vertices->size());
        }
        std::copy_n(vertices->getPointer(), vertices->size(), m_prevVertices->getPointer());
    }
}

void
RigidObjectLevelSetCollision::measureDisplacementFromPrevious()
{
    auto pointSet      = m_objectA.physicsGeometry;
    auto displacements = m_objectA.displacements;

    if (pointSet && displacements)
    {
        auto& displacementsArr = *displacements;
        auto& vertices     = *pointSet->getVertexPositions();
        auto& prevVertices = *m_prevVertices;

        ParallelUtils::parallelFor(displacements->size(),
            [&](const int i)
            {
                displacementsArr[i] = vertices[i] - prevVertices[i];
            });
    }
}
} // namespace imstk