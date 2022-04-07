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

#include "imstkPbdCollisionHandling.h"
#include "imstkCollisionData.h"
#include "imstkGeometryMap.h"
#include "imstkPbdEdgeEdgeConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdPointEdgeConstraint.h"
#include "imstkPbdPointPointConstraint.h"
#include "imstkPbdPointTriangleConstraint.h"
#include "imstkPbdSolver.h"
#include "imstkPointwiseMap.h"
#include "imstkSurfaceMesh.h"

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

static std::array<VertexMassPair, 1>
getVertex(const CollisionElement& elem, const MeshSide& side)
{
    int ptId = -1;
    if (elem.m_type == CollisionElementType::CellIndex && elem.m_element.m_CellIndexElement.cellType == IMSTK_VERTEX)
    {
        ptId = elem.m_element.m_CellIndexElement.ids[0];
    }
    else if (elem.m_type == CollisionElementType::PointIndexDirection)
    {
        ptId = elem.m_element.m_PointIndexDirectionElement.ptIndex;
    }
    std::array<VertexMassPair, 1> results;
    if (ptId != -1)
    {
        auto geomMap = dynamic_cast<PointwiseMap*>(side.m_mapPtr);
        if (geomMap != nullptr)
        {
            ptId = geomMap->getParentVertexId(ptId);
        }
        results[0] = { &side.m_vertices[ptId], side.m_invMasses[ptId], &side.m_velocities[ptId] };
    }
    return results;
}

static std::array<VertexMassPair, 2>
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
    std::array<VertexMassPair, 2> results;
    if (v1 != -1)
    {
        auto geomMap = dynamic_cast<PointwiseMap*>(side.m_mapPtr);
        if (side.m_mapPtr && geomMap != nullptr)
        {
            v1 = geomMap->getParentVertexId(v1);
            v2 = geomMap->getParentVertexId(v2);
        }
        results[0] = { &side.m_vertices[v1], side.m_invMasses[v1], &side.m_velocities[v1] };
        results[1] = { &side.m_vertices[v2], side.m_invMasses[v2], &side.m_velocities[v2] };
    }
    return results;
}

static std::array<VertexMassPair, 3>
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
    std::array<VertexMassPair, 3> results;
    if (v1 != -1)
    {
        auto geomMap = dynamic_cast<PointwiseMap*>(side.m_mapPtr);
        if (side.m_mapPtr && geomMap != nullptr)
        {
            v1 = geomMap->getParentVertexId(v1);
            v2 = geomMap->getParentVertexId(v2);
            v3 = geomMap->getParentVertexId(v3);
        }
        results[0] = { &side.m_vertices[v1], side.m_invMasses[v1], &side.m_velocities[v1] };
        results[1] = { &side.m_vertices[v2], side.m_invMasses[v2], &side.m_velocities[v2] };
        results[2] = { &side.m_vertices[v3], side.m_invMasses[v3], &side.m_velocities[v3] };
    }
    return results;
}

PbdCollisionHandling::PbdCollisionHandling() :
    m_pbdCollisionSolver(std::make_shared<PbdCollisionSolver>())
{
}

PbdCollisionHandling::~PbdCollisionHandling()
{
    for (const auto ptr: m_EEConstraintPool)
    {
        delete ptr;
    }
    for (const auto ptr: m_VTConstraintPool)
    {
        delete ptr;
    }
    for (const auto ptr : m_PEConstraintPool)
    {
        delete ptr;
    }
    for (const auto ptr : m_PPConstraintPool)
    {
        delete ptr;
    }
}

