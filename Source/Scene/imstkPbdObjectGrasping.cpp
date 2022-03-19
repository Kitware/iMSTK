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
/////
///// \brief PickData covers all picking datas
///// via composition.
///// 
//struct PickData
//{
//public:
//    Vec3d pickPoint;
//    int cellId;
//    std::vector<int> vertexIds;
//    std::vector<double> weights;
//};
//
/////
///// \brief Abstract functor for picking of shapes.
///// \note: A factory mapping shape types to picking types is
///// a strong possibility.
///// 
//struct PickingMethod
//{
//public:
//    ///
//    /// \brief Perform picking on provided geometry
//    /// 
//    virtual std::vector<PickData> Pick(std::shared_ptr<Geometry> geomToPick) = 0;
//};
//
/////
///// \brief Picks elements of geomToPick via those that that are
///// intersecting pickingGeom.
///// 
//struct ElementPicking : public PickingMethod
//{
//public:
//    std::vector<PickData> Pick(std::shared_ptr<Geometry> geometryToPick) override
//    {
//        // Crazy combinatoral explosion
//    }
//    void setPickingGeometry(std::shared_ptr<Geometry> pickGeometry)
//    {
//        m_pickGeometry = pickGeometry;
//    }
//    std::shared_ptr<Geometry> getPickGeometry() const { return m_pickGeometry; }
//
//protected:
//    std::shared_ptr<Geometry> m_pickGeometry = nullptr;
//};
//
/////
///// \brief Picks vertices of geomToPick via those that that are
///// intersecting pickingGeom.
///// 
//struct VertexPicking : public PickingMethod
//{
//public:
//    std::vector<PickData> Pick(std::shared_ptr<Geometry> geometryToPick) override
//    {
//        std::vector<PickData> pickDatas;
//
//        auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(geometryToPick);
//        CHECK(pointSetToPick != nullptr) << "Trying to vertex pick with geometry that has no vertices";
//
//        // Use implicit functions available in the geometries to sample if in or out of the shape
//        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSetToPick->getVertexPositions();
//        VecDataArray<double, 3>& vertices = *verticesPtr;
//        for (int i = 0; i < vertices.size(); i++)
//        {
//            const double signedDist = m_pickGeometry->getFunctionValue(vertices[i]);
//
//            // If inside the primitive
//            // \todo: come back to this
//            if (signedDist <= 0.0)
//            {
//                const Mat3d rot = m_pickGeometry->getRotation().transpose();
//                const Vec3d relativePos = rot * (vertices[i] - m_pickGeometry->getPosition());
//
//                m_pickedPtIdxOffset[i] = relativePos;
//
//                m_constraintPts.push_back({
//                        i,
//                        m_pickingGeometry->getPosition() + rot.transpose() * relativePos,
//                        Vec3d(0.0, 0.0, 0.0) });
//                std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();
//
//                addConstraint(
//                    { { &vertices[i], invMasses[i], &velocities[i] } }, { 1.0 },
//                    { { &std::get<1>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
//                    m_stiffness, 0.0);
//            }
//        }
//        return pickData;
//    }
//
//    void setPickingGeometry(std::shared_ptr<AnalyticalGeometry> pickGeometry) { m_pickGeometry = pickGeometry; }
//    std::shared_ptr<AnalyticalGeometry> getPickGeometry() const { return m_pickGeometry; }
//
//protected:
//    std::shared_ptr<AnalyticalGeometry> m_pickGeometry = nullptr;
//};
//
/////
///// \brief Picks points on elements of geomToPick via those that that are
///// intersecting the provided ray.
///// 
//struct PointPicking : public PickingMethod
//{
//public:
//    std::vector<PickData> Pick(std::shared_ptr<Geometry> geometryToPick) override
//    {
//        // Crazy combinatoral explosion
//    }
//    void setPickingRay(const Vec3d& rayStart, const Vec3d& rayDir)
//    {
//        m_rayStart = rayStart;
//        m_rayDir = rayDir;
//    }
//    const Vec3d& getPickRayStart() const { return m_rayStart; }
//    const Vec3d& getPickRayDir() const { return m_rayDir; }
//
//protected:
//    Vec3d m_rayStart = Vec3d::Zero();
//    Vec3d m_rayDir = Vec3d::Zero();
//};


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

PbdObjectGrasping::PbdObjectGrasping(std::shared_ptr<PbdObject> obj1) :
    SceneObject("PbdObjectPicking_" + obj1->getName()),
    m_objA(obj1)
{
    // We have 3 implementations for 3 methods
    //  - picking all points inside the primitive (uses CD)
    //  - picking nearest point to obj2 geometry center
    //  - picking point on element via interpolation

    m_pickingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectGrasping::updatePicking, this),
        "PbdPickingUpdate", true);
    m_taskGraph->addNode(m_pickingNode);

    m_taskGraph->addNode(obj1->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(obj1->getPbdModel()->getTaskGraph()->getSink());

    m_taskGraph->addNode(obj1->getTaskGraph()->getSource());
    m_taskGraph->addNode(obj1->getTaskGraph()->getSink());
}

void
PbdObjectGrasping::endPick()
{
    m_isPicking = false;
    LOG(INFO) << "End pick";
}

