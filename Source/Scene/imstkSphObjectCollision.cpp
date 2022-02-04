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

#include "imstkSphObjectCollision.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkImplicitGeometryToPointSetCD.h"
#include "imstkSphCollisionHandling.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkTaskGraph.h"

namespace imstk
{
// Pbd Collision will be tested before any step of pbd, then resolved after the solve steps of the two objects
SphObjectCollision::SphObjectCollision(std::shared_ptr<SphObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                       std::string cdType) :
    CollisionInteraction("SphObjectCollision_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2)
{
    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    cd->setInput(obj1->getCollidingGeometry(), 0);
    cd->setInput(obj2->getCollidingGeometry(), 1);
    cd->setGenerateCD(true, false); // CD data is only needed for the SPHObject geometry
    setCollisionDetection(cd);

    // Setup the handler
    std::shared_ptr<SphCollisionHandling> ch = std::make_shared<SphCollisionHandling>();
    ch->setInputObjectA(obj1);
    ch->setInputCollisionData(cd->getCollisionData());
    ch->setDetection(cd);
    setCollisionHandlingA(ch);

    // Collision should happen after positions and velocities are computed
    m_taskGraph->addNode(obj1->getUpdateGeometryNode());
    m_taskGraph->addNode(obj2->getUpdateGeometryNode());

    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSink());
}

void
SphObjectCollision::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    CollisionInteraction::initGraphEdges(source, sink);

    auto sphObj1 = std::dynamic_pointer_cast<SphObject>(m_objA);

    //
    // ...Sph steps...
    // Update Geometry A                  Update Geometry B
    //                 Collision Detection
    //                 Collision Handling A
    //    objA Sink                          objB Sink
    //
    m_taskGraph->addEdge(sphObj1->getUpdateGeometryNode(), m_collisionDetectionNode);
    m_taskGraph->addEdge(m_objB->getUpdateGeometryNode(), m_collisionDetectionNode);

    m_taskGraph->addEdge(m_collisionDetectionNode, m_collisionHandleANode);

    m_taskGraph->addEdge(m_collisionHandleANode, sphObj1->getTaskGraph()->getSink());
    m_taskGraph->addEdge(m_collisionHandleANode, m_objB->getTaskGraph()->getSink());
}
} // namespace imstk