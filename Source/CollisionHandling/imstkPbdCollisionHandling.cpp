/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdCollisionHandling.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdEdgeEdgeCCDConstraint.h"
#include "imstkPbdEdgeEdgeConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdPointEdgeConstraint.h"
#include "imstkPbdPointPointConstraint.h"
#include "imstkPbdPointTriangleConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkPointSet.h"
#include "imstkPointwiseMap.h"

namespace imstk
{
#define REGISTER_CASE(case0, case1, case2, func)                                                      \
    m_funcTable[{ case0, case1, case2 }] = [this](const ColElemSide& sideA, const ColElemSide& sideB) \
                                           { func(sideA, sideB); }

std::pair<PbdParticleId, Vec3d>
PbdCollisionHandling::getBodyAndContactPoint(const CollisionElement& elem, const CollisionSideData& data)
{
    if (elem.m_type == CollisionElementType::PointDirection)
    {
        return { { data.bodyId, 0 }, elem.m_element.m_PointDirectionElement.pt };
    }
    else
    {
        const PbdParticleId ptBv = getVertex(elem, data)[0];
        return { { data.bodyId, 0 }, (*data.vertices)[ptBv.second] };
    }
}

///
/// \brief Gets triangle, edge, or vertex from the mesh given the CollisionElement
/// \tparam Array type for vertex index storage, ex: VecDataArray<int, 4>, VecDataArray<int, 3>
/// \tparam Cell type the index array is representing
///
template<typename ArrType, int cellType>
static std::array<PbdParticleId, ArrType::NumComponents>
getElementVertIds(const CollisionElement& elem, const PbdCollisionHandling::CollisionSideData& side)
{
    // Note: The unrolling of this functions loops could be important to performance
    typename ArrType::ValueType cell = -ArrType::ValueType::Ones();
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == cellType)
    {
        // If one index it refers to the cell
        if (elem.m_element.m_CellIndexElement.idCount == 1)
        {
            cell = (*dynamic_cast<ArrType*>(side.indicesPtr))[elem.m_element.m_CellIndexElement.ids[0]];
        }
        else if (elem.m_element.m_CellIndexElement.idCount == ArrType::NumComponents)
        {
            for (int i = 0; i < ArrType::NumComponents; i++)
            {
                cell[i] = elem.m_element.m_CellIndexElement.ids[i];
            }
        }
    }

    std::array<PbdParticleId, ArrType::NumComponents> results;
    for (int i = 0; i < ArrType::NumComponents; i++)
    {
        results[i] = { -1, -1 };
    }
    if (cell[0] != -1)
    {
        if (side.mapPtr != nullptr)
        {
            for (int i = 0; i < ArrType::NumComponents; i++)
            {
                cell[i] = side.mapPtr->getParentVertexId(cell[i]);
            }
        }
        for (int i = 0; i < ArrType::NumComponents; i++)
        {
            int vid = cell[i];
            if (side.bodyId == 0)
            {
                vid = side.model->addVirtualParticle((*side.vertices)[vid], 0.0).second;
            }
            results[i] = { side.bodyId, vid };
        }
    }
    return results;
}

std::array<PbdParticleId, 2>
PbdCollisionHandling::getEdge(const CollisionElement& elem, const CollisionSideData& side)
{
    return getElementVertIds<VecDataArray<int, 2>, IMSTK_EDGE>(elem, side);
}

std::array<PbdParticleId, 3>
PbdCollisionHandling::getTriangle(const CollisionElement& elem, const CollisionSideData& side)
{
    return getElementVertIds<VecDataArray<int, 3>, IMSTK_TRIANGLE>(elem, side);
}

std::array<PbdParticleId, 1>
PbdCollisionHandling::getVertex(const CollisionElement& elem, const CollisionSideData& side)
{
    std::array<PbdParticleId, 1> results = { PbdParticleId(-1, -1) };
    int                          ptId    = -1;
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == IMSTK_VERTEX)
    {
        ptId = elem.m_element.m_CellIndexElement.ids[0];
    }
    else if (elem.m_type == CollisionElementType::PointIndexDirection)
    {
        ptId = elem.m_element.m_PointIndexDirectionElement.ptIndex;
    }
    if (ptId != -1)
    {
        if (side.mapPtr != nullptr)
        {
            ptId = side.mapPtr->getParentVertexId(ptId);
        }
        if (side.bodyId == 0)
        {
            ptId = side.model->addVirtualParticle((*side.vertices)[ptId], 0.0).second;
        }
        results[0] = { side.bodyId, ptId };
    }
    else
    {
        if (elem.m_type == CollisionElementType::PointDirection)
        {
            results[0] = { side.model->addVirtualParticle(elem.m_element.m_PointDirectionElement.pt, 0.0) };
        }
    }
    return results;
}

