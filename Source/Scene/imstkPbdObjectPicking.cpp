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

#include "imstkPbdObjectPicking.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkPBDPickingCH.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
// Pbd Collision will be tested before any step of pbd, then resolved after the solve steps of the two objects
PbdObjectPicking::PbdObjectPicking(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                   std::string cdType) : CollisionPair(obj1, obj2)
{
    // Setup the CD
    setCollisionDetection(makeCollisionDetectionObject(cdType, obj1->getCollidingGeometry(), obj2->getCollidingGeometry()));

    // Setup the handler
    auto ch = std::make_shared<PBDPickingCH>();
    ch->setInputObjectA(obj1);
    ch->setInputObjectB(obj2);
    ch->setInputCollisionData(getCollisionDetection()->getCollisionData());
    setCollisionHandlingA(ch);

    m_pickingNode = std::make_shared<TaskNode>([&]()
        {
            // Update collision geometry
            getObjectsPair().first->updateGeometries();

            // Do collision detection
            getCollisionDetection()->update();

            // Then do handling
            auto pickingCH = std::dynamic_pointer_cast<PBDPickingCH>(getCollisionHandlingA());
            pickingCH->update();
        }, "PbdPickingCD_and_CH", true);
}

void
PbdObjectPicking::apply()
{
    // Add the collision interaction
    CollisionPair::apply();

    auto pbdObj     = std::dynamic_pointer_cast<PbdObject>(m_objects.first);
    auto pickingObj = std::dynamic_pointer_cast<CollidingObject>(m_objects.second);

    // Define when too do collision detection and handling between the two objects
    std::shared_ptr<TaskGraph> taskGraphPbd = pbdObj->getTaskGraph();
    std::shared_ptr<TaskGraph> taskGraphPickingObj = pickingObj->getTaskGraph();

    taskGraphPbd->addNode(m_pickingNode);
    taskGraphPickingObj->addNode(m_pickingNode);

    std::shared_ptr<PbdModel> pbdModel = pbdObj->getPbdModel();

    // Do picking after everything in the pbd model
    taskGraphPbd->addEdge(pbdModel->getUpdateVelocityNode(), m_pickingNode);
    taskGraphPbd->addEdge(m_pickingNode, pbdModel->getTaskGraph()->getSink());

    taskGraphPickingObj->addEdge(pickingObj->getUpdateGeometryNode(), m_pickingNode);
    taskGraphPickingObj->addEdge(m_pickingNode, taskGraphPickingObj->getSink());
}
}