void
PbdCollisionHandling::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // Remove constraints without deallocating the memory
    // \todo: Consider using stack
    // \todo: Test counting them first
    m_VTConstraintPool.resize(0);
    m_EEConstraintPool.resize(0);
    m_PEConstraintPool.resize(0);
    m_PPConstraintPool.resize(0);

    m_fixedPoints.resize(0);
    m_fixedPointVelocities.resize(0);

    generateMeshMeshConstraints(elementsA, elementsB);
    generateMeshNonMeshConstraints(elementsA, elementsB);

    // Copy constraints
    for (int i = 0; i < m_PBDConstraints.size(); i++)
    {
        delete m_PBDConstraints[i];
    }
    m_PBDConstraints.resize(0);
    m_PBDConstraints.reserve(
        m_EEConstraintPool.size() + m_VTConstraintPool.size() +
        m_PEConstraintPool.size() + m_PPConstraintPool.size());

    for (size_t i = 0; i < m_EEConstraintPool.size(); i++)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_EEConstraintPool[i]));
    }
    for (size_t i = 0; i < m_VTConstraintPool.size(); i++)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_VTConstraintPool[i]));
    }
    for (size_t i = 0; i < m_PEConstraintPool.size(); i++)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_PEConstraintPool[i]));
    }
    for (size_t i = 0; i < m_PPConstraintPool.size(); i++)
    {
        m_PBDConstraints.push_back(static_cast<PbdCollisionConstraint*>(m_PPConstraintPool[i]));
    }

    if (m_PBDConstraints.size() == 0)
    {
        return;
    }

    m_pbdCollisionSolver->addCollisionConstraints(&m_PBDConstraints);
}

void
PbdCollisionHandling::generateMeshNonMeshConstraints(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // \todo: For when elementsA == 0 or elementsB == 0 and only one side of collision is handled
    std::shared_ptr<PbdObject>       pbdObjectA = std::dynamic_pointer_cast<PbdObject>(getInputObjectA()); // Garunteed
    std::shared_ptr<CollidingObject> objectB    = getInputObjectB();

    // For something to be a PbdObject it must have a pointset, it must also have invMasses defined
    std::shared_ptr<PointSet>                pointSetA      = std::dynamic_pointer_cast<PointSet>(pbdObjectA->getPhysicsGeometry());
    std::shared_ptr<VecDataArray<double, 3>> verticesAPtr   = pointSetA->getVertexPositions();
    std::shared_ptr<DataArray<double>>       invMassesAPtr  = std::dynamic_pointer_cast<DataArray<double>>(pointSetA->getVertexAttribute("InvMass"));
    std::shared_ptr<VecDataArray<double, 3>> velocitiesAPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetA->getVertexAttribute("Velocities"));

    const double stiffnessA = pbdObjectA->getPbdModel()->getConfig()->m_contactStiffness;

    MeshSide sideA(
        *verticesAPtr,
        *velocitiesAPtr,
        *invMassesAPtr,
        pbdObjectA->getPhysicsToCollidingMap().get(),
        nullptr);

    // \todo: Test if splitting constraints by type is more efficient
    for (int i = 0; i < elementsA.size(); i++)
    {
        const CollisionElement& colElemA = elementsA[i];

        // Point direction constraints are handled via pointpoint constraints
        if (colElemA.m_type == CollisionElementType::PointIndexDirection)
        {
            std::array<VertexMassPair, 1> vertexMassA = getVertex(colElemA, sideA);

            const Vec3d& dir   = colElemA.m_element.m_PointIndexDirectionElement.dir; // Direction to resolve point out of shape
            const Vec3d& pt    = *vertexMassA[0].vertex;                              // Point inside the shape
            const double depth = colElemA.m_element.m_PointIndexDirectionElement.penetrationDepth;

            // Point to resolve to
            m_fixedPoints.push_back(pt + dir * depth);
            m_fixedPointVelocities.push_back(Vec3d(0.0, 0.0, 0.0));

            addPPConstraint(
                { &m_fixedPoints.back(), 0.0, &m_fixedPointVelocities.back() },
                vertexMassA[0],
                0.0, stiffnessA);
        }
    }

    if (elementsA.size() == elementsB.size())
    {
        for (int i = 0; i < elementsA.size(); i++)
        {
            const CollisionElement& colElemA = elementsA[i];
            const CollisionElement& colElemB = elementsB[i];

            if (colElemA.m_type != CollisionElementType::CellIndex)
            {
                continue;
            }

            const CellTypeId cellTypeA = colElemA.m_element.m_CellIndexElement.cellType;

            // Triangle vs ...
            if (cellTypeA == IMSTK_TRIANGLE)
            {
                std::array<VertexMassPair, 3> vertexMassA = getTriangle(colElemA, sideA);

                bool shouldAddConstraint = false;
                // Triangle vs Vertex
                if (colElemB.m_type == CollisionElementType::CellVertex && colElemB.m_element.m_CellVertexElement.size == 1)
                {
                    m_fixedPoints.push_back(colElemB.m_element.m_CellVertexElement.pts[0]);
                    m_fixedPointVelocities.push_back(Vec3d(0.0, 0.0, 0.0));
                    shouldAddConstraint = true;
                }
                // Triangle vs PointDirection Vertex
                else if (colElemB.m_type == CollisionElementType::PointDirection)
                {
                    m_fixedPoints.push_back(colElemB.m_element.m_PointDirectionElement.pt);
                    m_fixedPointVelocities.push_back(Vec3d(0.0, 0.0, 0.0));
                    shouldAddConstraint = true;
                }
                if (shouldAddConstraint)
                {
                    // Only solve one side
                    addVTConstraint(
                        { &m_fixedPoints.back(), 0.0, &m_fixedPointVelocities.back() },
                        vertexMassA[0], vertexMassA[1], vertexMassA[2],
                        0.0, stiffnessA);
                }
            }
            // Edge vs ...
            else if (cellTypeA == IMSTK_EDGE)
            {
                std::array<VertexMassPair, 2> vertexMassA = getEdge(colElemA, sideA);

                bool shouldAddConstraint = false;
                // Edge vs vertex
                if (colElemB.m_type == CollisionElementType::CellVertex && colElemB.m_element.m_CellVertexElement.size == 1)
                {
                    m_fixedPoints.push_back(colElemB.m_element.m_PointDirectionElement.pt);
                    m_fixedPointVelocities.push_back(Vec3d(0.0, 0.0, 0.0));
                    shouldAddConstraint = true;
                }
                // Edge vs PointDirection vertex
                else if (colElemB.m_type == CollisionElementType::PointDirection)
                {
                    m_fixedPoints.push_back(colElemB.m_element.m_PointDirectionElement.pt);
                    m_fixedPointVelocities.push_back(Vec3d(0.0, 0.0, 0.0));
                    shouldAddConstraint = true;
                }

                if (shouldAddConstraint)
                {
                    // Only solve one side
                    addPEConstraint(
                        { &m_fixedPoints.back(), 0.0, &m_fixedPointVelocities.back() },
                        vertexMassA[0], vertexMassA[1],
                        0.0, stiffnessA);
                }
            }
        }
    }
}

