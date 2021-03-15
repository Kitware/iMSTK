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

#include "imstkPbdObjectCuttingPair.h"

#include "imstkAnalyticalGeometry.h"
#include "imstkCollidingObject.h"
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshCut.h"

namespace imstk
{
PbdObjectCuttingPair::PbdObjectCuttingPair(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> cutObj) : ObjectInteractionPair(pbdObj, cutObj)
{
    // check whether pbd object is a surfacemesh
    if (std::dynamic_pointer_cast<SurfaceMesh>(pbdObj->getPhysicsGeometry()) == nullptr)
    {
        LOG(WARNING) << "PbdObj is not a SurfaceMesh, could not create cutting pair";
        return;
    }

    // check whether cut object is valid
    if (std::dynamic_pointer_cast<SurfaceMesh>(cutObj->getCollidingGeometry()) == nullptr
        && std::dynamic_pointer_cast<AnalyticalGeometry>(cutObj->getCollidingGeometry()) == nullptr)
    {
        LOG(WARNING) << "CutObj is neither a SurfaceMesh nor an AnalyticalGeometry, could not create cutting pair";
        return;
    }
}

void
PbdObjectCuttingPair::apply()
{
    auto pbdObj   = std::static_pointer_cast<PbdObject>(m_objects.first);
    auto cutObj   = std::static_pointer_cast<CollidingObject>(m_objects.second);
    auto pbdMesh  = std::static_pointer_cast<SurfaceMesh>(pbdObj->getPhysicsGeometry());
    auto pbdModel = pbdObj->getPbdModel();

    // perform cutting
    imstkNew<SurfaceMeshCut> surfCut;
    surfCut->setInputMesh(pbdMesh);
    surfCut->setCutGeometry(cutObj->getCollidingGeometry());
    surfCut->update();
    auto newPbdMesh = surfCut->getOutputMesh();

    // update pbd mesh
    pbdMesh->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newPbdMesh->getInitialVertexPositions()));
    pbdMesh->setVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newPbdMesh->getVertexPositions()));
    pbdMesh->setTriangleIndices(std::make_shared<VecDataArray<int, 3>>(*newPbdMesh->getTriangleIndices()));
    pbdMesh->modified();

    // update pbd states, constraints and solver
    pbdModel->initState();
    pbdModel->removeConstraints(surfCut->getRemoveConstraintVertices());
    pbdModel->addConstraints(surfCut->getAddConstraintVertices());
    pbdModel->getSolver()->setInvMasses(pbdModel->getInvMasses());
    pbdModel->getSolver()->setPositions(pbdModel->getCurrentState()->getPositions());
}
}