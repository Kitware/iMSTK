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
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVecDataArray.h"

namespace imstk
{
RigidObjectLevelSetCollision::RigidObjectLevelSetCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<LevelSetDeformableObject> obj2) :
    CollisionInteraction("RigidObjectLevelSetCollision" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2, ""),
    m_prevVertices(std::make_shared<VecDataArray<double, 3>>())
{
    std::shared_ptr<PbdModel>      pbdModel    = obj1->getPbdModel();
    std::shared_ptr<LevelSetModel> lvlSetModel = obj2->getLevelSetModel();

    if (pbdModel == nullptr || lvlSetModel == nullptr)
    {
        LOG(WARNING) << "Cannot create collision pair.";
        return;
    }
    if (!obj1->getPbdBody()->isRigid())
    {
        LOG(WARNING) << "PBD object should be rigid.";
        return;
    }

    // Here the CH's adds constraints to the system on the LHS, and impulses to the levelset RHS
    m_taskGraph->addNode(pbdModel->getSolveNode());
    m_taskGraph->addNode(pbdModel->getIntegratePositionNode());
    m_taskGraph->addNode(pbdModel->getUpdateVelocityNode());

    m_taskGraph->addNode(lvlSetModel->getGenerateVelocitiesBeginNode());
    m_taskGraph->addNode(lvlSetModel->getGenerateVelocitiesEndNode());

    // Setup the rigid body handler to move the rigid body according to collision data
    auto pbdCH = std::make_shared<PbdCollisionHandling>();
    pbdCH->setInputObjectA(obj1);
    pbdCH->setInputObjectB(obj2);

    pbdCH->setInputCollisionData(m_colDetect->getCollisionData());
    pbdCH->setFriction(0.0);
    setCollisionHandlingA(pbdCH);

    // Setup the levelset handler to erode the levelset according to collision data
    auto lvlSetCH = std::make_shared<LevelSetCH>();
    lvlSetCH->setInputLvlSetObj(obj2);
    lvlSetCH->setInputRigidObj(obj1);
    lvlSetCH->setInputCollisionData(m_colDetect->getCollisionData());
    setCollisionHandlingB(lvlSetCH);

    m_copyVertToPrevNode = std::make_shared<TaskNode>([ = ]()
        {
            copyVertsToPrevious();
        }, "CopyVertsToPrevious");
    m_taskGraph->addNode(m_copyVertToPrevNode);

    m_computeDisplacementNode =
        std::make_shared<TaskNode>(std::bind(&RigidObjectLevelSetCollision::measureDisplacementFromPrevious, this),
            "ComputeDisplacements");
    m_taskGraph->addNode(m_computeDisplacementNode);

    // Give the point set displacements for CCD, if it doesn't already have them
    auto pointSet = std::dynamic_pointer_cast<PointSet>(Collider::getCollidingGeometryFromEntity(obj1.get()));
    if (pointSet != nullptr && !pointSet->hasVertexAttribute("displacements"))
    {
        auto displacementsPtr = std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices());
        pointSet->setVertexAttribute("displacements", displacementsPtr);
        displacementsPtr->fill(Vec3d::Zero());
    }

    m_taskGraph->addNode(obj1->getUpdateNode());
    m_taskGraph->addNode(obj1->getUpdateGeometryNode());

    m_taskGraph->addNode(obj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSink());
}

