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

#include "imstkRigidObjectLevelSetCollision.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometry.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkParallelFor.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVecDataArray.h"

namespace imstk
{
RigidObjectLevelSetCollision::RigidObjectLevelSetCollision(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<LevelSetDeformableObject> obj2) :
    CollisionInteraction(obj1, obj2), m_prevVertices(std::make_shared<VecDataArray<double, 3>>())
{
    std::shared_ptr<RigidBodyModel2> rbdModel    = obj1->getRigidBodyModel2();
    std::shared_ptr<LevelSetModel>   lvlSetModel = obj2->getLevelSetModel();

    if (rbdModel == nullptr || lvlSetModel == nullptr)
    {
        LOG(WARNING) << "RigidObjectCollisionPair, could not create";
        return;
    }

    // Graph of explicit interaction
    // [compute tentative velocities]  [lvlSetSource]
    //                           [CD]
    //     [Add rigid constraints]    [Apply impulses]
    //       [Solve rbd system]       [Evolve levelset]
    // Here the CH's adds constraints to the system on the LHS, and impulses to the levelset RHS

    m_taskGraph->addNode(rbdModel->getComputeTentativeVelocitiesNode());
    m_taskGraph->addNode(lvlSetModel->getGenerateVelocitiesBeginNode());

    m_taskGraph->addNode(rbdModel->getSolveNode());
    m_taskGraph->addNode(lvlSetModel->getGenerateVelocitiesEndNode());

    // Setup the CD
    auto cd = std::make_shared<ImplicitGeometryToPointSetCCD>();
    cd->setInputGeometryA(obj1->getCollidingGeometry());
    cd->setInputGeometryB(obj2->getCollidingGeometry());
    setCollisionDetection(cd);

    // Setup the rigid body handler to move the rigid body according to collision data
    auto rbdCH = std::make_shared<RigidBodyCH>();
    rbdCH->setInputRigidObjectA(obj1);
    rbdCH->setInputCollidingObjectB(obj2);
    rbdCH->setInputCollisionData(cd->getCollisionData());
    rbdCH->setFriction(0.0);
    setCollisionHandlingA(rbdCH);

    // Setup the levelset handler to erode the levelset according to collision data
    auto lvlSetCH = std::make_shared<LevelSetCH>();
    lvlSetCH->setInputLvlSetObj(obj2);
    lvlSetCH->setInputRigidObj(obj1);
    lvlSetCH->setInputCollisionData(cd->getCollisionData());
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
    auto pointSet = std::dynamic_pointer_cast<PointSet>(obj1->getCollidingGeometry());
    if (pointSet != nullptr && !pointSet->hasVertexAttribute("displacements"))
    {
        auto displacementsPtr = std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices());
        pointSet->setVertexAttribute("displacements", displacementsPtr);
        displacementsPtr->fill(Vec3d::Zero());
    }

    m_taskGraph->addNode(obj1->getUpdateGeometryNode());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
}

void
RigidObjectLevelSetCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto                             rbdObj1  = std::dynamic_pointer_cast<RigidObject2>(m_objA);
    std::shared_ptr<RigidBodyModel2> rbdModel = rbdObj1->getRigidBodyModel2();

    auto                           lvlSetObj2 = std::dynamic_pointer_cast<LevelSetDeformableObject>(m_objB);
    std::shared_ptr<LevelSetModel> lsmModel   = lvlSetObj2->getLevelSetModel();

    std::shared_ptr<TaskNode> rbdHandlerNode = m_collisionHandleANode;
    std::shared_ptr<TaskNode> lsmHandlerNode = m_collisionHandleBNode;

    ///
    /// Compute Tentative Velocities (pre col vel)         LSM Begin Compute Velocities
    ///                                Collision Detection
    /// Rigid Body Handler (add constraints)               LSM Handler (pointset erosion)
    ///              Rbd Solve                              LSM End Compute Velocities
    /// 
    m_taskGraph->addEdge(rbdModel->getComputeTentativeVelocitiesNode(), m_collisionDetectionNode);
    m_taskGraph->addEdge(lsmModel->getGenerateVelocitiesBeginNode(), m_collisionDetectionNode);

    m_taskGraph->addEdge(m_collisionDetectionNode, rbdHandlerNode);
    m_taskGraph->addEdge(m_collisionDetectionNode, lsmHandlerNode);

    m_taskGraph->addEdge(rbdHandlerNode, rbdModel->getSolveNode());
    m_taskGraph->addEdge(lsmHandlerNode, lsmModel->getGenerateVelocitiesEndNode());

    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(rbdObj1->getPhysicsGeometry());
    const bool                measureDisplacements = (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"));

    // The tentative body is never actually computed, it should be good to catch the contact
    // in the next frame
    if (measureDisplacements)
    {
        // 1.) Copy the vertices at the start of the frame
        m_taskGraph->addEdge(rbdObj1->getTaskGraph()->getSource(), m_copyVertToPrevNode);
        m_taskGraph->addEdge(m_copyVertToPrevNode,
            rbdObj1->getRigidBodyModel2()->getComputeTentativeVelocitiesNode());

        // If you were to update to tentative, you'd do it here, then compute displacements

        // 2.) Compute the displacements after updating geometry
        m_taskGraph->addEdge(rbdObj1->getUpdateGeometryNode(), m_computeDisplacementNode);
        m_taskGraph->addEdge(m_computeDisplacementNode, rbdObj1->getTaskGraph()->getSink());
    }
}

void
RigidObjectLevelSetCollision::copyVertsToPrevious()
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
RigidObjectLevelSetCollision::measureDisplacementFromPrevious()
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
}