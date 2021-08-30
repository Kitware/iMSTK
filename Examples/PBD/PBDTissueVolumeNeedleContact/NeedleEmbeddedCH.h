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

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkCollisionUtils.h"
#include "PbdTriangleEmbeddingConstraint.h"

#include <unordered_map>

namespace imstk
{
class PbdCollisionSolver;
class PbdEdgeEdgeConstraint;
class PbdObject;
class PbdPointEdgeConstraint;
class PbdPointPointConstraint;
class PbdPointTriangleConstraint;
}

using namespace imstk;

///
/// \brief Sorts and test equivalence of a set of vertex indices
///
struct TriCell
{
    std::uint32_t vertexIds[3];

    TriCell(std::uint32_t id0, std::uint32_t id1, std::uint32_t id2)
    {
        vertexIds[0] = id0;
        vertexIds[1] = id1;
        vertexIds[2] = id2;
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
        if (vertexIds[1] > vertexIds[2])
        {
            std::swap(vertexIds[1], vertexIds[2]);
        }
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
    }

    // Test true equivalence
    bool operator==(const TriCell& other) const
    {
        // Only works if sorted
        return (vertexIds[0] == other.vertexIds[0] && vertexIds[1] == other.vertexIds[1]
                && vertexIds[2] == other.vertexIds[2]);
    }
};
namespace std
{
template<>
struct hash<TriCell>
{
    // A 128 int could garuntee no collisions but its harder to find support for
    std::size_t operator()(const TriCell& k) const
    {
        using std::size_t;
        using std::hash;

        // Assuming sorted
        const std::size_t r =
            symCantor(static_cast<size_t>(k.vertexIds[0]), static_cast<size_t>(k.vertexIds[1]));
        return symCantor(r, static_cast<size_t>(k.vertexIds[2]));
    }
};
}

///
/// \class NeedleEmbeddedCH
///
/// \brief Implements PBD-RBD embedded tissue handling
///
class NeedleEmbeddedCH : public CollisionHandling
{
public:
    NeedleEmbeddedCH() :
        m_solver(std::make_shared<PbdCollisionSolver>())
    {
    }

    virtual ~NeedleEmbeddedCH() override = default;

    virtual const std::string getTypeName() const override { return "NeedleEmbeddedCH"; }

public:
    std::shared_ptr<Geometry> getHandlingGeometryA() override
    {
        auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
        return (tissueObj == nullptr) ? nullptr : tissueObj->getPhysicsGeometry();
    }

public:
    ///
    /// \brief Corrects for velocity (restitution and friction) after PBD is complete
    ///
    void correctVelocities()
    {
        for (int i = 0; i < m_solverConstraints.size(); i++)
        {
            m_solverConstraints[i]->correctVelocity(m_friction, 1.0);
        }
    }

    void solve() { m_solver->solve(); }

    void setFriction(const double friction) { m_friction = friction; }
    const double getFriction() const { return m_friction; }

protected:
    ///
    /// \brief Add embedding constraints based off contact data
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override
    {
        auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());

