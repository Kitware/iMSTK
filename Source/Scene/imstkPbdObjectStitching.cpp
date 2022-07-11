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
#include "imstkCellPicker.h"
#include "imstkLineMesh.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPointPicker.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTriangleToTetMap.h"

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
             AbstractDataArray* indicesPtrt, PointwiseMap* mapt) : vertices(verticest), velocities(velocitiest),
        invMasses(invMassest), indicesPtr(indicesPtrt), map(mapt)
    {
    }

    VecDataArray<double, 3>& vertices;
    VecDataArray<double, 3>& velocities;
    DataArray<double>& invMasses;
    AbstractDataArray* indicesPtr = nullptr;
    PointwiseMap* map = nullptr;
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
                vertexId = static_cast<int>(side.map->getParentVertexId(vertexId));
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
                vertexId = static_cast<int>(side.map->getParentVertexId(vertexId));
            }
            results[i] = { vertexId, { &side.vertices[vertexId], side.invMasses[vertexId], &side.velocities[vertexId] } };
        }
    }
    return results;
}

static std::vector<double>
getWeights(const std::vector<VertexMassPair>& cellVerts, const Vec3d& pt)
{
    std::vector<double> weights(cellVerts.size());
    if (cellVerts.size() == 4)
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
    else if (cellVerts.size() == 3)
    {
        const Vec3d baryCoord = baryCentric(pt,
            *cellVerts[0].vertex, *cellVerts[1].vertex, *cellVerts[2].vertex);
        weights[0] = baryCoord[0];
        weights[1] = baryCoord[1];
        weights[2] = baryCoord[2];
    }
    else if (cellVerts.size() == 2)
    {
        const Vec2d baryCoord = baryCentric(pt, *cellVerts[0].vertex, *cellVerts[1].vertex);
        weights[0] = baryCoord[0];
        weights[1] = baryCoord[1];
    }
    else if (cellVerts.size() == 1)
    {
        weights[0] = 1.0;
    }
    return weights;
};

PbdObjectStitching::PbdObjectStitching(std::shared_ptr<PbdObject> obj) :
    SceneObject("PbdObjectStitching_" + obj->getName()),
    m_objectToStitch(obj), m_pickMethod(std::make_shared<CellPicker>())
{
    m_stitchingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectStitching::updateStitching, this),
        "PbdStitchingUpdate", true);
    m_taskGraph->addNode(m_stitchingNode);

    m_taskGraph->addNode(m_objectToStitch->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(m_objectToStitch->getPbdModel()->getUpdateVelocityNode());

    m_taskGraph->addNode(m_objectToStitch->getTaskGraph()->getSource());
    m_taskGraph->addNode(m_objectToStitch->getTaskGraph()->getSink());
}

void
PbdObjectStitching::beginStitch(const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist)
{
    auto pointPicker = std::make_shared<PointPicker>();
    pointPicker->setPickingRay(rayStart, rayDir, maxDist);
    m_pickMethod = pointPicker;

    m_performStitch = true;
    LOG(INFO) << "Begin stitch";
}

void
PbdObjectStitching::removeStitchConstraints()
{
    m_constraints.clear();
}