template<int N>
static std::array<Vec3d*, N>
getElementVertIdsPrev(const std::array<PbdParticleId, N>& ids,
                      const PbdCollisionHandling::CollisionSideData& side)
{
    std::array<Vec3d*, N> results;
    for (int i = 0; i < N; i++)
    {
        // Add all vertices as virtual, 0 mass
        std::shared_ptr<VecDataArray<double, 3>> vertices =
            dynamic_cast<PointSet*>(side.prevGeometry)->getVertexPositions();
        const int vertexId = ids[i].second;
        results[i] = &(*vertices)[vertexId];
    }
    return results;
}

std::ostream&
operator<<(std::ostream& os, const PbdCHTableKey& key)
{
    os << getContactCaseStr(key.elemAType) << " vs " <<
        getContactCaseStr(key.elemBType) << ", CCD: " << key.ccd;
    return os;
}

PbdCollisionHandling::PbdCollisionHandling()
{
    REGISTER_CASE(PbdContactCase::Vertex, PbdContactCase::Vertex, false, addConstraint_V_V);
    REGISTER_CASE(PbdContactCase::Vertex, PbdContactCase::Edge, false, addConstraint_V_E);
    REGISTER_CASE(PbdContactCase::Edge, PbdContactCase::Edge, false, addConstraint_E_E);
    REGISTER_CASE(PbdContactCase::Vertex, PbdContactCase::Triangle, false, addConstraint_V_T);

    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::Triangle, false, addConstraint_Body_T);
    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::Edge, false, addConstraint_Body_E);
    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::Vertex, false, addConstraint_Body_V);
    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::Primitive, false, addConstraint_Body_V);
    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::Body, false, addConstraint_Body_Body);

    // If swap occurs the colliding object could be on the LHS causing issues
    REGISTER_CASE(PbdContactCase::Primitive, PbdContactCase::Triangle, false, addConstraint_V_T);
    REGISTER_CASE(PbdContactCase::Primitive, PbdContactCase::Edge, false, addConstraint_V_E);
    REGISTER_CASE(PbdContactCase::Primitive, PbdContactCase::Vertex, false, addConstraint_V_V);

    // One way point direction resolution
    REGISTER_CASE(PbdContactCase::Vertex, PbdContactCase::None, false, addConstraint_V_V);
    REGISTER_CASE(PbdContactCase::Body, PbdContactCase::None, false, addConstraint_Body_V);

    // CCD cases
    REGISTER_CASE(PbdContactCase::Edge, PbdContactCase::Edge, true, addConstraint_E_E_CCD);
}

PbdCollisionHandling::~PbdCollisionHandling()
{
    deleteCollisionConstraints();
}

