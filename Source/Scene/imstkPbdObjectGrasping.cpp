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

#include "imstkPbdObjectGrasping.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCDObjectFactory.h"
#include "imstkCellPicker.h"
#include "imstkLineMesh.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPointPicker.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVertexPicker.h"

namespace imstk
{
///
/// \brief Packs the info needed to add a constraint to a side
/// by reference (this way dynamic casting & dereferencing is not
/// being done in tight loops)
///
struct MeshSide
{
    MeshSide(VecDataArray<double, 3>& vertices, VecDataArray<double, 3>& velocities, DataArray<double>& invMasses,
             AbstractDataArray* indicesPtr) : m_vertices(vertices), m_velocities(velocities),
        m_invMasses(invMasses), m_indicesPtr(indicesPtr)
    {
    }

    VecDataArray<double, 3>& m_vertices;
    VecDataArray<double, 3>& m_velocities;
    DataArray<double>& m_invMasses;
    AbstractDataArray* m_indicesPtr = nullptr;
};

template<int N>
static std::vector<std::pair<int, VertexMassPair>>
getElement(const PickData& pickData, const MeshSide& side)
{
    std::vector<std::pair<int, VertexMassPair>> results(N);
    if (pickData.idCount == 1) // If given cell index
    {
        const Eigen::Matrix<int, N, 1>& cell = (*dynamic_cast<VecDataArray<int, N>*>(side.m_indicesPtr))[pickData.ids[0]];
        for (int i = 0; i < N; i++)
        {
            const int vertexId = cell[i];
            results[i] = { vertexId, { &side.m_vertices[vertexId], side.m_invMasses[vertexId], &side.m_velocities[vertexId] } };
        }
    }
    else // If given vertex indices
    {
        for (int i = 0; i < N; i++)
        {
            const int vertexId = pickData.ids[i];
            results[i] = { vertexId, { &side.m_vertices[vertexId], side.m_invMasses[vertexId], &side.m_velocities[vertexId] } };
        }
    }
    return results;
}

PbdObjectGrasping::PbdObjectGrasping(std::shared_ptr<PbdObject> obj) :
    SceneObject("PbdObjectGrasping_" + obj->getName()),
    m_objectToGrasp(obj), m_pickMethod(std::make_shared<CellPicker>())
{
    // We have 3 implementations for 3 methods
    //  - picking all points inside the primitive (uses CD)
    //  - picking nearest point to obj2 geometry center
    //  - picking point on element via interpolation

    m_pickingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectGrasping::updatePicking, this),
        "PbdPickingUpdate", true);
    m_taskGraph->addNode(m_pickingNode);

    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getTaskGraph()->getSink());

    m_taskGraph->addNode(m_objectToGrasp->getTaskGraph()->getSource());
    m_taskGraph->addNode(m_objectToGrasp->getTaskGraph()->getSink());
}

void
PbdObjectGrasping::beginVertexGrasp(std::shared_ptr<AnalyticalGeometry> geometry)
{
    auto vertexPicker = std::make_shared<VertexPicker>();
    vertexPicker->setPickingGeometry(geometry);
    m_pickMethod = vertexPicker;
    m_graspMode  = GraspMode::Vertex;
    m_graspGeom  = geometry;

    m_isGrasping = true;
    LOG(INFO) << "Begin grasp";
}

void
PbdObjectGrasping::beginCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType)
{
    auto cellPicker = std::make_shared<CellPicker>();
    cellPicker->setPickingGeometry(geometry);
    cellPicker->setCollisionDetection(CDObjectFactory::makeCollisionDetection(cdType));
    m_pickMethod = cellPicker;
    m_graspMode  = GraspMode::Cell;
    m_graspGeom  = geometry;

    m_isGrasping = true;
    LOG(INFO) << "Begin grasp";
}

void
PbdObjectGrasping::beginRayPointGrasp(std::shared_ptr<AnalyticalGeometry> geometry,
                                      const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist)
{
    auto pointPicker = std::make_shared<PointPicker>();
    pointPicker->setPickingRay(rayStart, rayDir, maxDist);
    m_pickMethod = pointPicker;
    m_graspMode  = GraspMode::RayPoint;
    m_graspGeom  = geometry;

    m_isGrasping = true;
    LOG(INFO) << "Begin grasp";
}

