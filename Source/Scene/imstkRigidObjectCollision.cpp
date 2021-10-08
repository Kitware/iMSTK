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
                                           std::string cdType) : CollisionPair(rbdObj1, obj2)
{
    std::shared_ptr<RigidBodyModel2> model1 = rbdObj1->getRigidBodyModel2();

    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    cd->setInput(rbdObj1->getCollidingGeometry(), 0);
    cd->setInput(obj2->getCollidingGeometry(), 1);
    setCollisionDetection(cd);

    auto ch = std::make_shared<RigidBodyCH>();
    ch->setInputCollisionData(cd->getCollisionData());
    ch->setInputObjectA(rbdObj1);
    ch->setInputObjectB(obj2);

    if (auto rbdObj2 = std::dynamic_pointer_cast<RigidObject2>(obj2))
    {
        std::shared_ptr<RigidBodyModel2> model2 = rbdObj2->getRigidBodyModel2();

        // Here we use RigidBodyCH which generates constraints for the rigid body model
        // This step is done *after* tentative velocities have been computed but *before*
        // constraints and new velocities are solved.

        // Define where collision interaction happens
        m_taskNodeInputs.first.push_back(model1->getComputeTentativeVelocitiesNode());
        m_taskNodeInputs.second.push_back(model2->getComputeTentativeVelocitiesNode());

        m_taskNodeOutputs.first.push_back(model1->getSolveNode());
        m_taskNodeOutputs.second.push_back(model2->getSolveNode());

        setCollisionHandlingAB(ch);
    }
    else
    {
        // Define where collision interaction happens
        m_taskNodeInputs.first.push_back(model1->getComputeTentativeVelocitiesNode());
        m_taskNodeInputs.second.push_back(obj2->getTaskGraph()->getSource());

        m_taskNodeOutputs.first.push_back(model1->getSolveNode());
        m_taskNodeOutputs.second.push_back(obj2->getUpdateNode());

        // Setup the handlers for only A and inform CD it only needs to generate A
        cd->setGenerateCD(true, false);
        setCollisionHandlingA(ch);
    }
}

void
RigidObjectCollision::setStiffness(double stiffness)
{
    std::dynamic_pointer_cast<RigidBodyCH>(getCollisionHandlingA())->setBeta(stiffness);
}

const double
RigidObjectCollision::getStiffness() const
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
RigidObjectCollision::apply()
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
RigidObjectCollision::copyVertsToPrevious()
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
RigidObjectCollision::measureDisplacementFromPrevious()
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