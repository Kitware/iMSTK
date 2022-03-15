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
#include "imstkAnalyticalGeometry.h"
#include "imstkCDObjectFactory.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkLineMesh.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdPointPointConstraint.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
///
/// \brief Packs the info needed to add a constraint to a side
///
struct MeshSide
{
    MeshSide(VecDataArray<double, 3>& vertices, VecDataArray<double, 3>& velocities, DataArray<double>& invMasses,
             GeometryMap* mapPtr, AbstractDataArray* indicesPtr) : m_vertices(vertices), m_velocities(velocities),
        m_invMasses(invMasses), m_mapPtr(mapPtr), m_indicesPtr(indicesPtr)
    {
    }

    VecDataArray<double, 3>& m_vertices;
    VecDataArray<double, 3>& m_velocities;
    DataArray<double>& m_invMasses;
    GeometryMap* m_mapPtr = nullptr;
    AbstractDataArray* m_indicesPtr = nullptr;
};

static std::array<std::pair<int, VertexMassPair>, 2>
getEdge(const CollisionElement& elem, const MeshSide& side)
{
    int v1, v2;
    v1 = v2 = -1;
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == IMSTK_EDGE)
    {
        if (elem.m_element.m_CellIndexElement.idCount == 1)
        {
            const Vec2i& cell = (*dynamic_cast<VecDataArray<int, 2>*>(side.m_indicesPtr))[elem.m_element.m_CellIndexElement.ids[0]];
            v1 = cell[0];
            v2 = cell[1];
        }
        else if (elem.m_element.m_CellIndexElement.idCount == 2)
        {
            v1 = elem.m_element.m_CellIndexElement.ids[0];
            v2 = elem.m_element.m_CellIndexElement.ids[1];
        }
    }
    std::array<std::pair<int, VertexMassPair>, 2> results;
    if (v1 != -1)
    {
        if (side.m_mapPtr && dynamic_cast<OneToOneMap*>(side.m_mapPtr) != nullptr)
        {
            v1 = side.m_mapPtr->getMapIdx(v1);
            v2 = side.m_mapPtr->getMapIdx(v2);
        }
        results[0] = { v1, { &side.m_vertices[v1], side.m_invMasses[v1], &side.m_velocities[v1] } };
        results[1] = { v2, { &side.m_vertices[v2], side.m_invMasses[v2], &side.m_velocities[v2] } };
    }
    return results;
}

static std::array<std::pair<int, VertexMassPair>, 3>
getTriangle(const CollisionElement& elem, const MeshSide& side)
{
    int v1, v2, v3;
    v1 = v2 = v3 = -1;
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == IMSTK_TRIANGLE)
    {
        if (elem.m_element.m_CellIndexElement.idCount == 1)
        {
            const Vec3i& cell = (*dynamic_cast<VecDataArray<int, 3>*>(side.m_indicesPtr))[elem.m_element.m_CellIndexElement.ids[0]];
            v1 = cell[0];
            v2 = cell[1];
            v3 = cell[2];
        }
        else if (elem.m_element.m_CellIndexElement.idCount == 3)
        {
            v1 = elem.m_element.m_CellIndexElement.ids[0];
            v2 = elem.m_element.m_CellIndexElement.ids[1];
            v3 = elem.m_element.m_CellIndexElement.ids[2];
        }
    }
    std::array<std::pair<int, VertexMassPair>, 3> results;
    if (v1 != -1)
    {
        if (side.m_mapPtr && dynamic_cast<OneToOneMap*>(side.m_mapPtr) != nullptr)
        {
            v1 = static_cast<int>(side.m_mapPtr->getMapIdx(v1));
            v2 = static_cast<int>(side.m_mapPtr->getMapIdx(v2));
            v3 = static_cast<int>(side.m_mapPtr->getMapIdx(v3));
        }
        results[0] = { v1, { &side.m_vertices[v1], side.m_invMasses[v1], &side.m_velocities[v1] } };
        results[1] = { v2, { &side.m_vertices[v2], side.m_invMasses[v2], &side.m_velocities[v2] } };
        results[2] = { v3, { &side.m_vertices[v3], side.m_invMasses[v3], &side.m_velocities[v3] } };
    }
    return results;
}

