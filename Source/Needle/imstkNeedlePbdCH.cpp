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
#include "imstkPbdContactConstraint.h"

#include <iostream>

// using namespace imstk;
namespace imstk
{
// Initialize interaction data
void
NeedlePbdCH::init(std::shared_ptr<PbdObject> threadObj)
{
    // Setup pbd tissue object
    m_pbdTissueObj   = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    m_tissueSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_pbdTissueObj->getCollidingGeometry());

    // Set up needle object
    m_needleObj  = std::dynamic_pointer_cast<PbdObject>(getInputObjectB());
    m_needleMesh = std::dynamic_pointer_cast<LineMesh>(m_needleObj->getCollidingGeometry());

    // set up thread mesh
    m_threadObj  = threadObj;
    m_threadMesh = std::dynamic_pointer_cast<LineMesh>(m_threadObj->getCollidingGeometry());
}

void
NeedlePbdCH::generateNewPunctureData()
{
    // Unpack needle and tissue data
    auto needle      = m_needleObj->getComponent<Needle>();
    auto puncturable = m_pbdTissueObj->getComponent<Puncturable>();

    // Get one to one map between the physics mesh and the surface mesh
    auto one2one = std::dynamic_pointer_cast<PointwiseMap>(m_pbdTissueObj->getPhysicsToCollidingMap());
    CHECK(one2one != nullptr) << "No one to one map in NeedlePbdCH for tissue to surface";

    auto physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdTissueObj->getPhysicsGeometry());

    // First, find new penetration points using the tip of the needle (needle mesh is reversed)
    int tipSegmentId = m_needleMesh->getNumCells() - 1;

    Vec2i       nodeIds = m_needleMesh->getCells()->at(tipSegmentId);
    const Vec3d tip1    = m_needleMesh->getVertexPositions()->at(nodeIds[0]);
    const Vec3d tip2    = m_needleMesh->getVertexPositions()->at(nodeIds[1]);

    // For every triangle, check if segment is in triangle (if so, puncture)
    for (int triangleId = 0; triangleId < m_tissueSurfMesh->getNumCells(); triangleId++)
    {
        const Vec3i& surfTriIds = m_tissueSurfMesh->getCells()->at(triangleId);

        // Indices of the vertices on the physics mesh (which could be a tet mesh)
        Vec3i physTriIds;
        physTriIds[0] = one2one->getParentVertexId(surfTriIds[0]);
        physTriIds[1] = one2one->getParentVertexId(surfTriIds[1]);
        physTriIds[2] = one2one->getParentVertexId(surfTriIds[2]);

        const Vec3d& a = physMesh->getVertexPositions()->at(physTriIds[0]);
        const Vec3d& b = physMesh->getVertexPositions()->at(physTriIds[1]);
        const Vec3d& c = physMesh->getVertexPositions()->at(physTriIds[2]);

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

                // Save the puncture data to the needle
                Puncture& data = *needle->getPuncture(punctureId);
                data.userData.id         = triangleId;
                data.userData.ids[0]     = physTriIds[0];
                data.userData.ids[1]     = physTriIds[1];
                data.userData.ids[2]     = physTriIds[2];
                data.userData.weights[0] = uvw[0];
                data.userData.weights[1] = uvw[1];
                data.userData.weights[2] = uvw[2];

                // Create penetration data for constraints
                PuncturePoint newPuncture;

                newPuncture.triId      = triangleId;
                newPuncture.triVertIds = physTriIds;
                newPuncture.baryCoords = uvw;
                newPuncture.segId      = tipSegmentId;

                pData.needle.push_back(newPuncture);

                m_needlePunctured = true;
                LOG(DEBUG) << "Needle punctured triangle: " << triangleId;
            }
        }
    }
}

