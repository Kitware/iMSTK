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
#include "imstkPbdContactConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkPointPicker.h"
#include "imstkPointwiseMap.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVertexPicker.h"
#include "imstkPbdAngularConstraint.h"

namespace imstk
{
///
/// \struct GraspedData
///
/// \brief Info needed to add a constraint for the grasped object
/// Garunteed to be a PbdObject
///
struct GraspedData
{
    // You can grasp a deformable or rigid
    enum class Type
    {
        Deformable,
        Rigid
    };

    GraspedData() = default;

    PbdObject* pbdObj = nullptr;
    Type objType      = Type::Deformable;

    VecDataArray<double, 3>* vertices = nullptr;
    AbstractDataArray* indices = nullptr;
    PointwiseMap* map = nullptr;
    int bodyId = -1;
};

static GraspedData
unpackGraspedSide(std::shared_ptr<PbdObject>   obj,
                  std::shared_ptr<Geometry>    geometry,
                  std::shared_ptr<GeometryMap> geomMap)
{
    GraspedData data;
    data.pbdObj = obj.get();
    data.bodyId = obj->getPbdBody()->bodyHandle;
    if (obj->getPbdBody()->bodyType == PbdBody::Type::RIGID)
    {
        data.objType = GraspedData::Type::Rigid;
    }
    else
    {
        data.objType = GraspedData::Type::Deformable;
    }

    if (auto pointSetToPick = std::dynamic_pointer_cast<PointSet>(geometry))
    {
        // Always should be the data that is constrained (ie: the physics mesh)
        data.vertices = std::dynamic_pointer_cast<PointSet>(obj->getPhysicsGeometry())->getVertexPositions().get();

        // This should stil be the picked geometry
        std::shared_ptr<AbstractDataArray> indicesPtr = nullptr;
        if (auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(pointSetToPick))
        {
            data.indices = cellMesh->getAbstractCells().get();
        }

        // If the user tries to pick
        if (geomMap != nullptr)
        {
            if (auto ptMap = std::dynamic_pointer_cast<PointwiseMap>(geomMap))
            {
                data.map = ptMap.get();
            }
        }
    }
    return data;
}

///
/// \struct GrasperData
///
/// \brief GrasperData is either a ray, a grasping geometry, or
/// another PbdObject (rigid)
///
struct GrasperData
{
    // You can grasp with a geometry or with another pbd object
    enum class Type
    {
        Geometry,
        Rigid
    };

    GrasperData() = default;