PbdCollisionHandling::CollisionSideData
PbdCollisionHandling::getDataFromObject(std::shared_ptr<CollidingObject> obj)
{
    // Pack info into struct, gives some contextual hints as well
    CollisionSideData side;
    auto              pbdObj = std::dynamic_pointer_cast<PbdObject>(obj);
    side.pbdObj  = pbdObj.get();              // Garunteed
    side.colObj  = obj.get();
    side.objType = ObjType::Colliding;
    std::shared_ptr<Geometry> collidingGeometry = side.colObj->getCollidingGeometry();
    side.geometry = collidingGeometry.get();

    if (side.pbdObj != nullptr)
    {
        if (side.pbdObj->getPbdBody()->bodyType == PbdBody::Type::RIGID)
        {
            side.objType = ObjType::PbdRigid;
        }
        else
        {
            side.objType = ObjType::PbdDeformable;
        }
        std::shared_ptr<PbdModel> model = side.pbdObj->getPbdModel();
        side.model = model.get();
        // If a physics geometry is provided always use that because
        // Either:
        //  A.) Physics geometry == Collision Geometry
        //  B.) A PointwiseMap is used and map should refer us back to physics geometry
        std::shared_ptr<Geometry> physicsGeometry = side.pbdObj->getPhysicsGeometry();
        side.geometry = physicsGeometry.get();
        side.bodyId   = side.pbdObj->getPbdBody()->bodyHandle;

        auto map = std::dynamic_pointer_cast<PointwiseMap>(side.pbdObj->getPhysicsToCollidingMap());
        side.mapPtr = map.get();
    }
    side.pointSet = dynamic_cast<PointSet*>(side.geometry);
    side.vertices = (side.pointSet == nullptr) ? nullptr : side.pointSet->getVertexPositions().get();
    if (side.objType == ObjType::PbdRigid)
    {
        if (auto pointSet = std::dynamic_pointer_cast<PointSet>(side.colObj->getCollidingGeometry()))
        {
            std::shared_ptr<VecDataArray<double, 3>> vertices = pointSet->getVertexPositions();
            side.vertices = vertices.get();
        }
        //side.vertices = side.colObj->getCollidingGeometry()
    }
    side.indicesPtr = nullptr;
    if (auto cellMesh = std::dynamic_pointer_cast<AbstractCellMesh>(side.colObj->getCollidingGeometry()))
    {
        std::shared_ptr<AbstractDataArray> indicesPtr = cellMesh->getAbstractCells();
        side.indicesPtr = indicesPtr.get();
    }

    return side;
}

PbdContactCase
PbdCollisionHandling::getCaseFromElement(const ColElemSide& elem)
{
    if (elem.data->objType == ObjType::PbdRigid)
    {
        return PbdContactCase::Body;
    }
    else
    {
        if (elem.elem->m_type == CollisionElementType::PointDirection)
        {
            return PbdContactCase::Primitive;
        }
        else if (elem.elem->m_type == CollisionElementType::CellIndex)
        {
            // 0 - vertex, 1 - edge, 2 - triangle
            return static_cast<PbdContactCase>(
                elem.elem->m_element.m_CellIndexElement.cellType);
        }
        else if (elem.elem->m_type == CollisionElementType::CellVertex)
        {
            return static_cast<PbdContactCase>(
                elem.elem->m_element.m_CellVertexElement.size);
        }
        // If not rigid, this must be asking to resolve the vertex
        else if (elem.elem->m_type == CollisionElementType::PointIndexDirection)
        {
            return PbdContactCase::Vertex;
        }
    }
    return PbdContactCase::None;
}

template<class T>
T*
PbdCollisionHandling::getCachedConstraint(ConstraintType type)
{
    if (!m_constraintCache[type].empty())
    {
        T* result = static_cast<T*>(m_constraintCache[type].back());
        m_constraintCache[type].pop_back();
        return result;
    }
    else
    {
        return new T;
    }
}

void
PbdCollisionHandling::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    if (m_clearData)
    {
        // Clear constraints vectors
        m_collisionConstraints.clear();
    }

    // Break early if no collision elements
    if (elementsA.size() != 0 || elementsB.size() != 0)
    {
        // Pack all the data needed for a particular side into a struct so we can
        // swap it with the contact & pass it around
        CollisionSideData dataSideA = getDataFromObject(getInputObjectA());
        dataSideA.compliance = m_compliance;
        dataSideA.stiffness  = m_stiffness[0];
        CollisionSideData dataSideB = getDataFromObject(getInputObjectB());
        dataSideB.compliance = m_compliance;
        dataSideB.stiffness  = (dataSideB.pbdObj == nullptr) ? 0.0 : m_stiffness[1];

        // Share the model with both sides even if B is not pbd, which makes it easier
        // to acquire the model without knowing which object is pbd
        if (dataSideB.model == nullptr)
        {
            dataSideB.model = dataSideA.model;
        }

        // If obj B is also pbd simulated, make sure they share the same model
        CHECK(dataSideB.pbdObj == nullptr || dataSideA.model == dataSideB.model) <<
            "PbdCollisionHandling input objects must share PbdModel";

        // For CCD (store if available)
        dataSideA.prevGeometry = m_colData->prevGeomA.get();
        dataSideB.prevGeometry = m_colData->prevGeomB.get();

        if (elementsA.size() == elementsB.size())
        {
            // Deal with two way contacts
            for (size_t i = 0; i < elementsA.size(); i++)
            {
                handleElementPair(
                    { &elementsA[i], &dataSideA },
                    { &elementsB[i], &dataSideB });
            }
        }
        else
        {
            // Deal with one way contacts (only one side is needed)
            for (size_t i = 0; i < elementsA.size(); i++)
            {
                handleElementPair(
                    { &elementsA[i], &dataSideA },
                    { nullptr, nullptr });
            }
            for (size_t i = 0; i < elementsB.size(); i++)
            {
                handleElementPair(
                    { &elementsB[i], &dataSideB },
                    { nullptr, nullptr });
            }
        }

        size_t constraints = 0;
        for (int i = 0; i < NumTypes; ++i)
        {
            constraints += m_constraintBins[i].size();
        }
    }

    if (m_processConstraints)
    {
        orderCollisionConstraints();

        if (m_collisionConstraints.size() == 0)
        {
            return;
        }
        // ObjA guaranteed to be PbdObject
        auto pbdObjectA = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
        pbdObjectA->getPbdModel()->getSolver()->addConstraints(&m_collisionConstraints);
    }
}

