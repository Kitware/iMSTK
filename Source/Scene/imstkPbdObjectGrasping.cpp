/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObjectGrasping.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCDObjectFactory.h"
#include "imstkCellPicker.h"
#include "imstkLineMesh.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkPointPicker.h"
#include "imstkPointwiseMap.h"
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
    MeshSide(VecDataArray<double, 3>& verticest, AbstractDataArray* indicesPtrt,
             PointwiseMap* mapt, int bodyIdt) : vertices(verticest),
        indicesPtr(indicesPtrt), map(mapt), bodyId(bodyIdt)
    {
    }

    VecDataArray<double, 3>& vertices;
    AbstractDataArray* indicesPtr = nullptr;
    PointwiseMap* map = nullptr;
    int bodyId = -1;
};

template<int N>
static std::vector<PbdParticleId>
getElement(const PickData& pickData, const MeshSide& side)
{
    std::vector<PbdParticleId> results(N);
    if (pickData.idCount == 1 && pickData.cellType != IMSTK_VERTEX) // If given cell index
    {
        const Eigen::Matrix<int, N, 1>& cell = (*dynamic_cast<VecDataArray<int, N>*>(side.indicesPtr))[pickData.ids[0]];
        for (int i = 0; i < N; i++)
        {
            int vertexId = cell[i];
            if (side.map != nullptr)
            {
                vertexId = side.map->getParentVertexId(vertexId);
            }
            results[i] = { side.bodyId, vertexId };
        }
    }
    else // If given vertex indices
    {
        for (int i = 0; i < N; i++)
        {
            int vertexId = pickData.ids[i];
            if (side.map != nullptr)
            {
                vertexId = side.map->getParentVertexId(vertexId);
            }
            results[i] = { side.bodyId, vertexId };
        }
    }
    return results;
}

static std::vector<double>
getWeights(const PbdState& bodies, const std::vector<PbdParticleId>& particles, const Vec3d& pt)
{
    std::vector<double> weights(particles.size());
    if (particles.size() == 4)
    {
        const Vec4d baryCoord = baryCentric(pt,
            bodies.getPosition(particles[0]),
            bodies.getPosition(particles[1]),
            bodies.getPosition(particles[2]),
            bodies.getPosition(particles[3]));
        weights[0] = baryCoord[0];
        weights[1] = baryCoord[1];
        weights[2] = baryCoord[2];
        weights[3] = baryCoord[3];
    }
    else if (particles.size() == 3)
    {
        const Vec3d baryCoord = baryCentric(pt,
            bodies.getPosition(particles[0]),
            bodies.getPosition(particles[1]),
            bodies.getPosition(particles[2]));
        weights[0] = baryCoord[0];
        weights[1] = baryCoord[1];
        weights[2] = baryCoord[2];
    }
    else if (particles.size() == 2)
    {
        const Vec2d baryCoord = baryCentric(pt,
            bodies.getPosition(particles[0]),
            bodies.getPosition(particles[1]));
        weights[0] = baryCoord[0];
        weights[1] = baryCoord[1];
    }
    else if (particles.size() == 1)
    {
        weights[0] = 1.0;
    }
    return weights;
}

PbdObjectGrasping::PbdObjectGrasping(std::shared_ptr<PbdObject> obj) :
    m_objectToGrasp(obj), m_pickMethod(std::make_shared<CellPicker>())
{
    m_pickingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectGrasping::updatePicking, this),
        "PbdPickingUpdate", true);
    m_taskGraph->addNode(m_pickingNode);

    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getCollisionSolveNode());

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
    m_collisionConstraints.clear();
}

