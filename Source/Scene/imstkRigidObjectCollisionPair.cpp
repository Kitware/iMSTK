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

#include "imstkRigidObjectCollisionPair.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkPointSet.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkTaskGraph.h"

namespace imstk
{
namespace expiremental
{
RigidObjectCollisionPair::RigidObjectCollisionPair(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<RigidObject2> obj2,
                                                   CollisionDetection::Type cdType) : CollisionPair(obj1, obj2)
{
    std::shared_ptr<RigidBodyModel2> model1 = obj1->getRigidBodyModel2();
    std::shared_ptr<RigidBodyModel2> model2 = obj2->getRigidBodyModel2();

    if (model1 == nullptr || model2 == nullptr)
    {
        LOG(WARNING) << "RigidObjectCollisionPair, could not create";
        return;
    }

    // Here we use RigidBodyCH which generates constraints for the rigid body model
    // This step is done *after* tentative velocities have been computed but *before*
    // constraints and new velocities are solved.

    // Define where collision interaction happens
    m_taskNodeInputs.first.push_back(model1->getComputeTentativeVelocitiesNode());
    m_taskNodeInputs.second.push_back(model2->getComputeTentativeVelocitiesNode());

    m_taskNodeOutputs.first.push_back(model1->getSolveNode());
    m_taskNodeOutputs.second.push_back(model2->getSolveNode());

    // Setup the CD
    m_colData = std::make_shared<CollisionData>();
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry(), m_colData));

    // Setup the handler
    CollisionHandling::Side side = CollisionHandling::Side::AB;
    // If both objects use the same model, we need only add one two-way constraint to the system
    if (model1 == model2)
    {
        side = CollisionHandling::Side::AB;
    }
    else
    {
        LOG(WARNING) << "RigidObjectCollisionPair, specified RigidObjects have differing models, one-way constraints not supported yet";
        return;
    }
    auto ch = std::make_shared<RigidBodyCH>(side, m_colData, obj1, obj2);
    setCollisionHandlingAB(ch);
}

void
RigidObjectCollisionPair::apply()
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
        obj1->getTaskGraph()->insertAfter(obj1->getUpdateGeometryNode(),
            std::make_shared<TaskNode>([ = ]()
                {
                    measureDisplacementFromPrevious();
                }, "ComputeDisplacements"));
    }
}

void
RigidObjectCollisionPair::copyVertsToPrevious()
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
RigidObjectCollisionPair::measureDisplacementFromPrevious()
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
}