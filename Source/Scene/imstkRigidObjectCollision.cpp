/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRigidObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkParallelFor.h"
#include "imstkPointSet.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"
#include "imstkVecDataArray.h"

namespace imstk
{
RigidObjectCollision::RigidObjectCollision(std::shared_ptr<RigidObject2> rbdObj1, std::shared_ptr<CollidingObject> obj2,
                                           std::string cdType) :
    CollisionInteraction("RigidObjectCollision" + rbdObj1->getName() + "_vs_" + obj2->getName(), rbdObj1, obj2, cdType)
{
    std::shared_ptr<RigidBodyModel2> model1 = rbdObj1->getRigidBodyModel2();

    // Only one handler is used, this means we only support one-way collisions or two-way
    // If you want two one-way's, use two RigidObjectCollisions
    auto ch = std::make_shared<RigidBodyCH>();
    ch->setInputCollisionData(m_colDetect->getCollisionData());
    ch->setInputObjectA(rbdObj1);
    ch->setInputObjectB(obj2);

    m_copyVertToPrevNode = std::make_shared<TaskNode>([ = ]()
        {
            copyVertsToPrevious();
        }, "CopyVertsToPrevious");
    m_taskGraph->addNode(m_copyVertToPrevNode);
    m_computeDisplacementNode = std::make_shared<TaskNode>([ = ]()
        {
            measureDisplacementFromPrevious();
        }, "ComputeDisplacements");
    m_taskGraph->addNode(m_computeDisplacementNode);

    if (auto rbdObj2 = std::dynamic_pointer_cast<RigidObject2>(obj2))
    {
        std::shared_ptr<RigidBodyModel2> model2 = rbdObj2->getRigidBodyModel2();

        // These could possibly be the same node if they belong to the same system
        // Handled implicitly
        m_taskGraph->addNode(model1->getComputeTentativeVelocitiesNode());
        m_taskGraph->addNode(model2->getComputeTentativeVelocitiesNode());

        m_taskGraph->addNode(model1->getSolveNode());
        m_taskGraph->addNode(model2->getSolveNode());

        setCollisionHandlingAB(ch);
    }
    else
    {
        // Define where collision interaction happens
        m_taskGraph->addNode(model1->getComputeTentativeVelocitiesNode());
        m_taskGraph->addNode(obj2->getTaskGraph()->getSource());

        m_taskGraph->addNode(model1->getSolveNode());
        m_taskGraph->addNode(obj2->getUpdateNode());

        // Setup the handlers for only A and inform CD it only needs to generate A
        m_colDetect->setGenerateCD(true, false);
        setCollisionHandlingA(ch);
    }

    m_taskGraph->addNode(rbdObj1->getUpdateGeometryNode());
    m_taskGraph->addNode(rbdObj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(rbdObj1->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSink());
}

void
RigidObjectCollision::setBaumgarteStabilization(double beta)
{
    std::dynamic_pointer_cast<RigidBodyCH>(getCollisionHandlingA())->setBaumgarteStabilization(beta);
}

const double
RigidObjectCollision::getBeta() const
{
    return std::dynamic_pointer_cast<RigidBodyCH>(getCollisionHandlingA())->getBeta();
}

void
RigidObjectCollision::setFriction(double frictionalCoefficient)
{
    std::dynamic_pointer_cast<RigidBodyCH>(getCollisionHandlingA())->setFriction(frictionalCoefficient);
}

const double
RigidObjectCollision::getFriction() const
{
    return std::dynamic_pointer_cast<RigidBodyCH>(getCollisionHandlingA())->getFriction();
}

void
RigidObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                             rbdObj1   = std::dynamic_pointer_cast<RigidObject2>(m_objA);
    std::shared_ptr<RigidBodyModel2> rbdModel1 = rbdObj1->getRigidBodyModel2();

    std::shared_ptr<TaskNode> handlerABNode = m_collisionHandleANode;

    if (auto rbdObj2 = std::dynamic_pointer_cast<RigidObject2>(m_objB))
    {
        std::shared_ptr<RigidBodyModel2> rbdModel2 = rbdObj2->getRigidBodyModel2();

        // Note: ComputeTenative and RbdModel may be the same
        // ComputeTenative Velocities 1   ComputeTenative Velocities 2
        //                   Collision Detection
        //                   Collision Handling
        //       Rbd Solve 1                      Rbd Solve 2
        m_taskGraph->addEdge(rbdModel1->getComputeTentativeVelocitiesNode(), m_collisionDetectionNode);
        m_taskGraph->addEdge(rbdModel2->getComputeTentativeVelocitiesNode(), m_collisionDetectionNode);

        m_taskGraph->addEdge(m_collisionDetectionNode, handlerABNode);
        m_taskGraph->addEdge(handlerABNode, rbdModel1->getSolveNode());
        m_taskGraph->addEdge(handlerABNode, rbdModel2->getSolveNode());
    }
    else
    {
        // Note: ComputeTenative and RbdModel may be the same
        /* ComputeTenative Velocities 1   CollidingObject Source
                           Collision Detection
               Collision Handling         \
               Rbd Solve 1               CollidingObject Update */
        m_taskGraph->addEdge(rbdModel1->getComputeTentativeVelocitiesNode(), m_collisionDetectionNode);
        m_taskGraph->addEdge(m_objB->getTaskGraph()->getSource(), m_collisionDetectionNode);

        m_taskGraph->addEdge(m_collisionDetectionNode, handlerABNode);
        m_taskGraph->addEdge(handlerABNode, rbdModel1->getSolveNode());
        m_taskGraph->addEdge(m_collisionDetectionNode, m_objB->getUpdateNode());
    }

    // \todo: This should be handled differently (per object, not per interaction)
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(rbdObj1->getPhysicsGeometry());
    const bool                measureDisplacements = (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"));

    // The tentative body is never actually computed, it should be good to catch the contact
    // in the next frame
    if (measureDisplacements)
    {
        // 1.) Copy the vertices at the start of the frame
        m_taskGraph->addEdge(rbdObj1->getTaskGraph()->getSource(), m_copyVertToPrevNode);
        m_taskGraph->addEdge(m_copyVertToPrevNode, rbdObj1->getRigidBodyModel2()->getComputeTentativeVelocitiesNode());

        // If you were to update to tentative, you'd do it here, then compute displacements

        // 2.) Compute the displacements after updating geometry
        m_taskGraph->addEdge(rbdObj1->getUpdateGeometryNode(), m_computeDisplacementNode);
        m_taskGraph->addEdge(m_computeDisplacementNode, rbdObj1->getTaskGraph()->getSink());
    }
}

void
RigidObjectCollision::copyVertsToPrevious()
{
    auto                      obj1     = std::dynamic_pointer_cast<RigidObject2>(m_objA);
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
RigidObjectCollision::measureDisplacementFromPrevious()
{
    auto                      obj1     = std::dynamic_pointer_cast<RigidObject2>(m_objA);
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