void
PbdObjectStitching::addStitchConstraints()
{
    // PbdModel geometry can only be PointSet
    std::shared_ptr<PointSet> pbdPhysicsGeom =
        std::dynamic_pointer_cast<PointSet>(m_objectToStitch->getPhysicsGeometry());

    // If the geometry to pick hasn't been set yet, default it to the physics geometry
    // Could be different in cases where user wants to pick a mapped geometry, mapping back
    // to the physics geometry
    std::shared_ptr<PointSet> pointSetToPick = std::dynamic_pointer_cast<PointSet>(m_geomToStitch);
    if (m_geomToStitch == nullptr)
    {
        pointSetToPick = pbdPhysicsGeom;
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pbdPhysicsGeom->getVertexPositions();

    // Get the attributes from the physics geometry
    auto velocitiesPtr =
        std::dynamic_pointer_cast<VecDataArray<double, 3>>(pbdPhysicsGeom->getVertexAttribute("Velocities"));
    CHECK(velocitiesPtr != nullptr) << "Trying to vertex pick with geometry that has no Velocities";

    auto invMassesPtr =
        std::dynamic_pointer_cast<DataArray<double>>(pbdPhysicsGeom->getVertexAttribute("InvMass"));
    CHECK(invMassesPtr != nullptr) << "Trying to vertex pick with geometry that has no InvMass";

    std::shared_ptr<AbstractDataArray> indicesPtr = nullptr;
    if (auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(pointSetToPick))
    {
        indicesPtr = cellMesh->getIndices();
    }

    // If the user tries to pick
    PointwiseMap* map = nullptr;
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

    auto getCellVerts =
        [&](const PickData& data)
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

    auto pointPicker = std::dynamic_pointer_cast<PointPicker>(m_pickMethod);
    pointPicker->setUseFirstHit(false);

    // Perform the picking only on surface data
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(pointSetToPick);
    auto tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(pointSetToPick);
    if (tetMesh != nullptr)
    {
        surfMesh = tetMesh->extractSurfaceMesh();
    }
    const std::vector<PickData>& pickData = m_pickMethod->pick(surfMesh);

    // Must have at least 2
    if (pickData.size() < 1)
    {
        return;
    }

    std::vector<std::pair<PickData, PickData>> constraintPair;
    if (std::dynamic_pointer_cast<TetrahedralMesh>(pointSetToPick) != nullptr)
    {
        // ** Warning **, surface triangles are not 100% garunteed to tell inside/out
        // Should use angle-weighted pseudonormals
        surfMesh->computeTrianglesNormals();
        std::shared_ptr<VecDataArray<double, 3>> faceNormalsPtr = surfMesh->getCellNormals();
        const VecDataArray<double, 3>&           faceNormals    = *faceNormalsPtr;

        // Find all neighbor pairs with normals facing each other
        for (size_t i = 0, j = 1; i < pickData.size() - 1; i++, j++)
        {
            const Vec3d& pt_i     = pickData[i].pickPoint;
            const Vec3d& pt_j     = pickData[j].pickPoint;
            const Vec3d& normal_i = faceNormals[pickData[i].ids[0]];
            const Vec3d& normal_j = faceNormals[pickData[j].ids[0]];
            const Vec3d  diff     = pt_j - pt_i;

            //bool faceOpposite = (normal_i.dot(normal_j) < 0.0);
            bool faceInwards = (diff.dot(normal_i) > 0.0) && (diff.dot(normal_j) < 0.0);

            // If they face into each other
            if (faceInwards)
            {
                constraintPair.push_back({ pickData[i], pickData[j] });
            }
        }

        // If no constraint pairs, no stitches can be placed
        if (constraintPair.size() == 0)
        {
            return;
        }

        // If we have a tet mesh and some results, map the picked surface triangles
        // back to the tetrahedrons
        TriangleToTetMap mapper;
        mapper.setParentGeometry(tetMesh);
        mapper.setChildGeometry(surfMesh);
        mapper.compute();

        for (size_t i = 0; i < constraintPair.size(); i++)
        {
            PickData& pickData1 = constraintPair[i].first;
            PickData& pickData2 = constraintPair[i].second;

            // Get the tet id from the triangle id
            pickData1.ids[0]   = mapper.getParentTetId(pickData1.ids[0]);
            pickData1.idCount  = 1;
            pickData1.cellType = IMSTK_TETRAHEDRON;

            pickData2.ids[0]   = mapper.getParentTetId(pickData2.ids[0]);
            pickData2.idCount  = 1;
            pickData2.cellType = IMSTK_TETRAHEDRON;
            // Leave pick point the same
        }
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(pointSetToPick) != nullptr)
    {
        // For a SurfaceMesh just constrain every pair
        for (size_t i = 0, j = 1; i < pickData.size() - 1; i++, j++)
        {
            constraintPair.push_back({ pickData[i], pickData[j] });
        }
    }

    // Constrain only the pick points between the two elements
    for (size_t i = 0; i < constraintPair.size(); i++)
    {
        const PickData& pickData1 = constraintPair[i].first;
        const PickData& pickData2 = constraintPair[i].second;

        if (m_maxStitchDist == -1.0 || (pickData2.pickPoint - pickData1.pickPoint).norm() < m_maxStitchDist)
        {
            std::vector<VertexMassPair> cellVerts1 = getCellVerts(pickData1);
            std::vector<double>         weights1   = getWeights(cellVerts1, pickData1.pickPoint);
            std::vector<VertexMassPair> cellVerts2 = getCellVerts(pickData2);
            std::vector<double>         weights2   = getWeights(cellVerts2, pickData2.pickPoint);

            // Cell to single point constraint
            addConstraint(
                cellVerts1, weights1,
                cellVerts2, weights2,
                m_stiffness, m_stiffness);
        }
    }
}

void
PbdObjectStitching::addConstraint(
    const std::vector<VertexMassPair>& ptsA,
    const std::vector<double>& weightsA,
    const std::vector<VertexMassPair>& ptsB,
    const std::vector<double>& weightsB,
    const double stiffnessA, const double stiffnessB)
{
    auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
    constraint->initConstraint(
        ptsA, weightsA,
        ptsB, weightsB,
        stiffnessA, stiffnessB);
    m_constraints.push_back(constraint);
}

void
PbdObjectStitching::updateStitching()
{
    m_objectToStitch->updateGeometries();

    // If started
    if (m_performStitch)
    {
        addStitchConstraints();
        m_performStitch = false;
    }

    updateConstraints();
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

    // Ensure a complete graph
    m_taskGraph->addEdge(source, m_objectToStitch->getTaskGraph()->getSource());
    m_taskGraph->addEdge(m_objectToStitch->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_stitchingNode);
    m_taskGraph->addEdge(m_stitchingNode, pbdModel->getUpdateVelocityNode());
}
} // namespace imstk