static std::array<std::pair<int, VertexMassPair>, 4>
getTetrahedron(const CollisionElement& elem, const MeshSide& side)
{
    int v1, v2, v3, v4;
    v1 = v2 = v3 = v4 = -1;
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == IMSTK_TETRAHEDRON)
    {
        if (elem.m_element.m_CellIndexElement.idCount == 1)
        {
            const Vec4i& cell = (*dynamic_cast<VecDataArray<int, 4>*>(side.m_indicesPtr))[elem.m_element.m_CellIndexElement.ids[0]];
            v1 = cell[0];
            v2 = cell[1];
            v3 = cell[2];
            v4 = cell[3];
        }
        else if (elem.m_element.m_CellIndexElement.idCount == 4)
        {
            v1 = elem.m_element.m_CellIndexElement.ids[0];
            v2 = elem.m_element.m_CellIndexElement.ids[1];
            v3 = elem.m_element.m_CellIndexElement.ids[2];
            v4 = elem.m_element.m_CellIndexElement.ids[3];
        }
    }
    std::array<std::pair<int, VertexMassPair>, 4> results;
    if (v1 != -1)
    {
        results[0] = { v1, { &side.m_vertices[v1], side.m_invMasses[v1], &side.m_velocities[v1] } };
        results[1] = { v2, { &side.m_vertices[v2], side.m_invMasses[v2], &side.m_velocities[v2] } };
        results[2] = { v3, { &side.m_vertices[v3], side.m_invMasses[v3], &side.m_velocities[v3] } };
        results[3] = { v2, { &side.m_vertices[v4], side.m_invMasses[v4], &side.m_velocities[v4] } };
    }
    return results;
}

PbdObjectPicking::PbdObjectPicking(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2,
                                   std::string cdType) :
    SceneObject("PbdObjectPicking_" + obj1->getName() + "_vs_" + obj2->getName()),
    m_objA(obj1), m_objB(obj2), m_cdType(cdType)
{
    // We have 3 implementations for 3 methods
    //  - picking all points inside the primitive (uses CD)
    //  - picking nearest point to obj2 geometry center
    //  - picking point on element via interpolation

    m_pickingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectPicking::updatePicking, this),
        "PbdPickingUpdate", true);
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
    m_isPicking = false;
    LOG(INFO) << "End pick";
}

void
PbdObjectPicking::beginPick()
{
    m_isPicking = true;
    LOG(INFO) << "Begin pick";
}

void
PbdObjectPicking::removePickConstraints()
{
    m_constraints.clear();
    m_constraintPts.clear();
    m_pickedPtIdxOffset.clear();
}