void
PbdCollisionHandling::generateMeshMeshConstraints(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // TwoWay CD, constraints used solve both sides with one constraint
    if (elementsA.size() != elementsB.size())
    {
        return;
    }

    // Get the objects
    std::shared_ptr<PbdObject>       pbdObjectA = std::dynamic_pointer_cast<PbdObject>(getInputObjectA()); // Garunteed
    std::shared_ptr<CollidingObject> objectB    = getInputObjectB();
    std::shared_ptr<PbdObject>       pbdObjectB = std::dynamic_pointer_cast<PbdObject>(objectB);           // Not garunteed to be pbd

    // Get the objects geometries, this should both be physics geometry as both can only solve physics geometry,
    // however B could be a colliding object, in which case it would be collidingGeometry
    std::shared_ptr<PointSet> pointSetA = std::dynamic_pointer_cast<PointSet>(pbdObjectA->getPhysicsGeometry());
    std::shared_ptr<PointSet> pointSetB = std::dynamic_pointer_cast<PointSet>((pbdObjectB != nullptr) ? pbdObjectB->getPhysicsGeometry() : objectB->getCollidingGeometry());

    // objectB may not have a pointSet, this func is only for handling mesh vs mesh constraints
    if (pointSetB == nullptr)
    {
        return;
    }

    // Get Vertex buffers
    std::shared_ptr<VecDataArray<double, 3>> verticesAPtr = pointSetA->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> verticesBPtr = pointSetB->getVertexPositions();

    // Get the Mass buffers
    std::shared_ptr<DataArray<double>> invMassesAPtr = std::dynamic_pointer_cast<DataArray<double>>(pointSetA->getVertexAttribute("InvMass"));
    std::shared_ptr<DataArray<double>> invMassesBPtr = std::dynamic_pointer_cast<DataArray<double>>(pointSetB->getVertexAttribute("InvMass"));
    if (invMassesBPtr == nullptr)
    {
        invMassesBPtr = std::make_shared<DataArray<double>>(pointSetB->getNumVertices());
        invMassesBPtr->fill(0.0); // Assume infinite mass if no mass given
        pointSetB->setVertexAttribute("InvMass", invMassesBPtr);
    }

    // Get the velocities
    std::shared_ptr<VecDataArray<double, 3>> velocitiesAPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetA->getVertexAttribute("Velocities"));
    std::shared_ptr<VecDataArray<double, 3>> velocitiesBPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetB->getVertexAttribute("Velocities"));
    if (velocitiesBPtr == nullptr)
    {
        velocitiesBPtr = std::make_shared<VecDataArray<double, 3>>(pointSetB->getNumVertices());
        velocitiesBPtr->fill(Vec3d::Zero());
        pointSetB->setVertexAttribute("Velocities", velocitiesBPtr);
    }

    // Get the configs (one may be nullptr)
    const double stiffnessA = pbdObjectA->getPbdModel()->getConfig()->m_contactStiffness;
    const double stiffnessB = (pbdObjectB == nullptr) ? 0.0 : pbdObjectB->getPbdModel()->getConfig()->m_contactStiffness;

    MeshSide sideA(
        *verticesAPtr,
        *velocitiesAPtr,
        *invMassesAPtr,
        pbdObjectA->getPhysicsToCollidingMap().get(),
        nullptr);
    MeshSide sideB(
        *verticesBPtr,
        *velocitiesBPtr,
        *invMassesBPtr,
        (pbdObjectB == nullptr) ? nullptr : pbdObjectB->getPhysicsToCollidingMap().get(),
        nullptr);

    // \todo: Test if splitting constraints by type is more efficient
    for (int i = 0; i < elementsA.size(); i++)
    {
        const CollisionElement& colElemA = elementsA[i];
        const CollisionElement& colElemB = elementsB[i];

        if (colElemA.m_type != CollisionElementType::CellIndex || colElemB.m_type != CollisionElementType::CellIndex)
        {
            continue;
        }

        const CellTypeId cellTypeA = colElemA.m_element.m_CellIndexElement.cellType;
        const CellTypeId cellTypeB = colElemB.m_element.m_CellIndexElement.cellType;

        // Vertex vs Triangle
        if (cellTypeA == IMSTK_VERTEX && cellTypeB == IMSTK_TRIANGLE)
        {
            std::array<VertexMassPair, 1> vertexMassA = getVertex(colElemA, sideA);
            std::array<VertexMassPair, 3> vertexMassB = getTriangle(colElemB, sideB);

            // Setup a constraint to solve both sides (move both the triangle vertices and point vertex)
            addVTConstraint(
                vertexMassA[0],
                vertexMassB[0], vertexMassB[1], vertexMassB[2],
                stiffnessA, stiffnessB);
        }
        // Triangle vs Vertex
        else if (cellTypeA == IMSTK_TRIANGLE && cellTypeB == IMSTK_VERTEX)
        {
            std::array<VertexMassPair, 3> vertexMassA = getTriangle(colElemA, sideA);
            std::array<VertexMassPair, 1> vertexMassB = getVertex(colElemB, sideB);

            // Setup a constraint to solve both sides (move both the triangle vertices and point vertex)
            addVTConstraint(
                vertexMassB[0],
                vertexMassA[0], vertexMassA[1], vertexMassA[2],
                stiffnessB, stiffnessA);
        }
        // Edge vs Edge
        else if (cellTypeA == IMSTK_EDGE && cellTypeB == IMSTK_EDGE)
        {
            std::array<VertexMassPair, 2> vertexMassA = getEdge(colElemA, sideA);
            std::array<VertexMassPair, 2> vertexMassB = getEdge(colElemB, sideB);

            addEEConstraint(
                vertexMassA[0], vertexMassA[1],
                vertexMassB[0], vertexMassB[1],
                stiffnessA, stiffnessB);
        }
        // Edge vs Vertex
        else if (cellTypeA == IMSTK_EDGE && cellTypeB == IMSTK_VERTEX)
        {
            std::array<VertexMassPair, 2> vertexMassA = getEdge(colElemA, sideA);
            std::array<VertexMassPair, 1> vertexMassB = getVertex(colElemB, sideB);

            // Setup a constraint to solve both sides (move both the triangle vertices and point vertex)
            addPEConstraint(
                vertexMassB[0],
                vertexMassA[0], vertexMassA[1],
                stiffnessB, stiffnessA);
        }
        // Vertex vs Edge
        else if (cellTypeA == IMSTK_VERTEX && cellTypeB == IMSTK_EDGE)
        {
            std::array<VertexMassPair, 1> vertexMassA = getVertex(colElemA, sideA);
            std::array<VertexMassPair, 2> vertexMassB = getEdge(colElemB, sideB);

            // Setup a constraint to solve both sides (move both the triangle vertices and point vertex)
            addPEConstraint(
                vertexMassA[0],
                vertexMassB[0], vertexMassB[1],
                stiffnessA, stiffnessB);
        }
        // Vertex vs Vertex
        else if (cellTypeA == IMSTK_VERTEX && cellTypeB == IMSTK_VERTEX)
        {
            std::array<VertexMassPair, 1> vertexMassA = getVertex(colElemA, sideA);
            std::array<VertexMassPair, 1> vertexMassB = getVertex(colElemB, sideB);

            addPPConstraint(
                vertexMassA[0],
                vertexMassB[0],
                stiffnessA, stiffnessB);
        }
    }
}

