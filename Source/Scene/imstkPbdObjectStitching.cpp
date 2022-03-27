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

#include "imstkPbdObjectStitching.h"
#include "imstkCDObjectFactory.h"
#include "imstkCellPicker.h"
#include "imstkLineMesh.h"
#include "imstkOneToOneMap.h"
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
    MeshSide(VecDataArray<double, 3>& verticest, VecDataArray<double, 3>& velocitiest, DataArray<double>& invMassest,
        AbstractDataArray* indicesPtrt, OneToOneMap* mapt) : vertices(verticest), velocities(velocitiest),
        invMasses(invMassest), indicesPtr(indicesPtrt), map(mapt)
    {
    }

    VecDataArray<double, 3>& vertices;
    VecDataArray<double, 3>& velocities;
    DataArray<double>& invMasses;
    AbstractDataArray* indicesPtr = nullptr;
    OneToOneMap* map = nullptr;
};

template<int N>
static std::vector<std::pair<int, VertexMassPair>>
getElement(const PickData& pickData, const MeshSide& side)
{
    std::vector<std::pair<int, VertexMassPair>> results(N);
    if (pickData.idCount == 1 && pickData.cellType != IMSTK_VERTEX) // If given cell index
    {
        const Eigen::Matrix<int, N, 1>& cell = (*dynamic_cast<VecDataArray<int, N>*>(side.indicesPtr))[pickData.ids[0]];
        for (int i = 0; i < N; i++)
        {
            int vertexId = cell[i];
            if (side.map != nullptr)
            {
                vertexId = static_cast<int>(side.map->getMapIdx(vertexId));
            }
            results[i] = { vertexId, { &side.vertices[vertexId], side.invMasses[vertexId], &side.velocities[vertexId] } };
        }
    }
    else // If given vertex indices
    {
        for (int i = 0; i < N; i++)
        {
            int vertexId = pickData.ids[i];
            if (side.map != nullptr)
            {
                vertexId = static_cast<int>(side.map->getMapIdx(vertexId));
            }
            results[i] = { vertexId, { &side.vertices[vertexId], side.invMasses[vertexId], &side.velocities[vertexId] } };
        }
    }
    return results;
}

PbdObjectStitching::PbdObjectStitching(std::shared_ptr<PbdObject> obj) :
    SceneObject("PbdObjectStitching_" + obj->getName()),
    m_objectToStitch(obj), m_pickMethod(std::make_shared<CellPicker>())
{
    m_stitchingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectStitching::updatePicking, this),
        "PbdStitchingUpdate", true);
    m_taskGraph->addNode(m_stitchingNode);

    m_taskGraph->addNode(m_objectToStitch->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(m_objectToStitch->getPbdModel()->getUpdateVelocityNode());

    m_taskGraph->addNode(m_objectToStitch->getTaskGraph()->getSource());
    m_taskGraph->addNode(m_objectToStitch->getTaskGraph()->getSink());
}

void
PbdObjectStitching::beginRayPointStitch(const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist)
{
    auto pointPicker = std::make_shared<PointPicker>();
    pointPicker->setPickingRay(rayStart, rayDir, maxDist);
    m_pickMethod = pointPicker;
    m_mode = StitchMode::RayPoint;

    m_isStitching = true;
    LOG(INFO) << "Begin stitch";
}

void
PbdObjectStitching::endStitch()
{
    m_isStitching = false;
    LOG(INFO) << "End stitch";
}

void
PbdObjectStitching::removeStitchConstraints()
{
    m_constraints.clear();
}

