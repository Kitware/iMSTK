/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleEmbeddedCH.h"
#include "EmbeddingConstraint.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkRigidBodyModel2.h"
#include "imstkTetrahedralMesh.h"
#include "NeedleObject.h"

#include <unordered_set>

using namespace imstk;

NeedleEmbeddedCH::NeedleEmbeddedCH() :
    m_solver(std::make_shared<PbdCollisionSolver>())
{
}

std::shared_ptr<Geometry>
NeedleEmbeddedCH::getHandlingGeometryA()
{
    auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    return (tissueObj == nullptr) ? nullptr : tissueObj->getPhysicsGeometry();
}

void
NeedleEmbeddedCH::correctVelocities()
{
    for (size_t i = 0; i < m_solverConstraints.size(); i++)
    {
        m_solverConstraints[i]->correctVelocity(m_friction, 1.0);
    }
}

void
NeedleEmbeddedCH::solve()
{
    m_solver->solve();
}

void
NeedleEmbeddedCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());

    // If needle needle is touching the surface
    if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
    {
        // Get force along the needle axes
        const Vec3d  needleAxes = needleObj->getNeedleAxes();
        const double fN = std::max(needleAxes.dot(needleObj->getRigidBody()->getForce()), 0.0);

        // If the normal force exceeds the threshold, mark needle as inserted
        if (fN > needleObj->getForceThreshold())
        {
            LOG(INFO) << "Puncture!";
            needleObj->setCollisionState(NeedleObject::CollisionState::INSERTED);
        }
    }

    // If the needle is inserted
    if (needleObj->getCollisionState() == NeedleObject::CollisionState::INSERTED)
    {
        // Check if there are no tet intersections. If none, mark removed
        if (elementsA.size() == 0)
        {
            LOG(INFO) << "Unpunctured!";
            needleObj->setCollisionState(NeedleObject::CollisionState::REMOVED);
            m_faceConstraints.clear();
        }
    }
    // If the needle is not inserted
    else
    {
        // Don't run the embedded CH
        return;
    }

    auto tissueGeom = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry());
    auto needleGeom = std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry());
    needleGeom->updatePostTransformData();

    std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = tissueGeom->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 4>>    tissueIndicesPtr  = tissueGeom->getTetrahedraIndices();

    auto tissueVelocitiesPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(tissueGeom->getVertexAttribute("Velocities"));
    auto tissueInvMassesPtr  = std::dynamic_pointer_cast<DataArray<double>>(tissueGeom->getVertexAttribute("InvMass"));

    VecDataArray<double, 3>&    tissueVertices   = *tissueVerticesPtr;
    const VecDataArray<int, 4>& tissueIndices    = *tissueIndicesPtr;
    VecDataArray<double, 3>&    tissueVelocities = *tissueVelocitiesPtr;
    const DataArray<double>&    tissueInvMasses  = *tissueInvMassesPtr;

    std::shared_ptr<VecDataArray<double, 3>> needleVerticesPtr = needleGeom->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 2>>    needleIndicesPtr  = needleGeom->getLinesIndices();
    VecDataArray<double, 3>&                 needleVertices    = *needleVerticesPtr;
    const VecDataArray<int, 2>&              needleIndices     = *needleIndicesPtr;

    // Keep track of the constraints that are added *this iteration* vs those already present
    // in m_faceConstraints so we can find the set that are no longer present
    std::unordered_set<std::shared_ptr<EmbeddingConstraint>> m_constraintEnabled;

    // Constrain the triangle to the intersection point
    // If constraint for triangle already exists, update existing intersection point
    m_debugEmbeddingPoints.clear();
    m_debugEmbeddedTriangles.clear();
    auto addConstraint =
        [&](int v1, int v2, int v3, const Vec3d& iPt)
        {
            // Hashable triangle (to resolve shared triangles, any order of v1,v2,v3 maps to same constraint)
            TriCell triCell(v1, v2, v3);

            m_debugEmbeddingPoints.push_back(iPt);
            m_debugEmbeddedTriangles.push_back(Vec3i(v1, v2, v3));

            // If constraint doesn't already exist for this triangle
            if (m_faceConstraints.count(triCell) == 0)
            {
                auto constraint = std::make_shared<EmbeddingConstraint>(needleObj->getRigidBody());

                constraint->initConstraint(
                    { &tissueVertices[v1], tissueInvMasses[v1], &tissueVelocities[v1] },
                    { &tissueVertices[v2], tissueInvMasses[v2], &tissueVelocities[v2] },
                    { &tissueVertices[v3], tissueInvMasses[v3], &tissueVelocities[v3] },
                &needleVertices[0], &needleVertices[1]);

                // Add the constraint to a map of face->constraint
                m_faceConstraints[triCell] = constraint;
                //printf("Adding embedding constraint at %d, %d, %d\n", v1, v2, v3);
            }

            // Mark as present
            m_constraintEnabled.insert(m_faceConstraints[triCell]);
        };

    // For every intersected element
    for (size_t i = 0; i < elementsA.size(); i++)
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
            const double lineLength = (lineVerts[1] - lineVerts[0]).norm();

            // For every face of the tet
            static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };
            for (int j = 0; j < 4; j++)
            {
                // Find intersection point and add constraints
                const Vec3d& a = tissueVertices[tet[faces[j][0]]];
                const Vec3d& b = tissueVertices[tet[faces[j][1]]];
                const Vec3d& c = tissueVertices[tet[faces[j][2]]];

                Vec3d iPt;
                if (CollisionUtils::testPlaneLine(lineVerts[0], lineVerts[1], a, (b - a).cross(c - a).normalized(), iPt))
                {
                    Vec3d uvw = baryCentric(iPt, a, b, c);
                    if (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0)
                    {
                        // If within line bounds
                        const double t = (lineVerts[1] - lineVerts[0]).normalized().dot(iPt - lineVerts[0]);
                        if (t > 0.0 && t < lineLength)
                        {
                            addConstraint(
                                tet[faces[j][0]],
                                tet[faces[j][1]],
                                tet[faces[j][2]],
                                iPt);
                        }
                    }
                }
            }
        }
    }

    // Add constraint to the PBD solver and RBD system
    m_solverConstraints.resize(0);
    m_solverConstraints.reserve(m_faceConstraints.size());
    for (auto i = m_faceConstraints.begin(); i != m_faceConstraints.end(); i++)
    {
        if (m_constraintEnabled.count(i->second) != 0)
        {
            // Add pbd and rbd constraint
            m_solverConstraints.push_back(i->second.get());
            i->second->compute(needleObj->getRigidBodyModel2()->getTimeStep());
            needleObj->getRigidBodyModel2()->addConstraint(i->second);
        }
        else
        {
            i = m_faceConstraints.erase(i);
        }
    }
    m_solver->addCollisionConstraints(&m_solverConstraints);
}