void
PbdCollisionHandling::correctVelocities()
{
    for (int i = 0; i < m_PBDConstraints.size(); i++)
    {
        m_PBDConstraints[i]->correctVelocity(m_friction, m_restitution);
    }
}

void
PbdCollisionHandling::addVTConstraint(
    VertexMassPair ptA,
    VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
    double stiffnessA, double stiffnessB)
{
    PbdPointTriangleConstraint* constraint = new PbdPointTriangleConstraint();
    constraint->initConstraint(ptA, ptB1, ptB2, ptB3, stiffnessA, stiffnessB);
    m_VTConstraintPool.push_back(constraint);
}

void
PbdCollisionHandling::addEEConstraint(
    VertexMassPair ptA1, VertexMassPair ptA2,
    VertexMassPair ptB1, VertexMassPair ptB2,
    double stiffnessA, double stiffnessB)
{
    PbdEdgeEdgeConstraint* constraint = new PbdEdgeEdgeConstraint();
    constraint->initConstraint(ptA1, ptA2, ptB1, ptB2, stiffnessA, stiffnessB);
    m_EEConstraintPool.push_back(constraint);
}

void
PbdCollisionHandling::addPEConstraint(
    VertexMassPair ptA1,
    VertexMassPair ptB1, VertexMassPair ptB2,
    double stiffnessA, double stiffnessB)
{
    PbdPointEdgeConstraint* constraint = new PbdPointEdgeConstraint();
    constraint->initConstraint(ptA1, ptB1, ptB2, stiffnessA, stiffnessB);
    m_PEConstraintPool.push_back(constraint);
}

void
PbdCollisionHandling::addPPConstraint(
    VertexMassPair ptA, VertexMassPair ptB,
    double stiffnessA, double stiffnessB)
{
    PbdPointPointConstraint* constraint = new PbdPointPointConstraint();
    constraint->initConstraint(ptA, ptB, stiffnessA, stiffnessB);
    m_PPConstraintPool.push_back(constraint);
}
} // namespace imstk