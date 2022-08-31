/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleEmbeddedCH.h"
#include "EmbeddingConstraint.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTetrahedralMesh.h"

#include "imstkEntity.h"

using namespace imstk;

TissueData::TissueData(std::shared_ptr<PbdObject> obj) :
    obj(obj),
    geom(std::dynamic_pointer_cast<TetrahedralMesh>(obj->getPhysicsGeometry())),
    verticesPtr(geom->getVertexPositions()),
    vertices(*verticesPtr),
    indicesPtr(geom->getTetrahedraIndices()),
    indices(*indicesPtr)
{
}

NeedleData::NeedleData(std::shared_ptr<PbdObject> obj) :
    obj(obj),
    needle(obj->getComponent<StraightNeedle>()),
    verticesPtr(needle->getNeedleGeometry()->getVertexPositions()),
    vertices(*verticesPtr)
{
    needle->getNeedleGeometry()->updatePostTransformData();
}

std::shared_ptr<Geometry>
NeedleEmbeddedCH::getHandlingGeometryA()
{
    auto tissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    return (tissueObj == nullptr) ? nullptr : tissueObj->getPhysicsGeometry();
}

void
NeedleEmbeddedCH::addFaceEmbeddingConstraint(
    TissueData& tissueData, NeedleData& needleData,
    int v1, int v2, int v3, const Vec3d& iPt)
{
    // Hashable triangle (to resolve shared triangles, any order of v1,v2,v3 maps to same constraint)
    TriCell triCell(v1, v2, v3);

    m_debugEmbeddingPoints.push_back(iPt);
    m_debugEmbeddedTriangles.push_back(Vec3i(v1, v2, v3));

    // If constraint doesn't already exist for this triangle
    if (m_faceConstraints.count(triCell) == 0)
    {
        const int bodyId = tissueData.obj->getPbdBody()->bodyHandle;

        auto constraint = std::make_shared<EmbeddingConstraint>();

        constraint->initConstraint(
            tissueData.obj->getPbdModel()->getBodies(),
            { needleData.obj->getPbdBody()->bodyHandle, 0 },
            { bodyId, v1 }, { bodyId, v2 }, { bodyId, v3 },
            &needleData.vertices[0], &needleData.vertices[1], m_compliance);
        constraint->setFriction(m_friction);
        constraint->setRestitution(1.0);

        // Add the constraint to a map of face->constraint
        m_faceConstraints[triCell] = constraint;
    }

    // Mark as present
    m_constraintEnabled.insert(m_faceConstraints[triCell]);
}

void
NeedleEmbeddedCH::updatePunctureState(
    std::shared_ptr<Puncturable> puncturable,
    std::shared_ptr<StraightNeedle> needle, Vec3d force,
    bool notIntersecting)
{
    // If needle needle is touching the surface
    const PunctureId punctureId = getPunctureId(needle, puncturable);
    if (needle->getState(punctureId) == Puncture::State::TOUCHING)
    {
        // Get force along the needle axes
        const Vec3d  needleAxes = needle->getNeedleDirection();
        const double fN = std::max(needleAxes.dot(force), 0.0);

        // If the normal force exceeds the threshold, mark needle as inserted
        if (fN > m_forceThreshold)
        {
            LOG(INFO) << "Puncture!";
            needle->setState(punctureId, Puncture::State::INSERTED);
        }
    }

    // If the needle is inserted
    if (needle->getState(punctureId) == Puncture::State::INSERTED)
    {
        // Check if there are no tet intersections. If none, mark removed/unpunctured
        if (!notIntersecting)
        {
            LOG(INFO) << "Unpunctured!";
            needle->setState(punctureId, Puncture::State::REMOVED);
            m_faceConstraints.clear();
        }
    }
}

void
NeedleEmbeddedCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    auto tissueObj   = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    auto puncturable = tissueObj->getComponent<Puncturable>();
    auto needleObj   = std::dynamic_pointer_cast<PbdObject>(getInputObjectB());
    auto needle      = needleObj->getComponent<StraightNeedle>();

    TissueData tissueData(tissueObj);
    NeedleData needleData(needleObj);

    // Update the puncture state
    updatePunctureState(
        puncturable,
        needle,
        needleData.obj->getPbdBody()->externalForce,
        elementsA.size() != 0);

    if (!needle->getInserted())
    {
        return;
    }

    // Now we compute the embedding constraints for the system. These are the constraints
    // on the tool that keep the tool embedded in tissue, & vice versa.

    m_constraintEnabled.clear();

    // Constrain the triangle to the intersection point
    // If constraint for triangle already exists, update existing intersection point
    m_debugEmbeddingPoints.clear();
    m_debugEmbeddedTriangles.clear();

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
                tet = tissueData.indices[elemA.ids[0]];
            }
            else if (elemA.idCount == 4)
            {
                tet = Vec4i(elemA.ids[0], elemA.ids[1], elemA.ids[2], elemA.ids[3]);
            }

            std::array<Vec3d, 2> lineVerts = {
                needle->getNeedleStart(),
                needle->getNeedleEnd() };
            const double         lineLength = (lineVerts[1] - lineVerts[0]).norm();

            // For every face of the tet
            static int faces[4][3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 } };
            for (int j = 0; j < 4; j++)
            {
                // Find intersection point and add constraints
                const Vec3d& a = tissueData.vertices[tet[faces[j][0]]];
                const Vec3d& b = tissueData.vertices[tet[faces[j][1]]];
                const Vec3d& c = tissueData.vertices[tet[faces[j][2]]];

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
                            addFaceEmbeddingConstraint(
                                tissueData, needleData,
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

    // Add a singular frictional constraint
    //if (m_dynamicFriction != 0.0)
    /*{
        const Vec3d needleAxes = needleData.obj->getNeedleAxes();
        const Vec3d contactNormal = -needleAxes;
        auto        frictionConstraint = std::make_shared<RbdBidirectionalFrictionConstraint>(
            needleData.obj->getRigidBody(), nullptr,
            contactNormal,
            m_dynamicFriction,
            m_staticFrictionForceThreshold,
            RbdConstraint::Side::A);
        frictionConstraint->compute(needleData.obj->getRigidBodyModel2()->getTimeStep());
        needleData.obj->getRigidBodyModel2()->addConstraint(frictionConstraint);
    }*/

    // Add constraint to the PBD solver and RBD system
    m_constraints.resize(0);
    m_constraints.reserve(m_faceConstraints.size());
    for (auto i = m_faceConstraints.begin(); i != m_faceConstraints.end(); i++)
    {
        if (m_constraintEnabled.count(i->second) != 0)
        {
            // Add pbd constraint
            m_constraints.push_back(i->second.get());
        }
        else
        {
            i = m_faceConstraints.erase(i);
        }
    }
    tissueData.obj->getPbdModel()->getCollisionSolver()->addConstraints(&m_constraints);
}