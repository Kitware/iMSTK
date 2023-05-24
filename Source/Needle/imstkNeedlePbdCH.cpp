/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkNeedlePbdCH.h"
#include "imstkCollisionUtils.h"
#include "imstkCollisionData.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkPointwiseMap.h"
#include "imstkPuncturable.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <cmath>
#include "imstkPbdPointPointConstraint.h"
#include "imstkPbdDistanceConstraint.h"

// using namespace imstk;
namespace imstk
{
// Initialize interaction data
void
NeedlePbdCH::init(std::shared_ptr<PbdObject> threadObj)
{
    // Setup pbd tissue object
    m_pbdTissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());

    // Get surface mesh
    m_tissueSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_pbdTissueObj->getCollidingGeometry());

    // set up thread mesh
    m_threadObj  = threadObj;
    m_threadMesh = std::dynamic_pointer_cast<LineMesh>(m_threadObj->getCollidingGeometry());

    // Create storage for puncture states
    m_isThreadPunctured.resize(m_tissueSurfMesh->getNumCells());

    // Initialize to false
    for (int triangleId = 0; triangleId < m_tissueSurfMesh->getNumCells(); triangleId++)
    {
        m_isThreadPunctured[triangleId] = false;
    }
}

void
NeedlePbdCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)         //override
{
    auto needleObj  = std::dynamic_pointer_cast<PbdObject>(getInputObjectB());
    auto needleMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry());

    m_constraints.clear();
    /* Not working atm
    // Initialization order, don't know when the state gets set up
    if (m_particles.size() == 0)
    {
        for (int i = 0; i < 2; ++i)
        {
            m_particles.push_back(m_threadObj->getPbdModel()->addVirtualParticle(Vec3d::Zero(), -1.0, Vec3d::Zero(), true));
            auto c = std::make_shared<PbdDistanceConstraint>();
            auto threadBodyId = m_threadObj->getPbdBody()->bodyHandle;
            c->initConstraint(0.0, m_particles[i], { threadBodyId, i });
            m_threadConstraints.push_back(c);
        }
    }

    // Update Thread To Needle Constraints
    needleObj->getPbdModel()->getBodies().getPosition(m_particles[0]) = needleMesh->getVertexPositions()->at(1);
    needleObj->getPbdModel()->getBodies().getPosition(m_particles[1]) = needleMesh->getVertexPositions()->at(0);

    m_constraints.push_back(m_threadConstraints[0]);
    m_constraints.push_back(m_threadConstraints[1]);
    */

    (*m_threadMesh->getVertexPositions())[1] = (*needleMesh->getVertexPositions())[0];
    (*m_threadMesh->getVertexPositions())[0] = (*needleMesh->getVertexPositions())[1];

    // Handle collision normally if no insertion
    m_punctured = didPuncture(elementsA, elementsB) || m_punctured;
    if (!m_punctured)
    {
        PbdCollisionHandling::handle(elementsA, elementsB); // (PBD Object, Needle Object)
    }
    else
    {
        // Get rigid object needle
        auto                                     needle   = needleObj->getComponent<Needle>();
        const int                                needleId = needleObj->getID();
        std::shared_ptr<VecDataArray<double, 3>> needleVerticesPtr = needleMesh->getVertexPositions();
        VecDataArray<double, 3>&                 needleVertices    = *needleVerticesPtr;
        std::shared_ptr<VecDataArray<int, 2>>    needleIndicesPtr  = needleMesh->getCells();
        const VecDataArray<int, 2>&              needleIndices     = *needleIndicesPtr;
        // Save the direction of the tip of the needle. NOTE: Needle indices are backwards
        const Vec3d needleDirection = (needleVertices[35] - needleVertices[34]).normalized();

        // Create one to one map between the physics mesh and the surface mesh
        auto one2one = std::dynamic_pointer_cast<PointwiseMap>(m_pbdTissueObj->getPhysicsToCollidingMap());
        CHECK(one2one != nullptr) << "Failed to generate one to one map in NeedlePbdCH";

        auto                                  puncturable = m_pbdTissueObj->getComponent<Puncturable>();
        const int                             tissueId    = m_pbdTissueObj->getID();
        std::shared_ptr<VecDataArray<int, 3>> tissueSurfMeshIndicesPtr = m_tissueSurfMesh->getCells();
        const VecDataArray<int, 3>&           tissueSurfMeshIndices    = *tissueSurfMeshIndicesPtr;

        auto                                     physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdTissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = physMesh->getVertexPositions();
        const VecDataArray<double, 3>&           tissueVertices    = *tissueVerticesPtr;


        // Needle tail
        const Vec3d& needleTail1 = needleVertices[0];
        const Vec3d& needleTail2 = needleVertices[1];

        bool endSeg = false;
        int nearestSeg = -1;

        // Scope for needle
        {
            // First, find new penetration points using the tip of the needle (needle mesh is reversed)
            int tipSegmentId = needleMesh->getNumCells() - 1;

            Vec2i       nodeIds = needleIndices[tipSegmentId];
            const Vec3d tip1    = needleVertices[nodeIds[0]];
            const Vec3d tip2    = needleVertices[nodeIds[1]];

            // For every triangle, check if segment is in triangle (if so, puncture)
            for (int triangleId = 0; triangleId < m_tissueSurfMesh->getNumCells(); triangleId++)
            {
                const Vec3i& surfTriIds = tissueSurfMeshIndices[triangleId];

                // Indices of the vertices on the physics mesh (which could be a tet mesh)
                Vec3i physTriIds;
                physTriIds[0] = one2one->getParentVertexId(surfTriIds[0]);
                physTriIds[1] = one2one->getParentVertexId(surfTriIds[1]);
                physTriIds[2] = one2one->getParentVertexId(surfTriIds[2]);

                const Vec3d& a = tissueVertices[physTriIds[0]];
                const Vec3d& b = tissueVertices[physTriIds[1]];
                const Vec3d& c = tissueVertices[physTriIds[2]];

                // Barycentric coordinates of intersection point
                Vec3d uvw = Vec3d::Zero();

                // If this triangle has not already been punctured
                const PunctureId punctureId = getPunctureId(needle, puncturable, triangleId);
                if (needle->getState(punctureId) != Puncture::State::INSERTED)
                {
                    // Check for intersection
                    if (CollisionUtils::testSegmentTriangle(tip1, tip2, a, b, c, uvw))
                    {
                        needle->setState(punctureId, Puncture::State::INSERTED);

                        // Save the puncture point
                        Puncture& data = *needle->getPuncture(punctureId);
                        data.userData.id         = triangleId;
                        data.userData.ids[0]     = physTriIds[0];
                        data.userData.ids[1]     = physTriIds[1];
                        data.userData.ids[2]     = physTriIds[2];
                        data.userData.weights[0] = uvw[0];
                        data.userData.weights[1] = uvw[1];
                        data.userData.weights[2] = uvw[2];

                        PenetrationData newPuncture;

                        newPuncture.triId = triangleId;

                        newPuncture.triVertIds[0] = physTriIds[0];
                        newPuncture.triVertIds[1] = physTriIds[1];
                        newPuncture.triVertIds[2] = physTriIds[2];

                        newPuncture.triBaryPuncturePoint[0] = uvw[0];
                        newPuncture.triBaryPuncturePoint[1] = uvw[1];
                        newPuncture.triBaryPuncturePoint[2] = uvw[2];

                        m_needlePData.push_back(newPuncture);

                        LOG(DEBUG) << "Needle punctured triangle: " << triangleId;
                    }
                }
            }

            // Loop over penetration points and find nearest point on the needle
            // Note: Nearest point will likely be the point between two segments,
            // its dualy defined, but thats ok
            
            for (auto puncture : needle->getPunctures())
            {
                if (puncture.second->state != Puncture::State::INSERTED)
                {
                    continue;
                }

                // Start with large value
                Vec3d  closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };
                double closestDist  = IMSTK_DOUBLE_MAX;

                Puncture::UserData& punctureData = puncture.second->userData;
                const Vec3d&        a = tissueVertices[punctureData.ids[0]];
                const Vec3d&        b = tissueVertices[punctureData.ids[1]];
                const Vec3d&        c = tissueVertices[punctureData.ids[2]];

                const Vec3d baryPoint  = punctureData.weights.head<3>();
                const Vec3d puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

                
                for (int segmentId = 0; segmentId < needleMesh->getNumCells(); segmentId++)
                {
                    const Vec2i& needleSegNodeIds = needleIndices[segmentId];
                    const Vec3d& x1 = needleVertices[needleSegNodeIds[0]];
                    const Vec3d& x2 = needleVertices[needleSegNodeIds[1]];

                    int caseType = -1;

                    // Find the closest point on this segment
                    const Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

                    double newDist = (segClosestPoint - puncturePt).squaredNorm();
                    if (newDist < closestDist)
                    {
                        closestDist  = newDist;
                        closestPoint = segClosestPoint;
                        nearestSeg = segmentId;
                       
                    }
                }
                if (nearestSeg == 0) {
                    endSeg = true;
                    // LOG(INFO) << "Puncture thread now";
                }

                // Check and see if the closest point is at the tips of the needle
                // Note: Needle mesh is backwards
                Vec3d diffTail = closestPoint - needleVertices[0];
                Vec3d diffTip  = closestPoint - needleVertices[needleMesh->getNumVertices() - 1];

                // If the closest point is sufficiently close to the tip or tail then unpuncture can occur
                const double unpunctureEpsilon = 1e-8;
                if (diffTail.norm() < unpunctureEpsilon || diffTip.norm() < unpunctureEpsilon)
                {
                    puncture.second->state = Puncture::State::REMOVED;
                    continue;
                }


                // Now that we have the closest point on the needle to this penetration point, we can
                // generate and solve the constraint

                const int bodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
                const int needleBodyId = needleObj->getPbdBody()->bodyHandle;
                auto      pointTriangleConstraint = std::make_shared<SurfaceInsertionConstraint>();
                pointTriangleConstraint->initConstraint(puncturePt,
                    { needleBodyId, 0 },
                    { bodyId, punctureData.ids[0] },
                    { bodyId, punctureData.ids[1] },
                    { bodyId, punctureData.ids[2] },
                    closestPoint,
                    baryPoint,
                    m_needleToSurfaceStiffness, m_surfaceToNeedleStiffness // stiffness parameters
                );
                m_constraints.push_back(pointTriangleConstraint);
            }
        }

        // Scope for thread
        {
            // Now on to the thread!!!
            // use the tip of the the same way as the tip of the needle
            // to set up thread penetration points

            // Get thread vertex positions
            std::shared_ptr<VecDataArray<double, 3>> threadVerticesPtr = m_threadMesh->getVertexPositions();
            const VecDataArray<double, 3>&           threadVertices    = *threadVerticesPtr;
            std::shared_ptr<VecDataArray<int, 2>>    threadIndcicesPtr = m_threadMesh->getCells();
            const VecDataArray<int, 2>&              threadIndices     = *threadIndcicesPtr;

            Vec3d threadTip = threadVertices[0];

            // First, find new penetration points
            const Vec2i& nodeIds    = threadIndices[0];
            const Vec3d& threadTip1 = threadVertices[nodeIds[0]];
            const Vec3d& threadTip2 = threadVertices[nodeIds[1]];

            // Loop over all triangles in the surface mesh
            // for (int triangleId = 0; triangleId < m_tissueSurfMesh->getNumCells(); triangleId++)
            for (int punctureId = 0; punctureId < m_needlePData.size(); punctureId++)
            {
                int triangleId = m_needlePData[punctureId].triId;
                const Vec3i& surfTriIds = tissueSurfMeshIndices[triangleId];

                // Indices of the vertices on the physics mesh
                Vec3i physTriIds;
                physTriIds[0] = one2one->getParentVertexId(surfTriIds[0]);
                physTriIds[1] = one2one->getParentVertexId(surfTriIds[1]);
                physTriIds[2] = one2one->getParentVertexId(surfTriIds[2]);

                const Vec3d& a = tissueVertices[physTriIds[0]];
                const Vec3d& b = tissueVertices[physTriIds[1]];
                const Vec3d& c = tissueVertices[physTriIds[2]];

                // Barycentric coordinates of interseciton point
                Vec3d uvw = Vec3d::Zero();

                // If this triangle has already been punctured by the needle
                if (needle->getState({ tissueId, needleId, triangleId }) == Puncture::State::INSERTED)
                {
                    // If this triangle has not yet been punctured by thread
                    if (m_isThreadPunctured[triangleId] == false)
                    {
                        // Check for intersection
                        if (endSeg == true) // CollisionUtils::testSegmentTriangle(needleTail1, needleTail2, a, b, c, uvw)
                        {
                            endSeg = false;
                            m_isThreadPunctured[triangleId] = true;

                            // Find matching needle puncture point
                            Puncture::UserData punctureData;
                            for (auto puncture : needle->getPunctures())
                            {
                                if (std::get<2>(puncture.first) == triangleId)
                                {
                                    punctureData = puncture.second->userData;
                                }
                            }

                            // Create thread puncture point and copy in data from needle puncture point
                            PenetrationData newPuncture;

                            newPuncture.triId = punctureData.id;

                            newPuncture.triVertIds[0] = punctureData.ids[0];
                            newPuncture.triVertIds[1] = punctureData.ids[1];
                            newPuncture.triVertIds[2] = punctureData.ids[2];

                            newPuncture.triBaryPuncturePoint[0] = punctureData.weights[0];
                            newPuncture.triBaryPuncturePoint[1] = punctureData.weights[1];
                            newPuncture.triBaryPuncturePoint[2] = punctureData.weights[2];



                            // newPuncture.triBaryPuncturePoint = uvw;

                            // Create and save the puncture point
                            m_threadPData.push_back(newPuncture);

                            LOG(DEBUG) << "Thread punctured triangle: " << triangleId;
                        }
                    }
                }
            }

            // Loop over thread penetration points and find nearest point on the thread
            // Note: Nearest point will likely be the point between two segments, its dualy defined
            for (int pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
            {
                // Start with arbitrary large value
                Vec3d closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };

                const Vec3d& a = tissueVertices[m_threadPData[pPointId].triVertIds[0]];
                const Vec3d& b = tissueVertices[m_threadPData[pPointId].triVertIds[1]];
                const Vec3d& c = tissueVertices[m_threadPData[pPointId].triVertIds[2]];

                Vec3d baryPoint = m_threadPData[pPointId].triBaryPuncturePoint;

                auto puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

                int closestSegmentId = -1;

                // Note: stopping before last segment for visualization
                for (int segmentId = 0; segmentId < m_threadMesh->getNumCells() - 1; segmentId++)
                {
                    const Vec2i& threadSegNodeIds = threadIndices[segmentId];
                    const Vec3d& x1 = threadVertices[threadSegNodeIds[0]];
                    const Vec3d& x2 = threadVertices[threadSegNodeIds[1]];

                    int caseType = -1;

                    const Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

                    const Vec3d newDist = segClosestPoint - puncturePt;
                    const Vec3d oldDist = closestPoint - puncturePt;

                    if (newDist.norm() <= oldDist.norm())
                    {
                        closestPoint     = segClosestPoint;
                        closestSegmentId = segmentId;
                    }
                } // end loop over thread segments

                // Check and see if the closest point is at the tips of the thread
                const Vec3d diffTip  = closestPoint - threadVertices[0];
                const Vec3d diffTail = closestPoint - threadVertices[m_threadMesh->getNumVertices() - 1];

                // NOTE: Commented out to force thread to stay inserted once inserted
                // If uncommented, the thread would be able to slide through the mesh and unpuncture.
                // Unpuncture if thread moves past last segment of the thread
                // const double unpunctureEpsilon = 1e-8;
                //if (diffTail.norm() < unpunctureEpsilon || diffTip.norm() < unpunctureEpsilon) {

                //    // If the tip of the needle has been removed,
                //    // this triangle is no longer punctured
                //    if (diffTail.norm() < unpunctureEpsilon) {
                //        m_isNeedlePunctured[m_threadPData[pPointId].triId] = false;
                //    }
                //
                //    m_threadPData.erase(m_threadPData.begin() + pPointId);
                //    continue;
                //}

                // Now that we have the closest point on the thread to this penetration point, we can
                // generate and solve the constraint
                auto threadTriangleConstraint = std::make_shared<ThreadInsertionConstraint>();

                // Set of VM pairs for thread
                const Vec2i& nearestSegNodeIds = threadIndices[closestSegmentId];
                const Vec3d& p = threadVertices[nearestSegNodeIds[0]];
                const Vec3d& q = threadVertices[nearestSegNodeIds[1]];

                // Thread barycentric intersection point
                const Vec2d segBary = baryCentric(closestPoint, p, q);

                const int tissueBodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
                const int threadBodyId = m_threadObj->getPbdBody()->bodyHandle;
                threadTriangleConstraint->initConstraint(
                m_pbdTissueObj->getPbdModel()->getBodies(),
                    { threadBodyId, nearestSegNodeIds[0] },
                    { threadBodyId, nearestSegNodeIds[1] },
                    segBary,
                    { tissueBodyId, m_threadPData[pPointId].triVertIds[0] },
                    { tissueBodyId, m_threadPData[pPointId].triVertIds[1] },
                    { tissueBodyId, m_threadPData[pPointId].triVertIds[2] },
                m_threadPData[pPointId].triBaryPuncturePoint,
                m_threadToSurfaceStiffness, m_surfaceToThreadStiffness); // Tissue is not currently moved
                m_constraints.push_back(threadTriangleConstraint);
            }
        }

        // Solve stitching constraint
        if (m_stitch)
        {
            for (size_t i = 0; i < m_stitchConstraints.size(); i++)
            {
                m_constraints.push_back(m_stitchConstraints[i]);
            }
        }

        // If there are no penetration points, the needle is removed
        if (!needle->getInserted() && m_threadPData.size() == 0)
        {
            LOG(INFO) << "Needle removed";
            m_punctured = false;
        }
    }

    m_solverConstraints.resize(m_constraints.size());
    for (int i = 0; i < m_constraints.size(); i++)
    {
        m_solverConstraints[i] = m_constraints[i].get();
    }
    m_pbdTissueObj->getPbdModel()->getSolver()->addConstraints(&m_solverConstraints);
}

