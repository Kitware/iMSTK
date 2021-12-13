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
#include "imstkCDObjectFactory.h"
#include "imstkCollisionData.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPBDPickingCH.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

namespace imstk
{
PbdObjectPicking::PbdObjectPicking(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                   std::string cdType) :
    CollisionInteraction("PbdObjectPicking_" + obj1->getName() + "_vs_" + obj2->getName(), obj1, obj2)
{
    // Setup the CD
    std::shared_ptr<CollisionDetectionAlgorithm> cd = CDObjectFactory::makeCollisionDetection(cdType);
    cd->setInput(obj1->getCollidingGeometry(), 0);
    cd->setInput(obj2->getCollidingGeometry(), 1);
    setCollisionDetection(cd);

    // Setup the handler
    auto ch = std::make_shared<PBDPickingCH>();
    ch->setInputObjectA(obj1);
    ch->setInputObjectB(obj2);
    ch->setInputCollisionData(getCollisionDetection()->getCollisionData());
    setCollisionHandlingA(ch);

    m_pickingNode = std::make_shared<TaskNode>([&]()
        {
            // Update collision geometry
            m_objA->updateGeometries();

            // Do collision detection
            getCollisionDetection()->update();

            // Then do handling
            auto pickingCH = std::dynamic_pointer_cast<PBDPickingCH>(getCollisionHandlingA());
            pickingCH->update();
        }, "PbdPickingCD_and_CH", true);
    m_taskGraph->addNode(m_pickingNode);

    m_taskGraph->addNode(obj1->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(obj2->getUpdateGeometryNode());
    m_taskGraph->addNode(obj1->getPbdModel()->getTaskGraph()->getSink());

    m_taskGraph->addNode(obj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
    m_taskGraph->addNode(obj2->getTaskGraph()->getSink());
}

void
PbdObjectPicking::endPick()
{
    auto ch = std::dynamic_pointer_cast<PBDPickingCH>(getCollisionHandlingA());
    if (ch == nullptr)
    {
        LOG(FATAL) << "PbdObjectPicking ending pick, but PBDPickingCH/handlerA is nullptr";
        return;
    }
    else
    {
        ch->endPick();
    }
}

void
PbdObjectPicking::beginPick()
{
    auto ch = std::dynamic_pointer_cast<PBDPickingCH>(getCollisionHandlingA());
    if (ch == nullptr)
    {
        LOG(FATAL) << "PbdObjectPicking beginning pick, but PBDPickingCH/handlerA is nullptr";
        return;
    }
    else
    {
        ch->beginPick();
    }
}

void
PbdObjectPicking::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    auto pbdObj     = std::dynamic_pointer_cast<PbdObject>(m_objA);
    auto pickingObj = m_objB;

    std::shared_ptr<PbdModel> pbdModel = pbdObj->getPbdModel();

    m_taskGraph->addEdge(source, pbdObj->getTaskGraph()->getSource());
    m_taskGraph->addEdge(source, pickingObj->getTaskGraph()->getSource());
    m_taskGraph->addEdge(pbdObj->getTaskGraph()->getSink(), sink);
    m_taskGraph->addEdge(pickingObj->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pbdModel->getTaskGraph()->getSink());

    m_taskGraph->addEdge(pickingObj->getUpdateGeometryNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pickingObj->getTaskGraph()->getSink());
}
}