void
PbdObjectPicking::addPickConstraints()
{
    removePickConstraints();

    CHECK(m_objA != nullptr && m_objB != nullptr)
        << "PBDPickingCH:addPickConstraints error: "
        << "no pdb object or colliding object.";

    auto pickGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(m_objB->getCollidingGeometry());
    CHECK(pickGeom != nullptr) << "Colliding geometry is analytical geometry ";
    const Vec3d pickGeomPos = pickGeom->getPosition();

    /*Vec3d min, max;
    pickGeom->computeBoundingBox(min, max);
    m_thickness = (max - min).norm() * 0.1;*/

    std::shared_ptr<PbdModel>                model         = m_objA->getPbdModel();
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr   = model->getCurrentState()->getPositions();
    VecDataArray<double, 3>&                 vertices      = *verticesPtr;
    std::shared_ptr<VecDataArray<double, 3>> velocitiesPtr = model->getCurrentState()->getVelocities();
    VecDataArray<double, 3>&                 velocities    = *velocitiesPtr;
    std::shared_ptr<DataArray<double>>       invMassesPtr  = model->getInvMasses();
    const DataArray<double>&                 invMasses     = *invMassesPtr;

    std::shared_ptr<AbstractDataArray> indicesPtr = nullptr;
    if (auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_objA->getPhysicsGeometry()))
    {
        indicesPtr = lineMesh->getLinesIndices();
    }
    if (auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_objA->getPhysicsGeometry()))
    {
        indicesPtr = surfMesh->getTriangleIndices();
    }
    else if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_objA->getPhysicsGeometry()))
    {
        indicesPtr = tetMesh->getTetrahedraIndices();
    }

    MeshSide meshStruct(
        *verticesPtr,
        *velocitiesPtr,
        *invMassesPtr,
        nullptr,
        indicesPtr.get());

    if (m_pickingMode == Mode::PickVertex)
    {
        // In PickPt mode we simply use SDF functions available in the geometries to
        // sample if in or out of the shape
        for (int i = 0; i < vertices.size(); i++)
        {
            const double signedDist = pickGeom->getFunctionValue(vertices[i]);

            // If inside the primitive
            // \todo: come back to this
            if (signedDist <= 0.0)
            {
                const Mat3d rot = pickGeom->getRotation().transpose();
                const Vec3d relativePos = rot * (vertices[i] - pickGeom->getPosition());

                m_pickedPtIdxOffset[i] = relativePos;

                m_constraintPts.push_back({
                        i,
                        pickGeom->getPosition() + rot.transpose() * relativePos,
                        Vec3d(0.0, 0.0, 0.0) });
                std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

                addConstraint(&std::get<1>(cPt), &std::get<2>(cPt),
                    &vertices[i], invMasses[i], &velocities[i]);
            }
        }
    }
    else if (m_pickingMode == Mode::PickElement)
    {
        // In PickPtInterpolated we actually perform element vs analytical geometry collision
        if (m_cdType != "" && m_colDetect == nullptr)
        {
            m_colDetect = CDObjectFactory::makeCollisionDetection(m_cdType);
        }
        m_colDetect->setInputGeometryA(m_objA->getCollidingGeometry());
        m_colDetect->setInputGeometryB(m_objB->getCollidingGeometry());
        m_colDetect->update();

        const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
        const std::vector<CollisionElement>& elementsB = m_colDetect->getCollisionData()->elementsB;

        const Mat3d rot = pickGeom->getRotation().transpose();
        for (int i = 0; i < elementsA.size(); i++)
        {
            // A is the mesh, B is the analytic geometry
            const CollisionElement& colElemA = elementsA[i];
            const CollisionElement& colElemB = elementsB[i];

            if (colElemA.m_type != CollisionElementType::CellIndex)
            {
                continue;
            }

            const CellTypeId cellTypeA = colElemA.m_element.m_CellIndexElement.cellType;

            std::vector<std::pair<int, VertexMassPair>> cellVerts;
            if (cellTypeA == IMSTK_TETRAHEDRON)
            {
                std::array<std::pair<int, VertexMassPair>, 4> vertexMass = getTetrahedron(colElemA, meshStruct);
                cellVerts.resize(4);
                cellVerts[0] = vertexMass[0];
                cellVerts[1] = vertexMass[1];
                cellVerts[2] = vertexMass[2];
                cellVerts[3] = vertexMass[3];
            }
            else if (cellTypeA == IMSTK_TRIANGLE)
            {
                std::array<std::pair<int, VertexMassPair>, 3> vertexMass = getTriangle(colElemA, meshStruct);
                cellVerts.resize(3);
                cellVerts[0] = vertexMass[0];
                cellVerts[1] = vertexMass[1];
                cellVerts[2] = vertexMass[2];
            }
            else if (cellTypeA == IMSTK_EDGE)
            {
                std::array<std::pair<int, VertexMassPair>, 2> vertexMass = getEdge(colElemA, meshStruct);
                cellVerts.resize(2);
                cellVerts[0] = vertexMass[0];
                cellVerts[1] = vertexMass[1];
            }

            // Does not resolve duplicate vertices yet
            // But pbd implicit solve with reprojection avoids issues
            for (size_t j = 0; j < cellVerts.size(); j++)
            {
                const Vec3d  relativePos = rot * (*cellVerts[j].second.vertex - pickGeom->getPosition());
                const size_t index       = m_constraintPts.size();
                const size_t vertexIndex = cellVerts[j].first;
                m_pickedPtIdxOffset[index] = relativePos;
                m_constraintPts.push_back({
                        index,
                        *cellVerts[j].second.vertex,
                        Vec3d(0.0, 0.0, 0.0) });
                std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

                addConstraint(&std::get<1>(cPt), &std::get<2>(cPt),
                    &vertices[vertexIndex], invMasses[vertexIndex], &velocities[vertexIndex]);
            }
        }
    }
    else if (m_pickingMode == Mode::PickPoint)
    {
        // In PickPtInterpolated we actually perform element vs analytical geometry collision
        if (m_cdType != "" && m_colDetect == nullptr)
        {
            m_colDetect = CDObjectFactory::makeCollisionDetection(m_cdType);
        }
        m_colDetect->setInputGeometryA(m_objA->getCollidingGeometry());
        m_colDetect->setInputGeometryB(m_objB->getCollidingGeometry());
        m_colDetect->update();

        const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
        const std::vector<CollisionElement>& elementsB = m_colDetect->getCollisionData()->elementsB;

        const Mat3d rot = pickGeom->getRotation().transpose();
        for (size_t i = 0; i < elementsA.size(); i++)
        {
            // A is the mesh, B is the analytic geometry
            const CollisionElement& colElemA = elementsA[i];
            const CollisionElement& colElemB = elementsB[i];

            if (colElemA.m_type != CollisionElementType::CellIndex)
            {
                continue;
            }

            const CellTypeId cellTypeA = colElemA.m_element.m_CellIndexElement.cellType;

            std::vector<VertexMassPair> cellVerts;
            if (cellTypeA == IMSTK_TETRAHEDRON)
            {
                std::array<std::pair<int, VertexMassPair>, 4> vertexMass = getTetrahedron(colElemA, meshStruct);
                cellVerts.resize(4);
                cellVerts[0] = vertexMass[0].second;
                cellVerts[1] = vertexMass[1].second;
                cellVerts[2] = vertexMass[2].second;
                cellVerts[3] = vertexMass[3].second;
            }
            else if (cellTypeA == IMSTK_TRIANGLE)
            {
                std::array<std::pair<int, VertexMassPair>, 3> vertexMass = getTriangle(colElemA, meshStruct);
                cellVerts.resize(3);
                cellVerts[0] = vertexMass[0].second;
                cellVerts[1] = vertexMass[1].second;
                cellVerts[2] = vertexMass[2].second;
            }
            // Edge vs ...
            else if (cellTypeA == IMSTK_EDGE)
            {
                std::array<std::pair<int, VertexMassPair>, 2> vertexMass = getEdge(colElemA, meshStruct);
                cellVerts.resize(2);
                cellVerts[0] = vertexMass[0].second;
                cellVerts[1] = vertexMass[1].second;
            }

            // The point to constrain the element too
            const Vec3d pickingPt = colElemB.m_element.m_PointDirectionElement.pt;

            std::vector<double> weights(cellVerts.size());
            if (cellTypeA == IMSTK_TETRAHEDRON)
            {
                const Vec4d baryCoord = baryCentric(pickingPt,
                    *cellVerts[0].vertex,
                    *cellVerts[1].vertex,
                    *cellVerts[2].vertex,
                    *cellVerts[3].vertex);
                weights[0] = baryCoord[0];
                weights[1] = baryCoord[1];
                weights[2] = baryCoord[2];
                weights[3] = baryCoord[3];
            }
            else if (cellTypeA == IMSTK_TRIANGLE)
            {
                const Vec3d baryCoord = baryCentric(pickingPt, *cellVerts[0].vertex, *cellVerts[1].vertex, *cellVerts[2].vertex);
                weights[0] = baryCoord[0];
                weights[1] = baryCoord[1];
                weights[2] = baryCoord[2];
            }
            else
            {
                const Vec2d baryCoord = baryCentric(pickingPt, *cellVerts[0].vertex, *cellVerts[1].vertex);
                weights[0] = baryCoord[0];
                weights[1] = baryCoord[1];
            }

            const Vec3d  relativePos = rot * (pickingPt - pickGeom->getPosition());
            const size_t index       = m_constraintPts.size();
            m_pickedPtIdxOffset[index] = relativePos;
            m_constraintPts.push_back({
                    index,
                    pickingPt,
                    Vec3d(0.0, 0.0, 0.0) });
            std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

            auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
            constraint->initConstraint(
                cellVerts,
                weights,
                { { &std::get<1>(cPt), 0.0, &std::get<2>(cPt) } },
                { 1.0 }, m_stiffness, 0.0);
            m_constraints.push_back(constraint);
        }
    }
}