void
PbdObjectGrasping::addPickConstraints()
{
    removePickConstraints();

    std::shared_ptr<PbdModel> model = m_objectToGrasp->getPbdModel();
    auto                      pbdPhysicsGeom =
        std::dynamic_pointer_cast<PointSet>(m_objectToGrasp->getPhysicsGeometry());

    // If the point set to pick hasn't been set yet, default it to the physics geometry
    // This would be the case if the user was mapping a geometry to another
    auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(m_geomToPick);
    if (m_geomToPick == nullptr)
    {
        pointSetToPick = pbdPhysicsGeom;
    }

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pbdPhysicsGeom->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;

    std::shared_ptr<AbstractDataArray> indicesPtr = nullptr;
    if (auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(pointSetToPick))
    {
        indicesPtr = cellMesh->getAbstractCells();
    }

    // If the user tries to pick
    PointwiseMap* map = nullptr;
    if (m_geometryToPickMap != nullptr)
    {
        map = m_geometryToPickMap.get();
    }

    // Place all the data into a struct to pass around & for quick access without casting
    // or dereferencing
    MeshSide meshStruct(
        *verticesPtr,
        indicesPtr.get(),
        map,
        m_objectToGrasp->getPbdBody()->bodyHandle);

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
            int             vertexId = data.ids[0];
            if (meshStruct.map != nullptr)
            {
                vertexId = meshStruct.map->getParentVertexId(vertexId);
            }

            const Vec3d         relativePos   = pickGeomRot * (vertices[vertexId] - pickGeomPos);
            const PbdParticleId graspPointPid = model->addVirtualParticle(vertices[vertexId], 0.0);
            m_constraintPts.push_back({ graspPointPid, relativePos, vertices[vertexId] });

            addConstraint(
                { { meshStruct.bodyId, vertexId } }, { 1.0 },
                { graspPointPid }, { 1.0 },
                m_stiffness, 0.0);
        }
    }
    else if (m_graspMode == GraspMode::Cell || m_graspMode == GraspMode::RayCell)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData&  data = pickData[i];
            const CellTypeId pickedCellType = data.cellType;

            std::vector<PbdParticleId> particles;
            if (pickedCellType == IMSTK_TETRAHEDRON)
            {
                particles = getElement<4>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                particles = getElement<3>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                particles = getElement<2>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_VERTEX)
            {
                particles = getElement<1>(data, meshStruct);
            }

            // Does not resolve duplicate vertices yet
            // But pbd implicit solve with reprojection avoids issues
            for (size_t j = 0; j < particles.size(); j++)
            {
                const int vertexId = particles[j].second;

                const Vec3d         relativePos   = pickGeomRot * (vertices[vertexId] - pickGeomPos);
                const PbdParticleId graspPointPid = model->addVirtualParticle(vertices[vertexId], 0.0);
                m_constraintPts.push_back({ graspPointPid, relativePos, vertices[vertexId] });

                addConstraint(
                    { particles[j] }, { 1.0 },
                    { graspPointPid }, { 1.0 },
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

            std::vector<PbdParticleId> particles;
            if (pickedCellType == IMSTK_TETRAHEDRON)
            {
                particles = getElement<4>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                particles = getElement<3>(data, meshStruct);
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                particles = getElement<2>(data, meshStruct);
            }

            // The point to constrain the element too
            const Vec3d pickingPt = data.pickPoint;

            std::vector<double> weights = getWeights(model->getBodies(), particles, pickingPt);

            const Vec3d         relativePos   = pickGeomRot * (pickingPt - pickGeomPos);
            const PbdParticleId graspPointPid = model->addVirtualParticle(pickingPt, 0.0);
            m_constraintPts.push_back({ graspPointPid, relativePos, pickingPt });

            // Cell to single point constraint
            addConstraint(
                particles, weights,
                { graspPointPid }, { 1.0 },
                m_stiffness, 0.0);
        }
    }

    m_collisionConstraints.reserve(m_constraints.size());
    for (int i = 0; i < m_constraints.size(); i++)
    {
        m_collisionConstraints.push_back(m_constraints[i].get());
    }
}

void
PbdObjectGrasping::addConstraint(
    const std::vector<PbdParticleId>& ptsA,
    const std::vector<double>& weightsA,
    const std::vector<PbdParticleId>& ptsB,
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
    std::shared_ptr<PbdModel> model = m_objectToGrasp->getPbdModel();

    // Update constraint point positions
    {
        const Vec3d& pos = m_graspGeom->getPosition();
        const Mat3d  rot = m_graspGeom->getRotation();
        for (size_t i = 0; i < m_constraintPts.size(); i++)
        {
            std::tuple<PbdParticleId, Vec3d, Vec3d>& cPt = m_constraintPts[i];
            std::shared_ptr<PbdConstraint>           c   = m_constraints[i];

            // Because virtual particles are cleared everytime, readd, also update the transform
            const Vec3d         relativePos = std::get<1>(cPt);
            const Vec3d         vPos = pos + rot * relativePos;
            const PbdParticleId vPid = model->addVirtualParticle(vPos, 0.0);
            // Then update the particle id (by convention the last particle is the virtual grasp point)
            c->getParticles().back() = vPid;
        }
    }

    if (m_collisionConstraints.size() > 0)
    {
        model->getCollisionSolver()->addConstraints(&m_collisionConstraints);
    }
}

void
PbdObjectGrasping::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    std::shared_ptr<PbdModel> pbdModel = m_objectToGrasp->getPbdModel();

    m_taskGraph->addEdge(source, m_objectToGrasp->getTaskGraph()->getSource());
    m_taskGraph->addEdge(m_objectToGrasp->getTaskGraph()->getSink(), sink);

    // The ideal location is after the internal positional solve, but before collisions are solved
    m_taskGraph->addEdge(pbdModel->getSolveNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pbdModel->getCollisionSolveNode());
}

bool
PbdObjectGrasping::hasConstraints() const
{
    return !m_constraints.empty();
}
} // namespace imstk