void
PbdObjectGrasping::beginRayCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry,
                                     const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist)
{
    auto pointPicker = std::make_shared<PointPicker>();
    pointPicker->setPickingRay(rayStart, rayDir, maxDist);
    m_pickMethod = pointPicker;
    m_graspMode  = GraspMode::RayCell;
    m_graspGeom  = geometry;

    m_isGrasping = true;
    LOG(INFO) << "Begin grasp";
}

void
PbdObjectGrasping::endGrasp()
{
    m_isGrasping = false;
    LOG(INFO) << "End grasp";
}

void
PbdObjectGrasping::removePickConstraints()
{
    m_constraints.clear();
    m_constraintPts.clear();
}

void
PbdObjectGrasping::addPickConstraints()
{
    removePickConstraints();

    auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(m_objectToGrasp->getPhysicsGeometry());
    CHECK(pointSetToPick != nullptr) << "Trying to vertex pick with geometry that has no vertices";

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSetToPick->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;

    auto velocitiesPtr =
        std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetToPick->getVertexAttribute("Velocities"));
    CHECK(velocitiesPtr != nullptr) << "Trying to vertex pick with geometry that has no Velocities";

    VecDataArray<double, 3>& velocities   = *velocitiesPtr;
    auto                     invMassesPtr =
        std::dynamic_pointer_cast<DataArray<double>>(pointSetToPick->getVertexAttribute("InvMass"));
    CHECK(invMassesPtr != nullptr) << "Trying to vertex pick with geometry that has no InvMass";

    const DataArray<double>& invMasses = *invMassesPtr;

    std::shared_ptr<AbstractDataArray> indicesPtr = nullptr;
    if (auto lineMesh = std::dynamic_pointer_cast<LineMesh>(pointSetToPick))
    {
        indicesPtr = lineMesh->getLinesIndices();
    }
    if (auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(pointSetToPick))
    {
        indicesPtr = surfMesh->getTriangleIndices();
    }
    else if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(pointSetToPick))
    {
        indicesPtr = tetMesh->getTetrahedraIndices();
    }

    // Place all the data into a struct to pass around & for quick access without casting
    // or dereferencing
    MeshSide meshStruct(
        *verticesPtr,
        *velocitiesPtr,
        *invMassesPtr,
        indicesPtr.get());

    const Vec3d& pickGeomPos = m_graspGeom->getPosition();
    const Mat3d  pickGeomRot = m_graspGeom->getRotation().transpose();

    // Perform the picking
    const std::vector<PickData>& pickData = m_pickMethod->pick(pointSetToPick);

    // Digest the pick data based on grasp mode
    if (m_graspMode == GraspMode::Vertex)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData& data     = pickData[i];
            const int       vertexId = data.ids[0];

            const Vec3d relativePos = pickGeomRot * (vertices[vertexId] - pickGeomPos);
            m_constraintPts.push_back({
                    vertices[vertexId],
                    relativePos,
                    Vec3d::Zero() });
            std::tuple<Vec3d, Vec3d, Vec3d>& cPt = m_constraintPts.back();

            addConstraint(
                { { &vertices[vertexId], invMasses[vertexId], &velocities[vertexId] } }, { 1.0 },
                { { &std::get<0>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
                m_stiffness, 0.0);
        }
    }
    else if (m_graspMode == GraspMode::Cell || m_graspMode == GraspMode::RayCell)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData&  data = pickData[i];
            const CellTypeId pickedCellType = data.cellType;

            std::vector<std::pair<int, VertexMassPair>> cellVerts;
            if (pickedCellType == IMSTK_TETRAHEDRON)
            {
                cellVerts = getElement<4>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                cellVerts = getElement<3>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                cellVerts = getElement<2>(data, meshStruct);
            }

            // Does not resolve duplicate vertices yet
            // But pbd implicit solve with reprojection avoids issues
            for (size_t j = 0; j < cellVerts.size(); j++)
            {
                const int vertexId = cellVerts[j].first;

                const Vec3d relativePos = pickGeomRot * (vertices[vertexId] - pickGeomPos);
                m_constraintPts.push_back({
                        vertices[vertexId],
                        relativePos,
                        Vec3d::Zero() });
                std::tuple<Vec3d, Vec3d, Vec3d>& cPt = m_constraintPts.back();

                addConstraint(
                    { { &vertices[vertexId], invMasses[vertexId], &velocities[vertexId] } }, { 1.0 },
                    { { &std::get<0>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
                    m_stiffness, 0.0);
            }
        }
    }
    else if (m_graspMode == GraspMode::RayPoint)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData&  data = pickData[i];
            const CellTypeId pickedCellType = data.cellType;

            std::vector<std::pair<int, VertexMassPair>> cellIdVerts;
            if (pickedCellType == IMSTK_TETRAHEDRON)
            {
                cellIdVerts = getElement<4>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                cellIdVerts = getElement<3>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                cellIdVerts = getElement<2>(data, meshStruct);
            }
            std::vector<VertexMassPair> cellVerts(cellIdVerts.size());
            for (size_t j = 0; j < cellIdVerts.size(); j++)
            {
                cellVerts[j] = cellIdVerts[j].second;
            }

            // The point to constrain the element too
            const Vec3d pickingPt = data.pickPoint;

            std::vector<double> weights(cellVerts.size());
            if (pickedCellType == IMSTK_TETRAHEDRON)
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
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                const Vec3d baryCoord = baryCentric(pickingPt, *cellVerts[0].vertex, *cellVerts[1].vertex, *cellVerts[2].vertex);
                weights[0] = baryCoord[0];
                weights[1] = baryCoord[1];
                weights[2] = baryCoord[2];
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                const Vec2d baryCoord = baryCentric(pickingPt, *cellVerts[0].vertex, *cellVerts[1].vertex);
                weights[0] = baryCoord[0];
                weights[1] = baryCoord[1];
            }

            const Vec3d  relativePos = pickGeomRot * (pickingPt - pickGeomPos);
            m_constraintPts.push_back({
                        pickingPt,
                        relativePos,
                        Vec3d::Zero() });
            std::tuple<Vec3d, Vec3d, Vec3d>& cPt = m_constraintPts.back();

            // Cell to single point constraint
            addConstraint(
                cellVerts, weights,
                { { &std::get<0>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
                m_stiffness, 0.0);
        }
    }
}