    Type objType      = Type::Geometry;
    PbdObject* pbdObj = nullptr;
    Geometry* grasperGeometry = nullptr;
    int bodyId = -1;
};

static GrasperData
unpackGrasperSide(std::shared_ptr<PbdObject> grasperObject,
                  std::shared_ptr<Geometry>  grasperGeometry)
{
    GrasperData data;
    data.objType = GrasperData::Type::Geometry;
    if (grasperObject != nullptr)
    {
        data.bodyId  = grasperObject->getPbdBody()->bodyHandle;
        data.pbdObj  = grasperObject.get();
        data.objType = GrasperData::Type::Rigid;
    }

    data.grasperGeometry = grasperGeometry.get();
    return data;
}

template<int N>
static std::vector<PbdParticleId>
getElement(const PickData& pickData, const GraspedData& side)
{
    std::vector<PbdParticleId> results(N);
    if (pickData.idCount == 1 && pickData.cellType != IMSTK_VERTEX) // If given cell index
    {
        const Eigen::Matrix<int, N, 1>& cell = (*dynamic_cast<VecDataArray<int, N>*>(side.indices))[pickData.ids[0]];
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

PbdObjectGrasping::PbdObjectGrasping(std::shared_ptr<PbdObject> graspedObject,
                                     std::shared_ptr<PbdObject> grasperObject) :
    m_objectToGrasp(graspedObject),
    m_grasperObject(grasperObject),
    m_pickMethod(std::make_shared<CellPicker>())
{
    m_pickingNode = std::make_shared<TaskNode>(std::bind(&PbdObjectGrasping::updatePicking, this),
        "PbdPickingUpdate", true);
    m_taskGraph->addNode(m_pickingNode);

    m_taskGraph->addNode(m_objectToGrasp->getTaskGraph()->getSource());
    m_taskGraph->addNode(m_objectToGrasp->getTaskGraph()->getSink());

    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getSolveNode());
    m_taskGraph->addNode(m_objectToGrasp->getPbdModel()->getIntegratePositionNode());

    if (grasperObject != nullptr)
    {
        CHECK(grasperObject->getPbdModel() == m_objectToGrasp->getPbdModel()) <<
            "Grasper object and object to grasp must shared a PbdModel";
        m_taskGraph->addNode(grasperObject->getTaskGraph()->getSource());
        m_taskGraph->addNode(grasperObject->getTaskGraph()->getSink());
    }
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
    // LOG(INFO) << "Begin grasp";
}

void
PbdObjectGrasping::beginCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType)
{
    auto cellPicker = std::make_shared<CellPicker>();
    cellPicker->setPickingGeometry(geometry);

    // If no cd provided try to automatically pick one
    if (cdType == "")
    {
        std::shared_ptr<Geometry> pbdPhysicsGeom = m_objectToGrasp->getPhysicsGeometry();

        // If a specific geometry wasn't specified to pick, then use the physics geometry
        std::shared_ptr<Geometry> geomtryToPick = (m_geomToPick == nullptr) ? pbdPhysicsGeom : m_geomToPick;
        cdType = CDObjectFactory::getCDType(*geometry, *geomtryToPick);
    }
    cellPicker->setCollisionDetection(CDObjectFactory::makeCollisionDetection(cdType));

    m_pickMethod = cellPicker;
    m_graspMode  = GraspMode::Cell;
    m_graspGeom  = geometry;

    m_isGrasping = true;
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
}

void
PbdObjectGrasping::endGrasp()
{
    m_isGrasping = false;
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
    std::shared_ptr<Geometry> pbdPhysicsGeom = m_objectToGrasp->getPhysicsGeometry();

    // If a specific geometry wasn't specified to pick, then use the physics geometry
    std::shared_ptr<Geometry> geomtryToPick = (m_geomToPick == nullptr) ? pbdPhysicsGeom : m_geomToPick;

    GraspedData graspedData = unpackGraspedSide(m_objectToGrasp, geomtryToPick, m_geometryToPickMap);
    GrasperData grasperData = unpackGrasperSide(m_grasperObject, m_graspGeom);

    const Vec3d& pickGeomPos = m_graspGeom->getPosition();
    const Mat3d  pickGeomRot = m_graspGeom->getRotation().transpose();

    // Perform the picking
    const std::vector<PickData>& pickData = m_pickMethod->pick(geomtryToPick);

    // Digest the pick data based on grasp mode
    if (m_graspMode == GraspMode::Vertex)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData& data     = pickData[i];
            int             vertexId = data.ids[0];
            if (graspedData.map != nullptr)
            {
                vertexId = graspedData.map->getParentVertexId(vertexId);
            }

            if (graspedData.objType == GraspedData::Type::Deformable
                && grasperData.objType == GrasperData::Type::Geometry)
            {
                const Vec3d         relativePos   = pickGeomRot * ((*graspedData.vertices)[vertexId] - pickGeomPos);
                const PbdParticleId graspPointPid = model->addVirtualParticle((*graspedData.vertices)[vertexId], 0.0);
                m_constraintPts.push_back({ graspPointPid, relativePos });

                addPointToPointConstraint(
                    { { graspedData.bodyId, vertexId } }, { 1.0 },
                    { graspPointPid }, { 1.0 },
                    m_stiffness, 0.0);
            }
            else if (graspedData.objType == GraspedData::Type::Deformable
                     && grasperData.objType == GrasperData::Type::Rigid)
            {
                addPointToBodyConstraint(
                    { graspedData.bodyId, vertexId },
                    { grasperData.bodyId, 0 },
                    (*graspedData.vertices)[vertexId],
                    m_compliance);
            }
        }
    }
    else if (m_graspMode == GraspMode::Cell || m_graspMode == GraspMode::RayCell)
    {
        for (size_t i = 0; i < pickData.size(); i++)
        {
            const PickData&  data = pickData[i];
            const CellTypeId pickedCellType = data.cellType;

            // If we select something without ids (typically analytic geometry)
            if (data.idCount == 0)
            {
                // Just add one constraint at the pick point
                if (graspedData.objType == GraspedData::Type::Rigid)
                {
                    if (grasperData.objType == GrasperData::Type::Geometry)
                    {
                        LOG(FATAL) << "Grasping rigid with static geometry (analytic) not supported";
                    }
                    else if (grasperData.objType == GrasperData::Type::Rigid)
                    {
                        addBodyToBodyConstraint(
                            { grasperData.bodyId, 0 },
                            { graspedData.bodyId, 0 },
                            data.pickPoint,
                            m_compliance);
                    }
                }
                else
                {
                    LOG(FATAL) << "Grasping deformable without any ids not supported";
                }
            }
            // If we select something with ids (typically meshes)
            else
            {
                // Get vertices selected (if cells selected, get vertices of those cells)
                std::vector<PbdParticleId> particles;
                if (pickedCellType == IMSTK_TETRAHEDRON)
                {
                    particles = getElement<4>(data, graspedData);
                }
                else if (pickedCellType == IMSTK_TRIANGLE)
                {
                    particles = getElement<3>(data, graspedData);
                }
                else if (pickedCellType == IMSTK_EDGE)
                {
                    particles = getElement<2>(data, graspedData);
                }
                else if (pickedCellType == IMSTK_VERTEX)
                {
                    particles = getElement<1>(data, graspedData);
                }

                // Does not resolve duplicate vertices yet, but implicit solve makes that ok
                for (size_t j = 0; j < particles.size(); j++)
                {
                    const int    vertexId  = particles[j].second;
                    const Vec3d& vertexPos = (*graspedData.vertices)[vertexId];

                    // If grasper is rigid
                    if (grasperData.objType == GrasperData::Type::Rigid)
                    {
                        // If grasped is a mesh, at pbd particle poitn to pbd body constraint
                        if (graspedData.objType == GraspedData::Type::Deformable)
                        {
                            // Deformable point to body constraint
                            addPointToBodyConstraint(particles[j],
                                { grasperData.bodyId, 0 },
                                vertexPos,
                                m_compliance);
                        }
                        // If grasped is a rigid (but still has a mesh), add body to body constraint
                        else if (graspedData.objType == GraspedData::Type::Rigid)
                        {
                            addBodyToBodyConstraint(
                                { grasperData.bodyId, 0 },
                                { graspedData.bodyId, 0 },
                                vertexPos,
                                m_compliance);
                        }
                    }
                    // If grasper is not rigid, static/unsimulated, then add virtual
                    // points moved around relative to the grasping geometry (in updateConstraints)
                    else
                    {
                        // If grasped is a mesh add virtual points at mesh positions
                        if (graspedData.objType == GraspedData::Type::Deformable)
                        {
                            const Vec3d         relativePos   = pickGeomRot * (vertexPos - pickGeomPos);
                            const PbdParticleId graspPointPid = model->addVirtualParticle(vertexPos, 0.0);
                            m_constraintPts.push_back({ graspPointPid, relativePos });

                            addPointToPointConstraint(
                                { particles[j] }, { 1.0 },
                                { graspPointPid }, { 1.0 },
                                m_stiffness, 0.0);
                        }
                        else
                        {
                            LOG(FATAL) << "Grasping rigid with static geometry (mesh) not supported";
                        }
                    }
                }
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
                particles = getElement<4>(data, graspedData);
            }
            else if (pickedCellType == IMSTK_TRIANGLE)
            {
                particles = getElement<3>(data, graspedData);
            }
            else if (pickedCellType == IMSTK_EDGE)
            {
                particles = getElement<2>(data, graspedData);
            }

            // The point to constrain the element too
            const Vec3d pickingPt = data.pickPoint;

            std::vector<double> weights = getWeights(model->getBodies(), particles, pickingPt);

            const Vec3d         relativePos   = pickGeomRot * (pickingPt - pickGeomPos);
            const PbdParticleId graspPointPid = model->addVirtualParticle(pickingPt, 0.0);
            m_constraintPts.push_back({ graspPointPid, relativePos });

            // Cell to single point constraint
            addPointToPointConstraint(
                particles, weights,
                { graspPointPid }, { 1.0 },
                m_stiffness, 0.0);
        }
    }

    if (m_constraints.size() == 0)
    {
        m_isGrasping = false;
    }

    m_collisionConstraints.reserve(m_constraints.size());
    for (int i = 0; i < m_constraints.size(); i++)
    {
        m_collisionConstraints.push_back(m_constraints[i].get());
    }
}