void
PbdObjectGrasping::beginVertexPick(std::shared_ptr<AnalyticalGeometry> geometry)
{
    m_pickingMode = Mode::PickVertex;
    m_pickingGeometry = geometry;

    m_isPicking = true;
    LOG(INFO) << "Begin pick";
}
void
PbdObjectGrasping::beginElementPick(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType)
{
    m_pickingMode = Mode::PickElement;
    m_pickingGeometry = geometry;
    m_cdType = cdType;

    m_isPicking = true;
    LOG(INFO) << "Begin pick";
}
void
PbdObjectGrasping::beginRayPick(Vec3d rayStart, Vec3d rayDir)
{
    m_pickingMode = Mode::PickRay;
    m_rayStart = rayStart;
    m_rayDir = rayDir;

    m_isPicking = true;
    LOG(INFO) << "Begin pick";
}
void
PbdObjectGrasping::beginRayElementPick(Vec3d rayStart, Vec3d rayDir)
{
    m_pickingMode = Mode::PickRayElement;
    m_rayStart = rayStart;
    m_rayDir = rayDir;

    m_isPicking = true;
    LOG(INFO) << "Begin pick";
}

void
PbdObjectGrasping::removePickConstraints()
{
    m_constraints.clear();
    m_constraintPts.clear();
    m_pickedPtIdxOffset.clear();
}

void
PbdObjectGrasping::addPickConstraints()
{
    removePickConstraints();

    const Vec3d pickGeomPos = m_pickingGeometry->getPosition();

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
            const double signedDist = m_pickingGeometry->getFunctionValue(vertices[i]);

            // If inside the primitive
            // \todo: come back to this
            if (signedDist <= 0.0)
            {
                const Mat3d rot = m_pickingGeometry->getRotation().transpose();
                const Vec3d relativePos = rot * (vertices[i] - m_pickingGeometry->getPosition());

                m_pickedPtIdxOffset[i] = relativePos;

                m_constraintPts.push_back({
                        i,
                        m_pickingGeometry->getPosition() + rot.transpose() * relativePos,
                        Vec3d(0.0, 0.0, 0.0) });
                std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

                addConstraint(
                    { { &vertices[i], invMasses[i], &velocities[i] } }, { 1.0 },
                    { { &std::get<1>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
                    m_stiffness, 0.0);
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
        m_colDetect->setInputGeometryB(m_pickingGeometry);
        m_colDetect->update();

        const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
        const std::vector<CollisionElement>& elementsB = m_colDetect->getCollisionData()->elementsB;

        const Mat3d rot = m_pickingGeometry->getRotation().transpose();
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
                const Vec3d  relativePos = rot * (*cellVerts[j].second.vertex - m_pickingGeometry->getPosition());
                const size_t index       = m_constraintPts.size();
                const size_t vertexIndex = cellVerts[j].first;
                m_pickedPtIdxOffset[index] = relativePos;
                m_constraintPts.push_back({
                        index,
                        *cellVerts[j].second.vertex,
                        Vec3d(0.0, 0.0, 0.0) });
                std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

                addConstraint(
                    { { &vertices[vertexIndex], invMasses[vertexIndex], &velocities[vertexIndex] } }, { 1.0 },
                    { { &std::get<1>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
                    m_stiffness, 0.0);
            }
        }
    }
    else if (m_pickingMode == Mode::PickRay)
    {
        // In PickPtInterpolated we actually perform element vs analytical geometry collision
        if (m_cdType != "" && m_colDetect == nullptr)
        {
            m_colDetect = CDObjectFactory::makeCollisionDetection(m_cdType);
        }
        m_colDetect->setInputGeometryA(m_objA->getCollidingGeometry());
        m_colDetect->setInputGeometryB(m_pickingGeometry);
        m_colDetect->update();

        const std::vector<CollisionElement>& elementsA = m_colDetect->getCollisionData()->elementsA;
        const std::vector<CollisionElement>& elementsB = m_colDetect->getCollisionData()->elementsB;

        const Mat3d rot = m_pickingGeometry->getRotation().transpose();
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

            const Vec3d  relativePos = rot * (pickingPt - m_pickingGeometry->getPosition());
            const size_t index       = m_constraintPts.size();
            m_pickedPtIdxOffset[index] = relativePos;
            m_constraintPts.push_back({
                    index,
                    pickingPt,
                    Vec3d(0.0, 0.0, 0.0) });
            std::tuple<int, Vec3d, Vec3d>& cPt = m_constraintPts.back();

            // Cell to single point constraint
            addConstraint(
                cellVerts, weights,
                { { &std::get<1>(cPt), 0.0, &std::get<2>(cPt) } }, { 1.0 },
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
PbdObjectGrasping::updateConstraints()
{
    const Mat3d rot = m_pickingGeometry->getRotation().transpose();

    // Update constraint point positions
    {
        for (auto& cPt : m_constraintPts)
        {
            const Vec3d offset = m_pickedPtIdxOffset[std::get<0>(cPt)];
            std::get<1>(cPt) = m_pickingGeometry->getPosition() + rot.transpose() * offset;
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
    auto pbdObj     = m_objA;

    std::shared_ptr<PbdModel> pbdModel = pbdObj->getPbdModel();

    m_taskGraph->addEdge(source, pbdObj->getTaskGraph()->getSource());
    m_taskGraph->addEdge(pbdObj->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pbdModel->getTaskGraph()->getSink());
}
} // namespace imstk