void
PbdObjectPicking::addConstraint(
    Vec3d* fixedPt, Vec3d* fixedPtVel,
    Vec3d* vertex2, double invMass2, Vec3d* velocity2)
{
    auto constraint = std::make_shared<PbdPointPointConstraint>();
    constraint->initConstraint(
        { fixedPt, 0.0, fixedPtVel },
        { vertex2, invMass2, velocity2 },
        0.0, m_stiffness); // LHS is considered infinite mass
    m_constraints.push_back(constraint);
}

void
PbdObjectPicking::updatePicking()
{
    m_objA->updateGeometries();

    // If started picking
    if (!m_isPrevPicking && m_isPicking)
    {
        addPickConstraints();
    }
    // If stopped picking
    if (!m_isPicking && m_isPrevPicking)
    {
        removePickConstraints();
    }
    // Push back the picking state
    m_isPrevPicking = m_isPicking;

    if (m_isPicking)
    {
        updateConstraints();
    }
}

void
PbdObjectPicking::updateConstraints()
{
    std::shared_ptr<PbdModel> model    = m_objA->getPbdModel();
    auto                      pickGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(m_objB->getCollidingGeometry());
    CHECK(pickGeom != nullptr) << "Colliding geometry is analytical geometry ";

    const Mat3d rot = pickGeom->getRotation().transpose();

    // Update constraint point positions
    {
        for (auto& cPt : m_constraintPts)
        {
            const Vec3d offset = m_pickedPtIdxOffset[std::get<0>(cPt)];
            std::get<1>(cPt) = pickGeom->getPosition() + rot.transpose() * offset;
        }
    }

    // Directly solve here
    for (const auto& constraint : m_constraints)
    {
        constraint->solvePosition();
    }
}

void
PbdObjectPicking::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    auto pbdObj     = m_objA;
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
} // namespace imstk