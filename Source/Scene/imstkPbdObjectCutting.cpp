/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAnalyticalGeometry.h"
#include "imstkCollider.h"
#include "imstkLineMesh.h"
#include "imstkLineMeshCut.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdObjectCutting.h"
#include "imstkPbdSolver.h"
#include "imstkPbdSystem.h"
#include "imstkPbdMethod.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshCut.h"

namespace imstk
{
PbdObjectCutting::PbdObjectCutting(std::shared_ptr<PbdMethod> cuttableObject, std::shared_ptr<Collider> cutterObject) :
    m_cuttable(cuttableObject), m_cutter(cutterObject)
{
    // check whether the cutable object is valid
    CHECK(std::dynamic_pointer_cast<SurfaceMesh>(m_cuttable->getGeometry()) != nullptr
        || std::dynamic_pointer_cast<LineMesh>(m_cuttable->getGeometry())) <<
        "Cutable is not a SurfaceMesh, could not create cutting pair";

    // check whether the cutter object is valid
    if (std::dynamic_pointer_cast<SurfaceMesh>(m_cutter->getGeometry()) == nullptr
        && std::dynamic_pointer_cast<AnalyticalGeometry>(m_cutter->getGeometry()) == nullptr)
    {
        LOG(WARNING) << "Cutter is neither a SurfaceMesh nor an AnalyticalGeometry, could not create cutting pair";
        return;
    }

    m_objBCollisionGeometry = m_cutter->getGeometry();
    CHECK(m_objBCollisionGeometry != nullptr) << "Cannot acquire shared_ptr to cutter collision geometry.";
}

void
PbdObjectCutting::apply()
{
    std::shared_ptr<PbdSystem> pbdSystem = m_cuttable->getPbdSystem();

    m_addConstraintVertices->clear();
    m_removeConstraintVertices->clear();

    // Perform cutting
    if (auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_cuttable->getGeometry()))
    {
        SurfaceMeshCut cutter;
        cutter.setInputMesh(surfMesh);
        cutter.setCutGeometry(m_objBCollisionGeometry);
        cutter.setEpsilon(m_epsilon);
        cutter.update();

        std::shared_ptr<SurfaceMesh> newMesh = cutter.getOutputMesh();

        // Only remove and add constraints related to the topological changes
        m_removeConstraintVertices = cutter.getRemoveConstraintVertices();
        m_addConstraintVertices    = cutter.getAddConstraintVertices();

        // update pbd mesh
        surfMesh->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newMesh->getInitialVertexPositions()));
        surfMesh->setVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newMesh->getVertexPositions()));
        surfMesh->setCells(std::make_shared<VecDataArray<int, 3>>(*newMesh->getCells()));
    }
    else if (auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_cuttable->getGeometry()))
    {
        LineMeshCut cutter;
        cutter.setInputMesh(lineMesh);
        cutter.setCutGeometry(m_objBCollisionGeometry);
        cutter.setEpsilon(m_epsilon);
        cutter.update();

        std::shared_ptr<LineMesh> newMesh = cutter.getOutputMesh();

        // Only remove and add constraints related to the topological changes
        m_removeConstraintVertices = cutter.getRemoveConstraintVertices();
        m_addConstraintVertices    = cutter.getAddConstraintVertices();

        // update pbd mesh
        lineMesh->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newMesh->getInitialVertexPositions()));
        lineMesh->setVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newMesh->getVertexPositions()));
        lineMesh->setCells(std::make_shared<VecDataArray<int, 2>>(*newMesh->getCells()));
    }

    // update pbd states, constraints and solver
    m_cuttable->setBodyFromGeometry();
    pbdSystem->getConstraints()->removeConstraints(m_removeConstraintVertices,
        m_cuttable->getPbdBody()->bodyHandle);
    pbdSystem->addConstraints(m_addConstraintVertices, m_cuttable->getPbdBody()->bodyHandle);

    m_cuttable->getGeometry()->postModified();
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
PbdObjectCutting::modifyTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                                  std::shared_ptr<std::vector<size_t>> modifiedTriangleIndices,
                                  std::shared_ptr<VecDataArray<int, 3>> modifiedTriangles)
{
    auto triangles = pbdMesh->getCells();
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
} // namespace imstk