void
RigidObjectLevelSetCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                      pbdObjA  = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<PbdModel> pbdModel = pbdObjA->getPbdModel();

    auto                           lvlSetObj2 = std::dynamic_pointer_cast<LevelSetDeformableObject>(m_objB);
    std::shared_ptr<LevelSetModel> lsmModel   = lvlSetObj2->getLevelSetModel();

    std::shared_ptr<TaskNode> pbdHandlerNode = m_collisionHandleANode;
    std::shared_ptr<TaskNode> lsmHandlerNode = m_collisionHandleBNode;

    // Ensure a complete graph
    m_taskGraph->addEdge(source, pbdObjA->getTaskGraph()->getSource());
    m_taskGraph->addEdge(pbdObjA->getTaskGraph()->getSink(), sink);
    m_taskGraph->addEdge(source, lvlSetObj2->getTaskGraph()->getSource());
    m_taskGraph->addEdge(lvlSetObj2->getTaskGraph()->getSink(), sink);

    // Add pbd process tasks
    m_taskGraph->addEdge(pbdModel->getIntegratePositionNode(), m_collisionGeometryUpdateNode);
    m_taskGraph->addEdge(m_collisionGeometryUpdateNode, m_collisionDetectionNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, pbdHandlerNode);
    m_taskGraph->addEdge(pbdHandlerNode, pbdModel->getSolveNode());
    m_taskGraph->addEdge(pbdModel->getSolveNode(), pbdModel->getUpdateVelocityNode());

    ///                   [pbdObject]                               [lvlSetObject]
    ///                Integrate Positions                   LSM Begin Compute Velocities
    ///                                   Collision Detection
    /// pbd collision handler (add constraints)             LSM Handler (pointset erosion)
    ///              pbd Solve                                LSM End Compute Velocities
    ///

    // Add levelset process tasks
    m_taskGraph->addEdge(lsmModel->getGenerateVelocitiesBeginNode(), m_collisionDetectionNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, lsmHandlerNode);
    m_taskGraph->addEdge(lsmHandlerNode, lsmModel->getGenerateVelocitiesEndNode());

    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(pbdObjA->getPhysicsGeometry());
    const bool                measureDisplacements = (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"));

    // The tentative body is never actually computed, it should be good to catch the contact
    // in the next frame
    if (measureDisplacements)
    {
        // 1.) Copy the vertices at the start of the frame
        m_taskGraph->addEdge(pbdObjA->getTaskGraph()->getSource(), m_copyVertToPrevNode);
        m_taskGraph->addEdge(m_copyVertToPrevNode, pbdObjA->getUpdateNode());

        // If you were to update to tentative, you'd do it here, then compute displacements

        // 2.) Compute the displacements after updating geometry
        m_taskGraph->addEdge(pbdObjA->getUpdateGeometryNode(), m_computeDisplacementNode);
        m_taskGraph->addEdge(m_computeDisplacementNode, pbdObjA->getTaskGraph()->getSink());
    }
}

void
RigidObjectLevelSetCollision::copyVertsToPrevious()
{
    auto                      obj1     = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(obj1->getPhysicsGeometry());

    if (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"))
    {
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr  = pointSet->getVertexPositions();
        VecDataArray<double, 3>&                 vertices     = *verticesPtr;
        VecDataArray<double, 3>&                 prevVertices = *m_prevVertices;

        if (prevVertices.size() != vertices.size())
        {
            prevVertices.resize(vertices.size());
        }
        std::copy_n(vertices.getPointer(), vertices.size(), prevVertices.getPointer());
    }
}

void
RigidObjectLevelSetCollision::measureDisplacementFromPrevious()
{
    auto                      obj1     = std::dynamic_pointer_cast<PbdObject>(m_objA);
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(obj1->getPhysicsGeometry());

    if (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"))
    {
        std::shared_ptr<VecDataArray<double, 3>> displacements =
            std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSet->getVertexAttribute("displacements"));
        VecDataArray<double, 3>& displacementsArr = *displacements;

        std::shared_ptr<VecDataArray<double, 3>> verticesPtr  = pointSet->getVertexPositions();
        VecDataArray<double, 3>&                 vertices     = *verticesPtr;
        VecDataArray<double, 3>&                 prevVertices = *m_prevVertices;

        ParallelUtils::parallelFor(displacements->size(),
            [&](const int i)
            {
                displacementsArr[i] = vertices[i] - prevVertices[i];
            });
    }
}
} // namespace imstk