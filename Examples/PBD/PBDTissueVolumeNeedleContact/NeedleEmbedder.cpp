/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleEmbedder.h"
#include "EmbeddingConstraint.h"
#include "imstkCollisionData.h"
#include "imstkLineMesh.h"
#include "imstkPbdModel.h"
#include "imstkPbdSolver.h"
#include "imstkTaskNode.h"
#include "imstkTetrahedralMesh.h"
#include "NeedleObject.h"

using namespace imstk;

static bool
testPlaneLine2(const Vec3d& p, const Vec3d& q,
               const Vec3d& planePt, const Vec3d& planeNormal, Vec3d& iPt,
               double& t)
{
    const Vec3d  n     = q - p;
    const double denom = n.dot(planeNormal);
    // Plane and line are parallel
    if (std::abs(denom) < IMSTK_DOUBLE_EPS)
    {
        return false;
    }
    // const Vec3d  dir = n.normalized();
    t   = (planePt - p).dot(planeNormal) / denom;
    iPt = p + t * n;
}

///
/// \brief Similar to other CollisionUtils functions but provides uvw
/// from ray intersection even when the line misses the triangle (given axes vs plane)
///
static bool
testSegmentTriangle2(const Vec3d& p, const Vec3d& q,
                     const Vec3d& a, const Vec3d& b, const Vec3d& c, Vec3d& uvw,
                     bool& segmentInPlane)
{
    segmentInPlane = false;
    Vec3d  iPt;
    double t;
    if (testPlaneLine2(p, q, a, (b - a).cross(c - a).normalized(), iPt, t))
    {
        uvw = baryCentric(iPt, a, b, c);
        if (t > 0.0 && t < 1.0)
        {
            segmentInPlane = true;
            if (uvw[0] >= 0.0 && uvw[1] >= 0.0 && uvw[2] >= 0.0)
            {
                return true;
            }
        }
    }
    return false;
}

TissueData::TissueData(std::shared_ptr<PbdObject> obj) :
    obj(obj),
    geom(std::dynamic_pointer_cast<TetrahedralMesh>(obj->getPhysicsGeometry())),
    verticesPtr(geom->getVertexPositions()),
    vertices(*verticesPtr),
    //prevVerticesPtr(obj->getPbdBody()->prevVertices),
    //prevVertices(*prevVerticesPtr),
    indicesPtr(geom->getCells()),
    indices(*indicesPtr)
{
}

NeedleData::NeedleData(std::shared_ptr<NeedleObject> obj) :
    obj(obj),
    geom(std::dynamic_pointer_cast<LineMesh>(obj->getCollidingGeometry())),
    verticesPtr(geom->getVertexPositions()),
    vertices(*verticesPtr),
    //prevVertices(VecDataArray<double, 3>(vertices.size())),
    indicesPtr(geom->getCells()),
    indices(*indicesPtr)
{
    geom->updatePostTransformData();

    const Vec3d bodyPos = (*obj->getPbdBody()->vertices)[0];
    const Quatd bodyOrientation     = (*obj->getPbdBody()->orientations)[0];
    const Vec3d bodyPrevPos         = (*obj->getPbdBody()->prevVertices)[0];
    const Quatd bodyPrevOrientation = (*obj->getPbdBody()->prevOrientations)[0];

    // From the needle rigid body data compute the vertex velocities
    //for (int i = 0; i < vertices.size(); i++)
    //{
    //    const Vec3d r = vertices[i] - bodyPos;
    //    velocities[i] = (*obj->getPbdBody()->velocities)[0] +
    //                    (*obj->getPbdBody()->angularVelocities)[0].cross(r);
    //    //const Vec3d restR = bodyOrientation.inverse()._transformVector(r);
    //    //prevVertices[i] = bodyPrevPos + bodyPrevOrientation._transformVector(restR);
    //}
}

void
NeedleEmbedder::addFaceEmbeddingConstraint(
    TissueData& tissueData, NeedleData& needleData,
    int v1, int v2, int v3, const Vec3d& iPt)
{
    // Hashable triangle (to resolve shared triangles, any order of v1,v2,v3 maps to same constraint)
    TriCell triCell(v1, v2, v3);

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
        // Constraint acts along needle perpendicular
        constraint->setFriction(m_friction);
        constraint->setRestitution(1.0);

        // Add the constraint to a map of face->constraint
        m_faceConstraints[triCell] = constraint;
    }
}

