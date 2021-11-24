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

#include "imstkPbdObjectCutting.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCollidingObject.h"
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshCut.h"
#include "imstkVecDataArray.h"

namespace imstk
{
PbdObjectCutting::PbdObjectCutting(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> cutObj) :
    SceneObject(pbdObj->getName() + "_vs_" + cutObj->getName() + "_pbdCutting"), m_objA(pbdObj), m_objB(cutObj)
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
PbdObjectCutting::apply()
{
    auto pbdModel = m_objA->getPbdModel();
    auto pbdMesh  = std::static_pointer_cast<SurfaceMesh>(pbdModel->getModelGeometry());

    m_addConstraintVertices->clear();
    m_removeConstraintVertices->clear();

    // Perform cutting
    imstkNew<SurfaceMeshCut> surfCut;
    surfCut->setInputMesh(pbdMesh);
    surfCut->setCutGeometry(m_objB->getCollidingGeometry());
    surfCut->update();
    auto newPbdMesh = surfCut->getOutputMesh();

    // Only remove and add constraints related to the topological changes
    m_removeConstraintVertices = surfCut->getRemoveConstraintVertices();
    m_addConstraintVertices    = surfCut->getAddConstraintVertices();

    // update pbd mesh
    pbdMesh->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newPbdMesh->getInitialVertexPositions()));
    pbdMesh->setVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newPbdMesh->getVertexPositions()));
    pbdMesh->setTriangleIndices(std::make_shared<VecDataArray<int, 3>>(*newPbdMesh->getTriangleIndices()));
    pbdMesh->postModified();

    // update pbd states, constraints and solver
    pbdModel->initState();
    pbdModel->getConstraints()->removeConstraints(m_removeConstraintVertices);
    // pbdModel->getConstraints()->addConstraintVertices(m_addConstraintVertices);
    pbdModel->addConstraints(m_addConstraintVertices);
    pbdModel->getSolver()->setInvMasses(pbdModel->getInvMasses());
    pbdModel->getSolver()->setPositions(pbdModel->getCurrentState()->getPositions());
}

void
PbdObjectCutting::addVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                              std::shared_ptr<VecDataArray<double, 3>> newVertices,
                              std::shared_ptr<VecDataArray<double, 3>> newInitialVertices)
{
    auto vertices = pbdMesh->getVertexPositions();
    auto initialVertices = pbdMesh->getInitialVertexPositions();

    auto nVertices    = vertices->size();
    auto nNewVertices = newVertices->size();
    if (nNewVertices != newInitialVertices->size())
    {
        LOG(WARNING) << "Number of new vertices does not match number of new initial vertices";
        return;
    }

    vertices->reserve(nVertices + nNewVertices);
    initialVertices->reserve(nVertices + nNewVertices);
    for (int i = 0; i < nNewVertices; ++i)
    {
        vertices->push_back((*newVertices)[i]);
        initialVertices->push_back((*newInitialVertices)[i]);
    }
}

void
PbdObjectCutting::modifyVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                                 std::shared_ptr<std::vector<size_t>> modifiedVertexIndices,
                                 std::shared_ptr<VecDataArray<double, 3>> modifiedVertices,
                                 std::shared_ptr<VecDataArray<double, 3>> modifiedInitialVertices)
{
    auto vertices = pbdMesh->getVertexPositions();
    auto initialVertices = pbdMesh->getInitialVertexPositions();

    auto nModifiedVertices = modifiedVertices->size();
    if (nModifiedVertices != modifiedInitialVertices->size()
        || static_cast<size_t>(nModifiedVertices) != modifiedVertexIndices->size())
    {
        LOG(WARNING) << "Numbers of vertices do not match.";
        return;
    }

    for (int i = 0; i < nModifiedVertices; ++i)
    {
        auto vertexIdx = modifiedVertexIndices->at(i);
        (*vertices)[vertexIdx] = (*modifiedVertices)[i];
        (*initialVertices)[vertexIdx] = (*modifiedInitialVertices)[i];
        m_removeConstraintVertices->insert(vertexIdx);
        m_addConstraintVertices->insert(vertexIdx);
    }
}

void
PbdObjectCutting::addTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                               std::shared_ptr<VecDataArray<int, 3>> newTriangles)
{
    auto triangles     = pbdMesh->getTriangleIndices();
    auto nTriangles    = triangles->size();
    auto nNewTriangles = newTriangles->size();

    triangles->reserve(nTriangles + nNewTriangles);
    for (int i = 0; i < nNewTriangles; ++i)
    {
        auto& tri = (*newTriangles)[i];
        triangles->push_back(tri);
        m_addConstraintVertices->insert(tri[0]);
        m_addConstraintVertices->insert(tri[1]);
        m_addConstraintVertices->insert(tri[2]);
    }
}

void
PbdObjectCutting::modifyTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                                  std::shared_ptr<std::vector<size_t>> modifiedTriangleIndices,
                                  std::shared_ptr<VecDataArray<int, 3>> modifiedTriangles)
{
    auto triangles = pbdMesh->getTriangleIndices();
    auto nModifiedTriangles = static_cast<size_t>(modifiedTriangles->size());
    if (nModifiedTriangles != modifiedTriangleIndices->size())
    {
        LOG(WARNING) << "Numbers of vertices do not match.";
        return;
    }

    for (size_t i = 0; i < nModifiedTriangles; ++i)
    {
        auto  triId  = (*modifiedTriangleIndices)[i];
        auto& oldTri = (*triangles)[triId];
        m_removeConstraintVertices->insert(oldTri[0]);
        m_removeConstraintVertices->insert(oldTri[1]);
        m_removeConstraintVertices->insert(oldTri[2]);

        auto& newTri = (*modifiedTriangles)[i];
        (*triangles)[triId] = newTri;
        m_addConstraintVertices->insert(newTri[0]);
        m_addConstraintVertices->insert(newTri[1]);
        m_addConstraintVertices->insert(newTri[2]);
    }
}
}