void
PbdCollisionHandling::handleElementPair(ColElemSide sideA, ColElemSide sideB)
{
    PbdCHTableKey key;
    key.elemAType = getCaseFromElement(sideA);
    key.elemBType = PbdContactCase::None;
    key.ccd       = sideA.elem->m_ccdData;

    // Data for sideB may not be present if handling one-way
    if (sideB.data != nullptr)
    {
        key.elemBType = getCaseFromElement(sideB);
        if (sideB.elem->m_ccdData)
        {
            key.ccd = true;
        }

        // Avoid a couple cases by swapping
        // Only V_T, no TV
        // Only V_E, no EV
        // Always Body on the right
        // Always Primitive on left
        if ((key.elemAType == PbdContactCase::Triangle && key.elemBType == PbdContactCase::Vertex)
            || (key.elemAType == PbdContactCase::Edge && key.elemBType == PbdContactCase::Vertex)
            || (key.elemBType == PbdContactCase::Body && key.elemAType != PbdContactCase::Body)
            || (key.elemAType != PbdContactCase::Primitive && key.elemBType == PbdContactCase::Primitive))
        {
            if (!(key.elemAType == PbdContactCase::Body && key.elemBType == PbdContactCase::Primitive))
            {
                std::swap(key.elemAType, key.elemBType);
                std::swap(sideA, sideB);
            }
        }
    }

    auto iter = m_funcTable.find(key);
    if (iter != m_funcTable.end())
    {
        iter->second(sideA, sideB);
    }
    else
    {
        // CH's may not handle all CollisionElements types
        LOG(INFO) << "Could not find handling case " << key;
    }
}