void
PbdObjectGrasping::addConstraint(
    std::vector<VertexMassPair> graspPtsA,
    std::vector<double> graspWeightsA,
    std::vector<VertexMassPair> meshPtsB,
    std::vector<double> meshWeightsB,
    double stiffnessA, double stiffnessB)
{
    auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
    constraint->initConstraint(
        graspPtsA, graspWeightsA,
        meshPtsB, meshWeightsB,
        stiffnessA, stiffnessB);
    m_constraints.push_back(constraint);
}

void
PbdObjectGrasping::updatePicking()
{
    m_objectToGrasp->updateGeometries();

    // If started picking
    if (!m_isPrevGrasping && m_isGrasping)
    {
        addPickConstraints();
    }
    // If stopped picking
    if (!m_isGrasping && m_isPrevGrasping)
    {
        removePickConstraints();
    }
    // Push back the picking state
    m_isPrevGrasping = m_isGrasping;

    if (m_isGrasping)
    {
        updateConstraints();
    }
}

void
PbdObjectGrasping::updateConstraints()
{
    // Update constraint point positions
    {
        const Vec3d& pos = m_graspGeom->getPosition();
        const Mat3d  rot = m_graspGeom->getRotation();
        for (auto& cPt : m_constraintPts)
        {
            const Vec3d relativePos = std::get<1>(cPt);
            std::get<0>(cPt) = pos + rot * relativePos;
        }
    }

    // Directly solve here
    for (const auto& constraint : m_constraints)
    {
        constraint->solvePosition();
    }
}

void
PbdObjectGrasping::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    std::shared_ptr<PbdModel> pbdModel = m_objectToGrasp->getPbdModel();

    m_taskGraph->addEdge(source, m_objectToGrasp->getTaskGraph()->getSource());
    m_taskGraph->addEdge(m_objectToGrasp->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pbdModel->getTaskGraph()->getSink());
}
} // namespace imstk