        auto tissueGeom = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry());
        auto needleGeom = std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry());

        std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr   = tissueGeom->getVertexPositions();
        std::shared_ptr<VecDataArray<int, 4>>    tissueIndicesPtr    = tissueGeom->getTetrahedraIndices();
        auto                                     tissueVelocitiesPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(tissueGeom->getVertexAttribute("Velocities"));
        auto                                     tissueInvMassesPtr  = std::dynamic_pointer_cast<DataArray<double>>(tissueGeom->getVertexAttribute("InvMass"));

        VecDataArray<double, 3>&    tissueVertices   = *tissueVerticesPtr;
        const VecDataArray<int, 4>& tissueIndices    = *tissueIndicesPtr;
        VecDataArray<double, 3>&    tissueVelocities = *tissueVelocitiesPtr;
        const DataArray<double>&    tissueInvMasses  = *tissueInvMassesPtr;

        std::shared_ptr<VecDataArray<double, 3>> needleVerticesPtr = needleGeom->getVertexPositions();
        std::shared_ptr<VecDataArray<int, 2>>    needleIndicesPtr  = needleGeom->getLinesIndices();
        VecDataArray<double, 3>&                 needleVertices    = *needleVerticesPtr;
        const VecDataArray<int, 2>&              needleIndices     = *needleIndicesPtr;

        // Keep track of the constraints that are added *this iteration* vs those already present
        // so we can find the set that are no longer present
        std::unordered_set<std::shared_ptr<PbdTriangleEmbeddingConstraint>> m_constraintEnabled;
        auto                                                                isConstraintEnabled = [&](std::shared_ptr<PbdTriangleEmbeddingConstraint> constraint)
                                                                                                  {
                                                                                                      return m_constraintEnabled.count(constraint) != 0;
                                                                                                  };

        // Constrain the triangle to the intersection point
        // If constraint for triangle already exists, update existing intersection point
        auto addConstraint = [&](int v1, int v2, int v3, const Vec3d& iPt)
                             {
                                 // Hashable triangle
                                 TriCell triCell(v1, v2, v3);

                                 // If constraint doesn't exist OR if constraint exists but has been disabled
                                 if (m_faceConstraints.count(triCell) == 0 || m_faceConstraints[triCell] == nullptr)
                                 {
                                     auto constraint = std::make_shared<PbdTriangleEmbeddingConstraint>();

                                     // Push back to a list so mem locations don't change
                                     m_fixedPoints.push_back(iPt);
                                     m_fixedPointVelocities.push_back(Vec3d::Zero());

                                     constraint->initConstraint(
                                         { &m_fixedPoints.back(), 0.0, &m_fixedPointVelocities.back() },
                                         { &tissueVertices[v1], tissueInvMasses[v1], &tissueVelocities[v1] },
                                         { &tissueVertices[v2], tissueInvMasses[v2], &tissueVelocities[v2] },
                                         { &tissueVertices[v3], tissueInvMasses[v3], &tissueVelocities[v3] },
                    1.0, 1.0, &needleVertices[0], &needleVertices[1], m_friction);

                                     // Add the constraint to a map of face->constraint
                                     m_faceConstraints[triCell] = constraint;
                                     m_constraintEnabled.insert(constraint);
                                 }
                                 // If already contains, then update the intersection point
                                 else
                                 {
                                     // Get the existing constraint
                                     std::shared_ptr<PbdTriangleEmbeddingConstraint> constraint = m_faceConstraints[triCell];
                                     (*constraint->getVertexMassA().vertex) = iPt;
                                     m_constraintEnabled.insert(constraint);
                                 }
                             };

        // If needle has been inserted
        if (needleObj->getInserted())
        {
            if (elementsA.size() != elementsB.size())
            {
                return;
            }

            // Tet faces
            static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };

            // For every intersected element
            for (int i = 0; i < elementsA.size(); i++)
            {
                const CollisionElement& colElemA = elementsA[i];
                const CollisionElement& colElemB = elementsB[i];

                if (colElemA.m_type != CollisionElementType::CellIndex || colElemB.m_type != CollisionElementType::CellIndex)
                {
                    continue;
                }

                const CellIndexElement& elemA = colElemA.m_element.m_CellIndexElement;
                const CellIndexElement& elemB = colElemB.m_element.m_CellIndexElement;

                if (elemA.cellType == IMSTK_TETRAHEDRON && elemB.cellType == IMSTK_EDGE)
                {
                    Vec4i tet;
                    if (elemA.idCount == 1)
                    {
                        tet = tissueIndices[elemA.ids[0]];
                    }
                    else if (elemA.idCount == 4)
                    {
                        tet = Vec4i(elemA.ids[0], elemA.ids[1], elemA.ids[2], elemA.ids[3]);
                    }
                    std::array<Vec3d, 4> tetVerts;
                    tetVerts[0] = tissueVertices[tet[0]];
                    tetVerts[1] = tissueVertices[tet[1]];
                    tetVerts[2] = tissueVertices[tet[2]];
                    tetVerts[3] = tissueVertices[tet[3]];

                    std::array<Vec3d, 2> lineVerts;
                    if (elemB.idCount == 1)
                    {
                        const Vec2i& line = needleIndices[elemB.ids[0]];
                        lineVerts[0] = needleVertices[line[0]];
                        lineVerts[1] = needleVertices[line[1]];
                    }
                    else if (elemB.idCount == 2)
                    {
                        lineVerts[0] = needleVertices[elemB.ids[0]];
                        lineVerts[1] = needleVertices[elemB.ids[1]];
                    }

                    // Determine which faces intersect
                    int   face0 = -1;
                    int   face1 = -1;
                    Vec3d iPt0  = Vec3d::Zero();
                    Vec3d iPt1  = Vec3d::Zero();
                    if (CollisionUtils::testTetToSegment(tetVerts,
                        lineVerts[0], lineVerts[1],
                        face0, face1, iPt0, iPt1))
                    {
                        if (face0 != -1)
                        {
                            addConstraint(
                                tet[faces[face0][0]],
                                tet[faces[face0][1]],
                                tet[faces[face0][2]],
                                iPt0);
                        }
                        if (face1 != -1)
                        {
                            addConstraint(
                                tet[faces[face1][0]],
                                tet[faces[face1][1]],
                                tet[faces[face1][2]],
                                iPt1);
                        }
                    }
                }
            }
        }

        m_solverConstraints.resize(0);
        m_solverConstraints.reserve(m_faceConstraints.size());
        for (auto& i : m_faceConstraints)
        {
            if (i.second != nullptr && isConstraintEnabled(i.second))
            {
                m_solverConstraints.push_back(i.second.get());
            }
            else
            {
                // Cleanup (without deleting to avoid slight cost)
                i.second = nullptr;
            }
        }
        m_solver->addCollisionConstraints(&m_solverConstraints);
    }

private:
    // TriCell takes care of duplicate faces
    std::unordered_map<TriCell, std::shared_ptr<PbdTriangleEmbeddingConstraint>> m_faceConstraints;

    std::shared_ptr<PbdCollisionSolver>  m_solver = nullptr;
    std::vector<PbdCollisionConstraint*> m_solverConstraints; ///> List of PBD constraints

    // Lists to hold valid references to them in memory
    std::list<Vec3d> m_fixedPoints;
    std::list<Vec3d> m_fixedPointVelocities;

    //double m_restitution = 0.0; ///> Coefficient of restitution (1.0 = perfect elastic, 0.0 = inelastic)
    double m_friction = 0.001; ///> Coefficient of friction (1.0 = full frictional force, 0.0 = none)
};