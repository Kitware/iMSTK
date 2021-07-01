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
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometry.h"
#include "imstkImplicitGeometryToPointSetCCD.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"

namespace imstk
{
RigidObjectLevelSetCollision::RigidObjectLevelSetCollision(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<LevelSetDeformableObject> obj2) :
    CollisionPair(obj1, obj2), m_prevVertices(std::make_shared<VecDataArray<double, 3>>())
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

    // Define where collision interaction happens
    m_taskNodeInputs.first.push_back(rbdModel->getComputeTentativeVelocitiesNode());
    m_taskNodeInputs.second.push_back(lvlSetModel->getGenerateVelocitiesBeginNode());

    m_taskNodeOutputs.first.push_back(rbdModel->getSolveNode());
    m_taskNodeOutputs.second.push_back(lvlSetModel->getGenerateVelocitiesEndNode());

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

    // Give the point set displacements for CCD, if it doesn't already have them
    auto pointSet = std::dynamic_pointer_cast<PointSet>(obj1->getCollidingGeometry());
    if (pointSet != nullptr && !pointSet->hasVertexAttribute("displacements"))
    {
        auto displacementsPtr = std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices());
        pointSet->setVertexAttribute("displacements", displacementsPtr);
        displacementsPtr->fill(Vec3d::Zero());
    }
}

void
RigidObjectLevelSetCollision::apply()
{
    CollisionPair::apply();

    auto                             obj1     = std::dynamic_pointer_cast<RigidObject2>(m_objects.first);
    std::shared_ptr<RigidBodyModel2> rbdModel = obj1->getRigidBodyModel2();
    std::shared_ptr<PointSet>        pointSet = std::dynamic_pointer_cast<PointSet>(obj1->getPhysicsGeometry());
    const bool                       measureDisplacements = (pointSet != nullptr && pointSet->hasVertexAttribute("displacements"));

    // The tentative body is never actually computed, it should be good to catch the contact
    // in the next frame
    if (measureDisplacements)
    {
        // 1.) Copy the vertices at the start of the frame
        obj1->getTaskGraph()->insertBefore(obj1->getRigidBodyModel2()->getComputeTentativeVelocitiesNode(),
            std::make_shared<TaskNode>([ = ]()
            {
                copyVertsToPrevious();
                }, "CopyVertsToPrevious"));

        // If you were to update to tentative, you'd do it here, then compute displacements

        // 2.) Compute the displacements after updating geometry
        std::shared_ptr<TaskNode> computeDisplacements =
            std::make_shared<TaskNode>(std::bind(&RigidObjectLevelSetCollision::measureDisplacementFromPrevious, this), "ComputeDisplacements");
        obj1->getTaskGraph()->insertAfter(obj1->getUpdateGeometryNode(),
            computeDisplacements);
    }
}

void
RigidObjectLevelSetCollision::copyVertsToPrevious()
{
    auto                      obj1     = std::dynamic_pointer_cast<RigidObject2>(m_objects.first);
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
    auto                      obj1     = std::dynamic_pointer_cast<RigidObject2>(m_objects.first);
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