void
PbdObjectGrasping::addPointToPointConstraint(
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
PbdObjectGrasping::addBodyToBodyConstraint(
    const PbdParticleId& graspedBodyId,
    const PbdParticleId& grasperBodyId,
    const Vec3d&         supportPt,
    const double         compliance)
{
    std::shared_ptr<PbdModel> model = m_objectToGrasp->getPbdModel();

    // Rigid on rigid grasping
    // Constrain supportPt on body0 to supportPt on body1 by a distance of 0
    auto constraint = std::make_shared<PbdBodyToBodyDistanceConstraint>();
    constraint->initConstraint(
        model->getBodies(),
        graspedBodyId,
        supportPt,
        grasperBodyId,
        supportPt,
        0.0, compliance);
    m_constraints.push_back(constraint);
    auto angularConstraint = std::make_shared<PbdAngularDistanceConstraint>();
    angularConstraint->initConstraintOffset(model->getBodies(),
        graspedBodyId, grasperBodyId, 0.0);
    m_constraints.push_back(angularConstraint);
}

void
PbdObjectGrasping::addPointToBodyConstraint(
    const PbdParticleId& graspedParticleId,
    const PbdParticleId& grasperBodyId,
    const Vec3d&         pointOnBody,
    const double         compliance)
{
    std::shared_ptr<PbdModel> model = m_objectToGrasp->getPbdModel();

    // Rigid on deformable
    // Constrain supportPt on body0 to supportPt on body1 by a distance of 0
    auto constraint = std::make_shared<PbdBodyToBodyDistanceConstraint>();
    constraint->initConstraint(
        model->getBodies(),
        grasperBodyId,
        pointOnBody,
        graspedParticleId,
        0.0, compliance);
    m_constraints.push_back(constraint);
}

void
PbdObjectGrasping::updatePicking()
{
    // If started picking
    if ((!m_isPrevGrasping && m_isGrasping) || m_regrasp)
    {
        m_objectToGrasp->updateGeometries();
        addPickConstraints();
        m_regrasp = false;
    }

    // If stopped picking
    if (!m_isGrasping && m_isPrevGrasping)
    {
        m_objectToGrasp->updateGeometries();
        removePickConstraints();
    }
    // Push back the picking state
    m_isPrevGrasping = m_isGrasping;

    if (m_isGrasping)
    {
        m_objectToGrasp->updateGeometries();
        updateConstraints();
    }
}

void
PbdObjectGrasping::updateConstraints()
{
    std::shared_ptr<PbdModel> model = m_objectToGrasp->getPbdModel();

    // Update the grasp points when not doing two-way.
    // In two-way the points are automatically recomputed in the constraint relative
    // to the rigid body (via local position saved).
    // When not doing two-way we need to update the points manually.
    if (m_grasperObject == nullptr)
    {
        const Vec3d& pos = m_graspGeom->getPosition();
        const Mat3d  rot = m_graspGeom->getRotation();
        for (size_t i = 0; i < m_constraintPts.size(); i++)
        {
            std::tuple<PbdParticleId, Vec3d>& cPt = m_constraintPts[i];
            std::shared_ptr<PbdConstraint>    c   = m_constraints[i];

            // Because virtual particles are cleared everytime, re-add, also update the transform
            const Vec3d         relativePos = std::get<1>(cPt);
            const Vec3d         vPos = pos + rot * relativePos;
            const PbdParticleId vPid = model->addVirtualParticle(vPos, 0.0);
            // Then update the particle id (by convention the last particle is the virtual grasp point)
            c->getParticles().back() = vPid;
        }
    }

    if (m_collisionConstraints.size() > 0)
    {
        model->getSolver()->addConstraints(&m_collisionConstraints);
    }
}

void
PbdObjectGrasping::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    std::shared_ptr<PbdModel> pbdModel = m_objectToGrasp->getPbdModel();

    // Add source and sink connections for a valid graph at all times
    m_taskGraph->addEdge(source, m_objectToGrasp->getTaskGraph()->getSource());
    m_taskGraph->addEdge(m_objectToGrasp->getTaskGraph()->getSink(), sink);

    if (m_grasperObject != nullptr)
    {
        m_taskGraph->addEdge(source, m_grasperObject->getTaskGraph()->getSource());
        m_taskGraph->addEdge(m_grasperObject->getTaskGraph()->getSink(), sink);
    }

    // The ideal location is after the internal positional solve, but before collisions are solved
    m_taskGraph->addEdge(pbdModel->getIntegratePositionNode(), m_pickingNode);
    m_taskGraph->addEdge(m_pickingNode, pbdModel->getSolveNode());
}

bool
PbdObjectGrasping::hasConstraints() const
{
    return !m_constraints.empty();
}
} // namespace imstk