void
PbdObjectStitching::addStitchConstraints()
{
    std::shared_ptr<PointSet> pbdPhysicsGeom =
        std::dynamic_pointer_cast<PointSet>(m_objectToStitch->getPhysicsGeometry());

    // If the point set to pick hasn't been set yet, default it to the physics geometry
    // This would be the case if the user was mapping a geometry to another
    std::shared_ptr<PointSet> pointSetToPick = std::dynamic_pointer_cast<PointSet>(m_geomToStitch);
    if (m_geomToStitch == nullptr)
    {
        pointSetToPick = pbdPhysicsGeom;
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pbdPhysicsGeom->getVertexPositions();
    //VecDataArray<double, 3>& vertices = *verticesPtr;

    // Get the attributes from the physics geometry
    auto velocitiesPtr =
        std::dynamic_pointer_cast<VecDataArray<double, 3>>(pbdPhysicsGeom->getVertexAttribute("Velocities"));
    CHECK(velocitiesPtr != nullptr) << "Trying to vertex pick with geometry that has no Velocities";
    //VecDataArray<double, 3>& velocities = *velocitiesPtr;

    auto invMassesPtr =
        std::dynamic_pointer_cast<DataArray<double>>(pbdPhysicsGeom->getVertexAttribute("InvMass"));
    CHECK(invMassesPtr != nullptr) << "Trying to vertex pick with geometry that has no InvMass";
    //const DataArray<double>& invMasses = *invMassesPtr;

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

    // If the user tries to pick
    OneToOneMap* map = nullptr;
    if (m_geometryToStitchMap != nullptr)
    {
        map = m_geometryToStitchMap.get();
    }

    // Place all the data into a struct to pass around & for quick access without casting
    // or dereferencing
    MeshSide meshStruct(
        *verticesPtr,
        *velocitiesPtr,
        *invMassesPtr,
        indicesPtr.get(),
        map);

    // Digest the pick data based on grasp mode
    if (m_mode == StitchMode::Vertex)
    {
        /*for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData& data = pickData[i];
            int             vertexId = data.ids[0];
            if (meshStruct.map != nullptr)
            {
                vertexId = static_cast<int>(meshStruct.map->getMapIdx(vertexId));
            }

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
        }*/
    }
    else if (m_mode == StitchMode::Cell || m_mode == StitchMode::RayCell)
    {
        //for (size_t i = 0; i < pickData.size(); i++)
        //{
        //    const PickData& data = pickData[i];
        //    const CellTypeId pickedCellType = data.cellType;

        //    std::vector<std::pair<int, VertexMassPair>> cellVerts;
        //    if (pickedCellType == IMSTK_TETRAHEDRON)
        //    {
        //        cellVerts = getElement<4>(data, meshStruct);
        //    }
        //    else if (pickedCellType == IMSTK_TRIANGLE)
        //    {
        //        cellVerts = getElement<3>(data, meshStruct);
        //    }
        //    else if (pickedCellType == IMSTK_EDGE)
        //    {
        //        cellVerts = getElement<2>(data, meshStruct);
        //    }
        //    else if (pickedCellType == IMSTK_VERTEX)
        //    {
        //        cellVerts = getElement<1>(data, meshStruct);
        //    }

        //    // Does not resolve duplicate vertices yet
        //    // But pbd implicit solve with reprojection avoids issues
        //    for (size_t j = 0; j < cellVerts.size(); j++)
        //    {
        //        const int vertexId = cellVerts[j].first;

        //        const Vec3d relativePos = pickGeomRot * (vertices[vertexId] - pickGeomPos);
        //        m_constraintPts.push_back({
        //                vertices[vertexId],
        //                relativePos,
        //                Vec3d::Zero() });
        //        std::tuple<Vec3d, Vec3d, Vec3d>& cPt = m_constraintPts.back();

        //        addConstraint(
        //            { { &vertices[vertexId], invMasses[vertexId], &velocities[vertexId] } }, { 1.0 },
        //            { { &std::get<0>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
        //            m_stiffness, 0.0);
        //    }
        //}
    }
    else if (m_mode == StitchMode::RayPoint)
    {
        auto pointPicker = std::dynamic_pointer_cast<PointPicker>(m_pickMethod);
        //const Vec3d& rayStart = pointPicker->getPickRayStart();
        //const Vec3d& rayDir = pointPicker->getPickRayDir();

        // Simple heuristic used here. Stitch all points together along the ray with normals
        // facing each other. Any normal pointing in/out can be assumed going inside/outside the mesh

        // Other possibility: If given a manfiold mesh get all the intersection points {1,2,3,4}.
        // Assuming we start outside the shape, stitch up 2 & 3. Any points beyond this we ignore
        // If given a non-manifold mesh we stitch up all points found along the ray
        
        // Perform the picking only on surface data
        std::shared_ptr<SurfaceMesh> surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(pointSetToPick);
        if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(pointSetToPick))
        {
            surfMesh = tetMesh->extractSurfaceMesh();
        }
        surfMesh->computeTrianglesNormals();

        const std::vector<PickData>& pickData = m_pickMethod->pick(surfMesh);

        // ** Warning **, surface triangles are not 100% garunteed to tell inside/out
        // Use angle-weighted psuedonormals as done in MeshToMeshBruteForceCD
        std::shared_ptr<VecDataArray<double, 3>> faceNormalsPtr = surfMesh->getCellNormals();
        const VecDataArray<double, 3>& faceNormals = *faceNormalsPtr;

        // Find all neighbor pairs with normals facing each other
        std::vector<std::pair<PickData, PickData>> constraintPair;
        for (size_t i = 0, j = 1; i < pickData.size() - 1; i++, j++)
        {
            const Vec3d& normal_i = faceNormals[pickData[i].ids[0]];
            const Vec3d& normal_j = faceNormals[pickData[j].ids[0]];

            // If they face in to each other
            if (normal_i.dot(normal_j) < 0.0)
            {
                constraintPair.push_back({ pickData[i], pickData[j] });
            }
        }

        // Now add constraints based on these
        // If a SurfaceMesh just directly add the element
        if (std::dynamic_pointer_cast<SurfaceMesh>(pointSetToPick) != nullptr)
        {
            auto getCellVerts = [&](const PickData& data)
            {
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
                return cellVerts;
            };

            auto getWeights = [](const std::vector<VertexMassPair>& cellVerts, const Vec3d& pt)
            {
                std::vector<double> weights(cellVerts.size());
                if (cellVerts.size() == IMSTK_TETRAHEDRON)
                {
                    const Vec4d baryCoord = baryCentric(pt,
                        *cellVerts[0].vertex,
                        *cellVerts[1].vertex,
                        *cellVerts[2].vertex,
                        *cellVerts[3].vertex);
                    weights[0] = baryCoord[0];
                    weights[1] = baryCoord[1];
                    weights[2] = baryCoord[2];
                    weights[3] = baryCoord[3];
                }
                else if (cellVerts.size() == IMSTK_TRIANGLE)
                {
                    const Vec3d baryCoord = baryCentric(pt,
                        *cellVerts[0].vertex, *cellVerts[1].vertex, *cellVerts[2].vertex);
                    weights[0] = baryCoord[0];
                    weights[1] = baryCoord[1];
                    weights[2] = baryCoord[2];
                }
                else if (cellVerts.size() == IMSTK_EDGE)
                {
                    const Vec2d baryCoord = baryCentric(pt, *cellVerts[0].vertex, *cellVerts[1].vertex);
                    weights[0] = baryCoord[0];
                    weights[1] = baryCoord[1];
                }
                else if (cellVerts.size() == IMSTK_VERTEX)
                {
                    weights[0] = 1.0;
                }
                return weights;
            };

            for (size_t i = 0; i < constraintPair.size(); i++)
            {
                const PickData& pickData1 = constraintPair[i].first;
                const PickData& pickData2 = constraintPair[i].second;

                std::vector<VertexMassPair> cellVerts1 = getCellVerts(pickData1);
                std::vector<double> weights1 = getWeights(cellVerts1, pickData1.pickPoint);
                std::vector<VertexMassPair> cellVerts2 = getCellVerts(pickData2);
                std::vector<double> weights2 = getWeights(cellVerts2, pickData2.pickPoint);

                // Cell to single point constraint
                addConstraint(
                    cellVerts1, weights1,
                    cellVerts2, weights2,
                    m_stiffness, m_stiffness);
            }
        }
        // If a TetrahedralMesh we need to map our triangle/surface element
        //  back to our tetrahedral one
        else if (std::dynamic_pointer_cast<TetrahedralMesh>(pointSetToPick) != nullptr)
        {

        }
    }
}