void
PbdCollisionHandling::addConstraint_Body_V(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const std::pair<PbdParticleId, Vec3d>& ptAAndContact = getBodyAndContactPoint(*sideA.elem, *sideA.data);
    PbdParticleId                          ptB;

    // If one-way/sideB doesn't exist
    if (sideB.data == nullptr)
    {
        // Try to resolve with PointDirection
        const CollisionElement::Element& elem = sideA.elem->m_element;
        Vec3d                            resolvePos;
        if (sideA.elem->m_type == CollisionElementType::PointIndexDirection)
        {
            const Vec3d& pos   = ptAAndContact.second;
            const Vec3d& dir   = elem.m_PointIndexDirectionElement.dir;
            const double depth = elem.m_PointIndexDirectionElement.penetrationDepth;
            resolvePos = pos + dir * depth;
        }
        else if (sideA.elem->m_type == CollisionElementType::PointDirection)
        {
            const Vec3d& pos   = elem.m_PointDirectionElement.pt;
            const Vec3d& dir   = elem.m_PointDirectionElement.dir;
            const double depth = elem.m_PointDirectionElement.penetrationDepth;
            resolvePos = pos + dir * depth;
        }
        else
        {
            return;
        }
        ptB = sideA.data->model->addVirtualParticle(resolvePos, 0.0);
    }
    else
    {
        ptB = getVertex(*sideB.elem, *sideB.data)[0];
    }

    PbdVertexToBodyConstraint* constraint = getCachedConstraint<PbdVertexToBodyConstraint>(BodyVertex);
    constraint->initConstraint(sideA.data->model->getBodies(),
                        ptAAndContact.first,
                        ptAAndContact.second,
                        ptB,
                        sideA.data->compliance);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[BodyVertex].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_Body_E(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const std::pair<PbdParticleId, Vec3d>& ptAAndContact = getBodyAndContactPoint(*sideA.elem, *sideA.data);
    std::array<PbdParticleId, 2>           ptsB = getEdge(*sideB.elem, *sideB.data);

    PbdEdgeToBodyConstraint* constraint = getCachedConstraint<PbdEdgeToBodyConstraint>(BodyEdge);
    constraint->initConstraint(sideB.data->model->getBodies(),
                        ptAAndContact.first,
                        ptAAndContact.second,
                        ptsB[0], ptsB[1],
                        sideA.data->compliance);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[BodyEdge].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_Body_T(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const std::pair<PbdParticleId, Vec3d>& ptAAndContact = getBodyAndContactPoint(*sideA.elem, *sideA.data);
    std::array<PbdParticleId, 3>           ptsB = getTriangle(*sideB.elem, *sideB.data);

    PbdTriangleToBodyConstraint* constraint = getCachedConstraint<PbdTriangleToBodyConstraint>(BodyTriangle);
    constraint->initConstraint(sideB.data->model->getBodies(),
                        ptAAndContact.first,
                        ptAAndContact.second,
                        ptsB[0], ptsB[1], ptsB[2],
                        sideA.data->compliance);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[BodyTriangle].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_Body_Body(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const std::pair<PbdParticleId, Vec3d>& ptAAndContact = getBodyAndContactPoint(*sideA.elem, *sideA.data);
    const std::pair<PbdParticleId, Vec3d>& ptBAndContact = getBodyAndContactPoint(*sideB.elem, *sideB.data);

    Vec3d normal = Vec3d::Zero();
    if (sideA.elem->m_type == CollisionElementType::PointDirection)
    {
        normal = sideA.elem->m_element.m_PointDirectionElement.dir;
    }

    PbdBodyToBodyNormalConstraint* constraint = getCachedConstraint<PbdBodyToBodyNormalConstraint>(BodyBody);

    constraint->initConstraint(
                        sideA.data->model->getBodies(),
                        ptAAndContact.first,
                        ptAAndContact.second,
                        ptBAndContact.first,
                        ptBAndContact.second,
                        normal,
                        sideA.data->compliance);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[BodyBody].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_V_T(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const PbdParticleId          ptA  = getVertex(*sideA.elem, *sideA.data)[0];
    std::array<PbdParticleId, 3> ptsB = getTriangle(*sideB.elem, *sideB.data);

    PbdPointTriangleConstraint* constraint = getCachedConstraint<PbdPointTriangleConstraint>(VertexTriangle);
    constraint->initConstraint(ptA, ptsB[0], ptsB[1], ptsB[2],
                        sideA.data->stiffness, sideB.data->stiffness);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setEnableBoundaryCollisions(m_enableBoundaryCollisions);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[VertexTriangle].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_E_E(const ColElemSide& sideA, const ColElemSide& sideB)
{
    std::array<PbdParticleId, 2> ptsA = getEdge(*sideA.elem, *sideA.data);
    std::array<PbdParticleId, 2> ptsB = getEdge(*sideB.elem, *sideB.data);

    PbdEdgeEdgeConstraint* constraint = getCachedConstraint<PbdEdgeEdgeConstraint>(EdgeEdge);
    constraint->initConstraint(ptsA[0], ptsA[1], ptsB[0], ptsB[1],
                        sideA.data->stiffness, sideB.data->stiffness);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setEnableBoundaryCollisions(m_enableBoundaryCollisions);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[EdgeEdge].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_E_E_CCD(
    const ColElemSide& sideA,
    const ColElemSide& sideB)
{
    std::array<PbdParticleId, 2> ptsA = getEdge(*sideA.elem, *sideA.data);
    std::array<PbdParticleId, 2> ptsB = getEdge(*sideB.elem, *sideB.data);

    // We want to refer to the same vertices but from a different geometry
    // We want to ensure the vertex is added as virtual
    std::array<Vec3d*, 2> prevPtsA = getElementVertIdsPrev<2>(ptsA, *sideA.data);
    std::array<Vec3d*, 2> prevPtsB = getElementVertIdsPrev<2>(ptsB, *sideB.data);

    PbdEdgeEdgeCCDConstraint* constraint = getCachedConstraint<PbdEdgeEdgeCCDConstraint>(EdgeEdgeCCD);
    constraint->initConstraint(
                        prevPtsA[0], prevPtsA[1], prevPtsB[0], prevPtsB[1],
                        ptsA[0], ptsA[1], ptsB[0], ptsB[1],
                        sideA.data->stiffness, sideB.data->stiffness,
                        m_ccdSubsteps);
    constraint->setEnableBoundaryCollisions(m_enableBoundaryCollisions);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[EdgeEdgeCCD].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_V_E(const ColElemSide& sideA, const ColElemSide& sideB)
{
    const PbdParticleId          ptA  = getVertex(*sideA.elem, *sideA.data)[0];
    std::array<PbdParticleId, 2> ptsB = getEdge(*sideB.elem, *sideB.data);

    PbdPointEdgeConstraint* constraint = getCachedConstraint<PbdPointEdgeConstraint>(VertexEdge);
    constraint->initConstraint(ptA, ptsB[0], ptsB[1],
                        sideA.data->stiffness, sideB.data->stiffness);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setEnableBoundaryCollisions(m_enableBoundaryCollisions);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[VertexEdge].push_back(constraint);
}

void
PbdCollisionHandling::addConstraint_V_V(const ColElemSide& sideA, const ColElemSide& sideB)
{
    // One special case with one-way
    const PbdParticleId ptA = getVertex(*sideA.elem, *sideA.data)[0];
    PbdParticleId       ptB;

    // If one-way/sideB doesn't exist
    if (sideB.data == nullptr)
    {
        // Try to resolve with PointDirection
        const CollisionElement::Element& elem = sideA.elem->m_element;
        Vec3d                            resolvePos;
        if (sideA.elem->m_type == CollisionElementType::PointIndexDirection)
        {
            const Vec3d& pos   = sideA.data->model->getBodies().getPosition(ptA);
            const Vec3d& dir   = elem.m_PointIndexDirectionElement.dir;
            const double depth = elem.m_PointIndexDirectionElement.penetrationDepth;
            resolvePos = pos + dir * depth;
        }
        else if (sideA.elem->m_type == CollisionElementType::PointDirection)
        {
            const Vec3d& pos   = elem.m_PointDirectionElement.pt;
            const Vec3d& dir   = elem.m_PointDirectionElement.dir;
            const double depth = elem.m_PointDirectionElement.penetrationDepth;
            resolvePos = pos + dir * depth;
        }
        else
        {
            return;
        }
        ptB = sideA.data->model->addVirtualParticle(resolvePos, 0.0);
    }
    else
    {
        ptB = getVertex(*sideB.elem, *sideB.data)[0];
    }

    PbdPointPointConstraint* constraint = getCachedConstraint<PbdPointPointConstraint>(VertexVertex);
    constraint->initConstraint(ptA, ptB,
                        sideA.data->stiffness,
        (sideB.data == nullptr) ? 0.0 : sideB.data->stiffness);
    constraint->setFriction(m_friction);
    constraint->setRestitution(m_restitution);
    constraint->setEnableBoundaryCollisions(m_enableBoundaryCollisions);
    constraint->setCorrectVelocity(m_useCorrectVelocity);
    m_constraintBins[VertexVertex].push_back(constraint);
}

void
PbdCollisionHandling::deleteCollisionConstraints()
{
    // Deletes all collision Constraints
    // There could be a large variance in constraints/contacts count,
    // 10s to 100s of constraints changing frequently over few frames.
    for (int i = 0; i < NumTypes; i++)
    {
        for (size_t j = 0; j < m_constraintCache[i].size(); j++)
        {
            delete  m_constraintCache[i][j];
        }
        m_constraintCache[i].resize(0);

        for (size_t j = 0; j < m_constraintBins[i].size(); j++)
        {
            delete  m_constraintBins[i][j];
        }
        m_constraintBins[i].resize(0);
    }

    m_collisionConstraints.resize(0);
}

void
PbdCollisionHandling::orderCollisionConstraints()
{
    // Add constraints from bins to constraints vector for solver
    for (int i = 0; i < NumTypes; i++)
    {
        m_collisionConstraints.insert(m_collisionConstraints.end(), m_constraintBins[i].begin(), m_constraintBins[i].end());
        m_constraintCache[i].insert(m_constraintCache[i].end(), m_constraintBins[i].begin(), m_constraintBins[i].end());
        m_constraintBins[i].resize(0);
    }
}
} // namespace imstk