void
NeedlePbdCH::addPunctureConstraints()
{
    // Unpack needle and tissue data
    auto needle      = m_needleObj->getComponent<Needle>();
    auto physMesh    = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdTissueObj->getPhysicsGeometry());
    auto puncturable = m_pbdTissueObj->getComponent<Puncturable>();

    // Loop over penetration points and find nearest point on the needle
    // Note: Nearest point will likely be the point between two segments,
    // its dualy defined, but thats ok
    for (auto puncture = pData.needle.begin(); puncture != pData.needle.end();)
    {
        // Start with large value
        Vec3d  closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };
        double closestDist  = IMSTK_DOUBLE_MAX;

        // Variable for storing the segement nearest to the needle
        int nearestSegmentId = -1;

        const Vec3d& a = physMesh->getVertexPositions()->at(puncture->triVertIds[0]);
        const Vec3d& b = physMesh->getVertexPositions()->at(puncture->triVertIds[1]);
        const Vec3d& c = physMesh->getVertexPositions()->at(puncture->triVertIds[2]);

        const Vec3d baryPoint  = puncture->baryCoords.head<3>();
        const Vec3d puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

        // Only check segments within checkStride of previous segments.
        // This helps with thread switching directions
        int checkStride     = 1;
        int previousSegment = puncture->segId;

        int lowerBound = previousSegment - checkStride;
        int upperBound = previousSegment + checkStride;

        int strideStart = (lowerBound > 0) ? lowerBound : 0;
        int strideEnd   = (upperBound < m_needleMesh->getNumCells()) ? upperBound : m_needleMesh->getNumCells() - 1;

        for (int segmentId = strideStart; segmentId <= strideEnd; segmentId++)
        {
            const Vec2i& needleSegNodeIds = m_needleMesh->getCells()->at(segmentId);
            const Vec3d& x1 = m_needleMesh->getVertexPositions()->at(needleSegNodeIds[0]);
            const Vec3d& x2 = m_needleMesh->getVertexPositions()->at(needleSegNodeIds[1]);

            int caseType = -1;

            // Find the closest point on this segment
            const Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

            double newDist = (segClosestPoint - puncturePt).squaredNorm();
            if (newDist < closestDist)
            {
                closestDist      = newDist;
                closestPoint     = segClosestPoint;
                nearestSegmentId = segmentId;
            }
        }

        puncture->segId = nearestSegmentId;

        // Check and see if the closest point is at the tips of the needle
        // Note: Needle mesh is backwards
        Vec3d diffTail = closestPoint - m_needleMesh->getVertexPositions()->at(0);
        Vec3d diffTip  = closestPoint - m_needleMesh->getVertexPositions()->at(m_needleMesh->getNumVertices() - 1);

        // If the closest point is sufficiently close to the tip or tail then unpuncture can occur
        const double unpunctureEpsilon = 1e-8;
        if (diffTail.norm() < unpunctureEpsilon || diffTip.norm() < unpunctureEpsilon)
        {
            const PunctureId punctureId = getPunctureId(needle, puncturable, puncture->triId);
            needle->setState(punctureId, Puncture::State::REMOVED);
            puncture = pData.needle.erase(puncture);
            continue;
        }

        // If the tissue is on the last segment of the needle, then transition onto the thread
        if (puncture->segId == 0)
        {
            // Switch to thread
            puncture->segId = 0; // start at first segment on thread
            pData.thread.push_back(*puncture);

            const PunctureId punctureId = getPunctureId(needle, puncturable, puncture->triId);
            needle->setState(punctureId, Puncture::State::REMOVED);

            LOG(DEBUG) << "Thread punctured triangle: " << puncture->triId;
            m_threadPunctured = true;
            puncture = pData.needle.erase(puncture);

            continue;
        }

        // Now that we have the closest point on the needle to this penetration point, we can
        // generate and solve the constraint
        const int bodyId       = m_pbdTissueObj->getPbdBody()->bodyHandle;
        const int needleBodyId = m_needleObj->getPbdBody()->bodyHandle;
        auto      pointTriangleConstraint = std::make_shared<SurfaceInsertionConstraint>();
        pointTriangleConstraint->initConstraint(puncturePt,
            { needleBodyId, 0 },
            { bodyId, puncture->triVertIds[0] },
            { bodyId, puncture->triVertIds[1] },
            { bodyId, puncture->triVertIds[2] },
            closestPoint,
            baryPoint,
            m_needleToSurfaceStiffness, m_surfaceToNeedleStiffness // stiffness parameters
            );
        m_constraints.push_back(pointTriangleConstraint);

        puncture++;
    }

    // Use to keep track of the segments that have puncture points
    std::vector<std::pair<Vec2i, Vec2d>> punctureSegments;

    // Loop over thread penetration points and find nearest point on the thread
    // Note: Nearest point will likely be the point between two segments, its dualy defined
    // for (auto puncture : m_threadPData)
    for (auto puncture = pData.thread.begin(); puncture != pData.thread.end();)
    {
        const Vec3d& a = physMesh->getVertexPositions()->at(puncture->triVertIds[0]);
        const Vec3d& b = physMesh->getVertexPositions()->at(puncture->triVertIds[1]);
        const Vec3d& c = physMesh->getVertexPositions()->at(puncture->triVertIds[2]);

        const Vec3d& baryPoint  = puncture->baryCoords;
        const Vec3d& puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

        // Only check segments within checkStride of previous segments.
        // This helps with thread switching directions
        const int checkStride     = 1;
        const int previousSegment = puncture->segId;

        const int lowerBound = previousSegment - checkStride;
        const int upperBound = previousSegment + checkStride;

        const int numSegsNonPenetrating = 4;

        const int strideStart = (lowerBound > 0) ? lowerBound : 0;
        const int strideEnd   = (upperBound < m_threadMesh->getNumCells() - numSegsNonPenetrating) ? upperBound : m_threadMesh->getNumCells() - numSegsNonPenetrating;

        int closestSegmentId = -1;

        Vec3d  closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };
        double closestDist  = IMSTK_DOUBLE_MAX;

        int caseType = -1;

        // Note: stopping before last segment for visualization
        for (int segmentId = strideStart; segmentId <= strideEnd; segmentId++)
        {
            const Vec2i& threadSegNodeIds = m_threadMesh->getCells()->at(segmentId);
            const Vec3d& x1 = m_threadMesh->getVertexPositions()->at(threadSegNodeIds[0]);
            const Vec3d& x2 = m_threadMesh->getVertexPositions()->at(threadSegNodeIds[1]);

            const Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

            double newDist = (segClosestPoint - puncturePt).squaredNorm();

            if (newDist < closestDist)
            {
                closestPoint     = segClosestPoint;
                closestDist      = newDist;
                closestSegmentId = segmentId;
            }
        } // end loop over thread segments

        puncture->segId = closestSegmentId;

        // NOTE: Commented out to force thread to stay inserted once inserted
        // If uncommented, the thread would be able to slide through the mesh and unpuncture.
        // Unpuncture if puncture moves past last segment of the thread
        //const double unpunctureEpsilon = 1e-8;
        //int numCells = m_threadMesh->getNumCells();
        //if (closestSegmentId == numCells-1) {

        //    puncture = pData.thread.erase(puncture);
        //    continue;
        //}

        // Set of VM pairs for thread
        const Vec2i& closestSegment = m_threadMesh->getCells()->at(puncture->segId);

        if (closestSegment[0] < 0 || closestSegment[0] >= m_threadMesh->getNumCells()
            || closestSegment[1] < 0 || closestSegment[1] >= m_threadMesh->getNumCells())
        {
            LOG(FATAL) << "Invalid thread segment id: " << closestSegment[0] << ", " <<
                closestSegment[1];
        }

        const Vec3d& p = m_threadMesh->getVertexPositions()->at(closestSegment[0]);
        const Vec3d& q = m_threadMesh->getVertexPositions()->at(closestSegment[1]);

        // Thread barycentric intersection point
        const Vec2d segBary = baryCentric(closestPoint, p, q);

        punctureSegments.push_back(std::make_pair(closestSegment, segBary));

        puncture++;
    }

    // To match the order of the closest segments to the puncture points we
    // sort as the order of the puncture points is know to be in order of the thread
    std::sort(punctureSegments.begin(), punctureSegments.end(), [](const auto& seg1, const auto& seg2) {
            return seg1.first[0] > seg2.first[0] || (seg1.first[0] == seg2.first[0] && seg1.second[1] > seg2.second[1]);
        });

    // Generate constraints for each puncture point and the appropriate thread segment
    for (int i = 0; i < pData.thread.size() && i < punctureSegments.size(); ++i)
    {
        const auto& puncture = pData.thread[i];
        const auto& nearestSegNodeIds = punctureSegments[i].first;
        const auto& segBary = punctureSegments[i].second;

        const int tissueBodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
        const int threadBodyId = m_threadObj->getPbdBody()->bodyHandle;

        auto threadTriangleConstraint = std::make_shared<ThreadInsertionConstraint>();

        threadTriangleConstraint->initConstraint(
            m_pbdTissueObj->getPbdModel()->getBodies(),
            { threadBodyId, nearestSegNodeIds[0] },
            { threadBodyId, nearestSegNodeIds[1] },
            segBary,
            { tissueBodyId, puncture.triVertIds[0] },
            { tissueBodyId, puncture.triVertIds[1] },
            { tissueBodyId, puncture.triVertIds[2] },
            puncture.baryCoords,
            m_threadToSurfaceStiffness, m_surfaceToThreadStiffness); // Tissue is not currently moved
        m_constraints.push_back(threadTriangleConstraint);
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

    auto threadBodyId = m_threadObj->getPbdBody()->bodyHandle;
    auto needleBodyId = m_needleObj->getPbdBody()->bodyHandle;

    // Add constraint to connect the needle to the thread
    int numTiedSegments = 1;
    for (int i = 0; i <= numTiedSegments; i++)
    {
        auto needleThreadConstraint = std::make_shared<PbdVertexToBodyConstraint>();
        needleThreadConstraint->initConstraint(
            m_threadObj->getPbdModel()->getBodies(),
            { needleBodyId, 0 },
            needleMesh->getVertexPositions()->at(numTiedSegments - i),
            { threadBodyId, i },
            0.0001);
        m_constraints.push_back(needleThreadConstraint);
    }

    // Add constraints for stitched tissue
    for (size_t i = 0; i < m_stitchConstraints.size(); i++)
    {
        m_constraints.push_back(m_stitchConstraints[i]);
    }

    // Handle needle collision normally if no insertion
    m_needlePunctured = didPuncture(elementsA, elementsB) || m_needlePunctured;
    if (!m_needlePunctured)
    {
        PbdCollisionHandling::handle(elementsA, elementsB); // (PBD Object, Needle Object)
    }
    if (m_needlePunctured == true || m_threadPunctured == true)
    {
        // If a new puncture is found, generate data for constraints
        if (m_needlePunctured == true)
        {
            generateNewPunctureData();
        }

        // Handle generating constraints for both needle
        // and thread with existing puncture points
        addPunctureConstraints();

        // Solve stitching constraint
        //if (m_stitch)
        //{
        //    for (size_t i = 0; i < m_stitchConstraints.size(); i++)
        //    {
        //        m_constraints.push_back(m_stitchConstraints[i]);
        //    }
        //}

        if (pData.needle.size() == 0)
        {
            m_needlePunctured = false;
        }

        if (pData.thread.size() == 0)
        {
            m_threadPunctured = false;
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
    if (pData.thread.size() < 4)
    {
        LOG(INFO) << "Cant stitch less than 4 points punctured by thread, currently only " << pData.thread.size() << " points";
        return;
    }

    // If stitching, move the puncture data from the thread to the stitch, and generate
    // constraints to solve for the stitch.
    if (pData.thread.size() >= 4)
    {
        LOG(INFO) << "Stitching!";

        auto                                     physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdTissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = physMesh->getVertexPositions();
        VecDataArray<double, 3>&                 tissueVertices    = *tissueVerticesPtr;

        std::vector<PuncturePoint> stitchedPoints;
        for (size_t pPointId = 0; pPointId < pData.thread.size(); pPointId++)
        {
            stitchedPoints.push_back(pData.thread[pPointId]);
        }

        pData.stitch.push_back(stitchedPoints);
        pData.thread.clear();

        // Calculate the average position of the points punctured by thread
        Vec3d center = Vec3d::Zero();
        Vec3d pPoint = Vec3d::Zero();
        for (size_t pPointId = 0; pPointId < stitchedPoints.size(); pPointId++)
        {
            pPoint += tissueVertices[stitchedPoints[pPointId].triVertIds[0]] * stitchedPoints[pPointId].baryCoords[0]
                      + tissueVertices[stitchedPoints[pPointId].triVertIds[1]] * stitchedPoints[pPointId].baryCoords[1]
                      + tissueVertices[stitchedPoints[pPointId].triVertIds[2]] * stitchedPoints[pPointId].baryCoords[2];
        }

        center = pPoint / stitchedPoints.size();
        m_stitchPoints.push_back(center);

        m_stitch = true;

        // Create constraints to pull the puncture points to the center location
        auto points = pData.stitch.back();
        for (int pPointId = 0; pPointId < points.size(); pPointId++)
        {
            // Now create values for the central point
            const PbdParticleId& stitchCenterPt = m_pbdTissueObj->getPbdModel()->addVirtualParticle(center, 0.0, Vec3d::Zero(), true);

            const int           bodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
            const PbdParticleId p0     = { bodyId, points[pPointId].triVertIds[0] };
            const PbdParticleId p1     = { bodyId, points[pPointId].triVertIds[1] };
            const PbdParticleId p2     = { bodyId, points[pPointId].triVertIds[2] };

            auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
            constraint->initConstraint(
                { p0, p1, p2 },
                { points[pPointId].baryCoords[0],
                  points[pPointId].baryCoords[1],
                  points[pPointId].baryCoords[2] },
                { stitchCenterPt }, { 1.0 },
                0.01, 0.0);

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
        const auto& needleElement = needleElements->at(i);
        const auto& tissueElement = tissueElements->at(i);

        CHECK(tissueElement.m_type == CollisionElementType::CellIndex)
            << "Suturing only works with CDs that report CellIndex contact";
        CHECK(tissueElement.m_element.m_CellIndexElement.parentId != -1)
            << "Suturing only works with CDs that report parent ids";
        // if (deformableElement.m_type != CollisionElementType::CellIndex || deformableElement.m_element.m_CellIndexElement.parentId == -1) continue;

        // Get info about what point on the needle is in contact, used to only allow tip to create puncture
        int contactPtId = -1;
        if (needleElement.m_type == CollisionElementType::CellIndex)
        {
            contactPtId = needleElement.m_element.m_CellIndexElement.ids[0];
        }

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

        const Vec3d needleTip = needleVertices[endIndex];

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

        if (contactPtId == endIndex)
        {
            if (needle->getState(punctureId) == Puncture::State::TOUCHING) // Touching
            {
                // If the needle is close to perpendicular to the face if may insert
                // Note: This is a short term solution
                if (dotProduct > m_threshold)
                {
                    // LOG(INFO) << "Needle inserted";
                    return true;
                }
            }
        }
    }
    return false;
}
} // namespace imstk