void
PbdObjectStitching::addConstraint(
    std::vector<VertexMassPair> ptsA,
    std::vector<double> weightsA,
    std::vector<VertexMassPair> ptsB,
    std::vector<double> weightsB,
    double stiffnessA, double stiffnessB)
{
    auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
    constraint->initConstraint(
        ptsA, weightsA,
        ptsB, weightsB,
        stiffnessA, stiffnessB);
    m_constraints.push_back(constraint);
}

void
PbdObjectStitching::updatePicking()
{
    m_objectToStitch->updateGeometries();

    // If started
    if (!m_isPrevStitching && m_isStitching)
    {
        addStitchConstraints();
    }
    // If stopped
    if (!m_isStitching && m_isPrevStitching)
    {
        removeStitchConstraints();
    }
    // Push back the state
    m_isPrevStitching = m_isStitching;

    if (m_isStitching)
    {
        updateConstraints();
    }
}

void
PbdObjectStitching::updateConstraints()
{
    // Directly solve here
    for (const auto& constraint : m_constraints)
    {
        constraint->solvePosition();
    }
}

void
PbdObjectStitching::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    std::shared_ptr<PbdModel> pbdModel = m_objectToStitch->getPbdModel();

    m_taskGraph->addEdge(source, m_objectToStitch->getTaskGraph()->getSource());
    m_taskGraph->addEdge(m_objectToStitch->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_stitchingNode);
    m_taskGraph->addEdge(m_stitchingNode, pbdModel->getUpdateVelocityNode());
}
} // namespace imstk