// Create stitching constraints
void
NeedlePbdCH::stitch()
{
    // First, verify that at least 4 points have been penetrated by the thread
    if (m_threadPData.size() < 4)
    {
        LOG(INFO) << "Cant stitch less than 4 points, currently only "<< m_threadPData.size()<<" points";
        return;
    }

    if (m_threadPData.size() >= 4)
    {
        LOG(INFO) << "Stitching!";

        std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = m_tissueSurfMesh->getVertexPositions();
        VecDataArray<double, 3>&                 tissueVertices    = *tissueVerticesPtr;

        // Only calculate the center point once
        if (m_stitch == false)
        {
            // Calculate the average position of the points punctured by thread
            for (size_t pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
            {
                Vec3d pPoint = tissueVertices[m_threadPData[pPointId].triVertIds[0]] * m_threadPData[pPointId].triBaryPuncturePoint[0]
                               + tissueVertices[m_threadPData[pPointId].triVertIds[1]] * m_threadPData[pPointId].triBaryPuncturePoint[1]
                               + tissueVertices[m_threadPData[pPointId].triVertIds[2]] * m_threadPData[pPointId].triBaryPuncturePoint[2];

                for (int i = 0; i < 3; i++)
                {
                    m_stitchCenter[i] += pPoint[i] / static_cast<double>(m_threadPData.size());
                }
            }
        }

        m_stitch = true;

        // Create constraints to pull the puncture points to the center location
        for (int pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
        {
            // Now create values for the central point
            const PbdParticleId& stitchCenterPt = m_pbdTissueObj->getPbdModel()->addVirtualParticle(m_stitchCenter, 0.0);

            const int           bodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
            const PbdParticleId p0     = { bodyId, m_threadPData[pPointId].triVertIds[0] };
            const PbdParticleId p1     = { bodyId, m_threadPData[pPointId].triVertIds[1] };
            const PbdParticleId p2     = { bodyId, m_threadPData[pPointId].triVertIds[2] };

            auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
            constraint->initConstraint(
                { p0, p1, p2 },
                { m_threadPData[pPointId].triBaryPuncturePoint[0],
                  m_threadPData[pPointId].triBaryPuncturePoint[1],
                  m_threadPData[pPointId].triBaryPuncturePoint[2] },
                { stitchCenterPt }, { 1.0 },
                    0.2, 0.0);

            // Add to list of constraints to be solved together in the handler
            m_stitchConstraints.push_back(constraint);
        }
    }
}

bool
NeedlePbdCH::didPuncture(const std::vector<CollisionElement>& elementsA, const std::vector<CollisionElement>& elementsB)
{
    // Pack all the data needed for a particular side into a struct so we can
    // swap it with the contact & pass it around
    CollisionSideData tissueData = getDataFromObject(getInputObjectA());
    CollisionSideData needleData = getDataFromObject(getInputObjectB());

    // Old code expects the needle to be object A (and therefor elementsA)
    // and the mesh to be object B (and therefor elementsB)
    std::shared_ptr<CollidingObject> tissueObj   = getInputObjectA();
    auto                             puncturable = tissueObj->getComponent<Puncturable>();
    std::shared_ptr<CollidingObject> needleObj   = getInputObjectB();
    auto                             needle      = needleObj->getComponent<Needle>();

    CHECK(tissueObj != nullptr) << "NeedlePbdCH: tissueObj is null";
    CHECK(needleObj != nullptr) << "NeedlePbdCH: needleObj is null";
    CHECK(puncturable != nullptr) << "NeedlePbdCH: puncturable is null";
    CHECK(needle != nullptr) << "NeedlePbdCH: needle is null";

    CHECK(elementsA.size() == elementsB.size()) << "Number of elements in A and B must be the same";
    if (elementsA.empty())
    {
        return false;
    }

    // Check for ordering expecting triangle indices in either A or B
    ;

    auto* needleElements = &elementsB;
    auto* tissueElements = &elementsA;

    for (int i = 0; i < needleElements->size(); ++i)
    {
        auto needleElement = needleElements->at(i);
        auto tissueElement = tissueElements->at(i);

        CHECK(tissueElement.m_type == CollisionElementType::CellIndex)
            << "Suturing only works with CDs that report CellIndex contact";
        CHECK(tissueElement.m_element.m_CellIndexElement.parentId != -1)
            << "Suturing only works with CDs that report parent ids";
        // if (deformableElement.m_type != CollisionElementType::CellIndex || deformableElement.m_element.m_CellIndexElement.parentId == -1) continue;

        const PunctureId punctureId = getPunctureId(needle, puncturable, tissueElement.m_element.m_CellIndexElement.parentId);

        // If removed and we are here, we must now be touching
        if (needle->getState(punctureId) == Puncture::State::REMOVED) // Removed
        {
            needle->setState(punctureId, Puncture::State::TOUCHING);
            puncturable->setPuncture(punctureId, needle->getPuncture(punctureId));
        }

        auto                                     needleMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry());
        std::shared_ptr<VecDataArray<double, 3>> needleVerticesPtr = needleMesh->getVertexPositions();
        VecDataArray<double, 3>&                 needleVertices    = *needleVerticesPtr;
        // Save the direction of the tip of the needle. NOTE: Needle indices are backwards
        int         endIndex = needleVertices.size() - 1;
        const Vec3d needleDirection = (needleVertices[endIndex] - needleVertices[endIndex - 1]).normalized();

        // If touching we may test for insertion
        // Calculate the surface normal using the set of vertices associated with the triangle and normalize
        // use dot product to project onto the needle stabbing direction, if close to 1 assume its inserted
        // Possibly add contact time or pseudo force calculation to know if penetration occurs

        // Note: assumes closed mesh

        // Assuming triangle has points a,b,c
        if (tissueElement.m_element.m_CellIndexElement.cellType != IMSTK_TRIANGLE)
        {
            continue;
        }
        std::array<PbdParticleId, 3> ptsB   = PbdCollisionHandling::getTriangle(tissueElement, tissueData);
        const PbdState&              bodies = m_pbdTissueObj->getPbdModel()->getBodies();
        const Vec3d                  ab     = bodies.getPosition(ptsB[1]) - bodies.getPosition(ptsB[0]);
        const Vec3d                  ac     = bodies.getPosition(ptsB[2]) - bodies.getPosition(ptsB[0]);

        // Calculate surface normal
        const Vec3d surfNormal = ac.cross(ab).normalized();

        // Get vector pointing in direction of needle
        // Use absolute value to ignore direction issues
        const double dotProduct = std::fabs(needleDirection.dot(surfNormal));

        // Arbitrary threshold
        const double threshold = 0.8;

        if (needle->getState(punctureId) == Puncture::State::TOUCHING) // Touching
        {
            // If the needle is close to perpendicular to the face if may insert
            // Note: This is a short term solution
            if (dotProduct > threshold)
            {
                // LOG(INFO) << "Needle inserted";
                return true;
            }
        }
    }
    return false;
}
} // namespace imstk