void
NeedleEmbedder::update()
{
    TissueData tissueData(m_tissueObject);
    NeedleData needleData(std::dynamic_pointer_cast<NeedleObject>(m_needleObject));

    // If collision elements are present transition to touching
    if ((m_cdData->elementsA.size() > 0 || m_cdData->elementsB.size() > 0)
        && needleData.obj->getCollisionState(tissueData.obj) == NeedleObject::CollisionState::REMOVED)
    {
        needleData.obj->setCollisionState(tissueData.obj, NeedleObject::CollisionState::TOUCHING);
    }

    // If needle needle is touching the surface then test for puncture/insertion
    if (needleData.obj->getCollisionState(tissueData.obj) == NeedleObject::CollisionState::TOUCHING)
    {
        // Get force along the needle axes
        const Vec3d  needleAxes = needleData.obj->getNeedleAxes();
        const double fN = std::max(needleAxes.dot(needleData.obj->getPbdBody()->externalForce), 0.0);

        // If the normal force exceeds the threshold, mark needle as inserted
        if (fN > m_forceThreshold)
        {
            // Disable collision handling if needle is inside
            needleData.obj->setCollisionState(tissueData.obj, NeedleObject::CollisionState::INSERTED);
            m_pbdCHNode->setEnabled(false);
        }
    }

    // Debug points and triangles for visualization
    m_debugEmbeddingPoints.clear();
    m_debugEmbeddedTriangles.clear();

    if (needleData.obj->getCollisionState(tissueData.obj) == NeedleObject::CollisionState::INSERTED)
    {
        // To "enter" a triangle we must be previously above it and then below it
        // To "exit" a triangle the same is true but we need to be "inside" the triangle

        // For culling we compute the sphere bounding the tet, then compute whether this
        // sphere intersects the line.
        // Note: This is fastest for a single straight large line vs many tets. It should
        // be noted that a full sphere sweep (ie: bound line with sphere) would cause a giant
        // sphere for the line.

        const double dt = m_needleObject->getPbdModel()->getTimeStep();
        for (int i = 0; i < needleData.indices.size(); i++)
        {
            const Vec2i& seg = needleData.indices[i];

            const Vec3d& line_x0 = needleData.vertices[seg[0]];
            const Vec3d& line_x1 = needleData.vertices[seg[1]];

            const Vec3d& prev_line_x0 = needlePrevVertices[seg[0]];
            const Vec3d& prev_line_x1 = needlePrevVertices[seg[1]];

            const Vec3d diff = line_x1 - line_x0;
            const Vec3d axes = diff.normalized();

            for (int j = 0; j < tissueData.indices.size(); j++)
            {
                const Vec4i& tet = tissueData.indices[j];

                // Compute bounding sphere for tet
                const Vec3d& tet_x0 = tissueData.vertices[tet[0]];
                const Vec3d& tet_x1 = tissueData.vertices[tet[1]];
                const Vec3d& tet_x2 = tissueData.vertices[tet[2]];
                const Vec3d& tet_x3 = tissueData.vertices[tet[3]];
                const Vec3d  center = (tet_x0 + tet_x1 + tet_x2 + tet_x3) * 0.25;

                // Find the max distance from the center
                const double tetSphereSqrDist =
                    std::max((tissueData.vertices[tet[0]] - center).squaredNorm(),
                    std::max((tissueData.vertices[tet[1]] - center).squaredNorm(),
                    std::max((tissueData.vertices[tet[2]] - center).squaredNorm(),
                                (tissueData.vertices[tet[3]] - center).squaredNorm())));

                // Compute distance to line axes
                const Vec3d  diffCenter = center - line_x0;
                const double sqrDistCenterToAxes = (diffCenter - diffCenter.dot(axes) * axes).squaredNorm();
                // Slight increase in size to account for movement (this imposes a speed limit since we
                // are checking intersection with both previous and current)
                if (sqrDistCenterToAxes <= tetSphereSqrDist * 2.0)
                {
                    // For every face of the tet
                    int faces[4][3] = {
                        { tet[0], tet[1], tet[2] },
                        { tet[1], tet[2], tet[3] },
                        { tet[0], tet[2], tet[3] },
                        { tet[0], tet[1], tet[3] } };
                    for (int k = 0; k < 4; k++)
                    {
                        const Vec3d& tri_x0 = tissueData.vertices[faces[k][0]];
                        const Vec3d& tri_x1 = tissueData.vertices[faces[k][1]];
                        const Vec3d& tri_x2 = tissueData.vertices[faces[k][2]];

                        bool       currInPlane       = false;
                        Vec3d      curr_uvw          = Vec3d::Zero();
                        const bool isCurrIntersected = testSegmentTriangle2(
                            line_x0, line_x1,
                            tri_x0, tri_x1, tri_x2,
                            curr_uvw, currInPlane);

                        // If currently intersecting
                        if (isCurrIntersected)
                        {
                            // Check if the previous planar intersection of the axes of the line
                            // was above the triangle
                            const Vec3d& prev_tri_x0 = tissuePrevVertices[faces[k][0]];
                            const Vec3d& prev_tri_x1 = tissuePrevVertices[faces[k][1]];
                            const Vec3d& prev_tri_x2 = tissuePrevVertices[faces[k][2]];

                            Vec3d      prev_uvw          = Vec3d::Zero();
                            bool       prevInPlane       = false;
                            const bool isPrevIntersected = testSegmentTriangle2(
                                prev_line_x0, prev_line_x1,
                                prev_tri_x0, prev_tri_x1, prev_tri_x2,
                                prev_uvw, prevInPlane);

                            // If previous in triangle it must be entering correctly
                            if (prev_uvw[0] >= 0.0 && prev_uvw[1] >= 0.0 && prev_uvw[2] >= 0.0)
                            {
                                addFaceEmbeddingConstraint(
                                    tissueData, needleData,
                                    faces[k][0],
                                    faces[k][1],
                                    faces[k][2],
                                    curr_uvw[0] * tri_x0 + curr_uvw[1] * tri_x1 + curr_uvw[2] * tri_x2);
                            }
                        }
                    }
                }
            }
        }

        // Add constraint to the PBD solver and RBD system
        m_constraints.resize(0);
        m_constraints.reserve(m_faceConstraints.size());

        // Check all existing constraints too see if they exited
        std::vector<TriCell> constraintsToDelete;
        for (auto i = m_faceConstraints.begin(); i != m_faceConstraints.end(); i++)
        {
            const std::vector<PbdParticleId>& particles = i->second->getParticles();
            const Vec3d&                      tri_x0    = tissueData.vertices[particles[0].second];
            const Vec3d&                      tri_x1    = tissueData.vertices[particles[1].second];
            const Vec3d&                      tri_x2    = tissueData.vertices[particles[2].second];

            bool       currInPlane       = false;
            Vec3d      curr_uvw          = Vec3d::Zero();
            const bool isCurrIntersected = testSegmentTriangle2(
                *i->second->getP(), *i->second->getQ(),
                tri_x0, tri_x1, tri_x2,
                curr_uvw, currInPlane);

            // If exited/no longer touching triangle
            if (!currInPlane)
            {
                constraintsToDelete.push_back(TriCell(particles[0].second, particles[1].second, particles[2].second));
            }
        }
        for (auto i : constraintsToDelete)
        {
            m_faceConstraints.erase(i);
        }

        for (auto i = m_faceConstraints.begin(); i != m_faceConstraints.end(); i++)
        {
            const std::vector<PbdParticleId>& particles = i->second->getParticles();
            m_debugEmbeddingPoints.push_back(i->second->getIntersectionPoint());
            m_debugEmbeddedTriangles.push_back(Vec3i(particles[0].second, particles[1].second, particles[2].second));

            // Add pbd constraint
            m_constraints.push_back(i->second.get());
        }
        if (m_constraints.size() == 0)
        {
            needleData.obj->setCollisionState(tissueData.obj, NeedleObject::CollisionState::REMOVED);
            m_pbdCHNode->setEnabled(true);
            //LOG(INFO) << "Unpunctured!";
        }
        tissueData.obj->getPbdModel()->getCollisionSolver()->addConstraints(&m_constraints);
    }

    // Stash the vertices at this point
    tissuePrevVertices.resize(tissueData.vertices.size());
    std::copy(tissueData.vertices.begin(), tissueData.vertices.end(), tissuePrevVertices.begin());
    needlePrevVertices.resize(needleData.vertices.size());
    std::copy(needleData.vertices.begin(), needleData.vertices.end(), needlePrevVertices.begin());
}