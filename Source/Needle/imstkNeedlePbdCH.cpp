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
                PenetrationData newPuncture;

                newPuncture.triId = triangleId;
                newPuncture.triVertIds[0] = physTriIds[0];
                newPuncture.triVertIds[1] = physTriIds[1];
                newPuncture.triVertIds[2] = physTriIds[2];
                newPuncture.triBaryPuncturePoint[0] = uvw[0];
                newPuncture.triBaryPuncturePoint[1] = uvw[1];
                newPuncture.triBaryPuncturePoint[2] = uvw[2];
                newPuncture.segId = tipSegmentId;

                m_needlePData.push_back(newPuncture);
                m_punctureData.push_back(newPuncture);
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
    for (auto puncture = m_needlePData.begin(); puncture != m_needlePData.end();)
    {
        // Start with large value
        Vec3d  closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };
        double closestDist  = IMSTK_DOUBLE_MAX;

        // Variable for storing the segement nearest to the needle
        int nearestSegmentId = -1;

        const Vec3d& a = physMesh->getVertexPositions()->at(puncture->triVertIds[0]);
        const Vec3d& b = physMesh->getVertexPositions()->at(puncture->triVertIds[1]);
        const Vec3d& c = physMesh->getVertexPositions()->at(puncture->triVertIds[2]);

        const Vec3d baryPoint  = puncture->triBaryPuncturePoint.head<3>();
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
            puncture = m_needlePData.erase(puncture);
            continue;
        }

        // If the tissue is on the last segment of the needle, then transition onto the thread
        if (puncture->segId == 0)
        {
            // Switch to thread
            puncture->segId = 0; // start at first segment on thread
            m_threadPData.push_back(*puncture);

            const PunctureId punctureId = getPunctureId(needle, puncturable, puncture->triId);
            needle->setState(punctureId, Puncture::State::REMOVED);

            LOG(DEBUG) << "Thread punctured triangle: " << puncture->triId;
            m_threadPunctured = true;
            puncture = m_needlePData.erase(puncture);

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

    // Loop over thread penetration points and find nearest point on the thread
    // Note: Nearest point will likely be the point between two segments, its dualy defined
    // for (auto puncture : m_threadPData)
    for (auto puncture = m_threadPData.begin(); puncture != m_threadPData.end();)
    {
        // Start with arbitrary large value
        Vec3d closestPoint = { IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX };

        const Vec3d& a = physMesh->getVertexPositions()->at(puncture->triVertIds[0]);
        const Vec3d& b = physMesh->getVertexPositions()->at(puncture->triVertIds[1]);
        const Vec3d& c = physMesh->getVertexPositions()->at(puncture->triVertIds[2]);

        Vec3d baryPoint = puncture->triBaryPuncturePoint;

        auto puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

        int nearestSegmentId = -1;

        // Only check segments within checkStride of previous segments.
        // This helps with thread switching directions
        int checkStride     = 1;
        int previousSegment = puncture->segId;

        int lowerBound = previousSegment - checkStride;
        int upperBound = previousSegment + checkStride;

        int numSegsNonPenetrating = 4;

        int strideStart = (lowerBound > 0) ? lowerBound : 0;
        int strideEnd   = (upperBound < m_threadMesh->getNumCells() - numSegsNonPenetrating) ? upperBound : m_threadMesh->getNumCells() - numSegsNonPenetrating;

        // Note: stopping before last segment for visualization
        for (int segmentId = strideStart; segmentId <= strideEnd; segmentId++)
        {
            const Vec2i& threadSegNodeIds = m_threadMesh->getCells()->at(segmentId);
            const Vec3d& x1 = m_threadMesh->getVertexPositions()->at(threadSegNodeIds[0]);
            const Vec3d& x2 = m_threadMesh->getVertexPositions()->at(threadSegNodeIds[1]);

            int caseType = -1;

            const Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

            const Vec3d newDist = segClosestPoint - puncturePt;
            const Vec3d oldDist = closestPoint - puncturePt;

            if (newDist.norm() <= oldDist.norm())
            {
                closestPoint     = segClosestPoint;
                nearestSegmentId = segmentId;
            }
        } // end loop over thread segments

        puncture->segId = nearestSegmentId;

        // NOTE: Commented out to force thread to stay inserted once inserted
        // If uncommented, the thread would be able to slide through the mesh and unpuncture.
        // Unpuncture if puncture moves past last segment of the thread
        //const double unpunctureEpsilon = 1e-8;
        //int numCells = m_threadMesh->getNumCells();
        //if (closestSegmentId == numCells-1) {

        //    puncture = m_threadPData.erase(puncture);
        //    continue;
        //}

        // Now that we have the closest point on the thread to this penetration point, we can
        // generate and solve the constraint
        auto threadTriangleConstraint = std::make_shared<ThreadInsertionConstraint>();

        // Set of VM pairs for thread
        const Vec2i& nearestSegNodeIds = m_threadMesh->getCells()->at(puncture->segId);
        const Vec3d& p = m_threadMesh->getVertexPositions()->at(nearestSegNodeIds[0]);
        const Vec3d& q = m_threadMesh->getVertexPositions()->at(nearestSegNodeIds[1]);

        // Thread barycentric intersection point
        const Vec2d segBary = baryCentric(closestPoint, p, q);

        const int tissueBodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
        const int threadBodyId = m_threadObj->getPbdBody()->bodyHandle;
        threadTriangleConstraint->initConstraint(
            m_pbdTissueObj->getPbdModel()->getBodies(),
            { threadBodyId, nearestSegNodeIds[0] },
            { threadBodyId, nearestSegNodeIds[1] },
            segBary,
            { tissueBodyId, puncture->triVertIds[0] },
            { tissueBodyId, puncture->triVertIds[1] },
            { tissueBodyId, puncture->triVertIds[2] },
            puncture->triBaryPuncturePoint,
            m_threadToSurfaceStiffness, m_surfaceToThreadStiffness); // Tissue is not currently moved
        m_constraints.push_back(threadTriangleConstraint);

        puncture++;
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
        if (m_stitch)
        {
            for (size_t i = 0; i < m_stitchConstraints.size(); i++)
            {
                m_constraints.push_back(m_stitchConstraints[i]);
            }
        }

        if (m_needlePData.size() == 0)
        {
            m_needlePunctured = false;
        }

        if (m_threadPData.size() == 0)
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
    if (m_punctureData.size() < 4)
    {
        LOG(INFO) << "Cant stitch less than 4 points, currently only " << m_punctureData.size() << " points";
        return;
    }

    if (m_punctureData.size() >= 4)
    {
        LOG(INFO) << "Stitching!";

        std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = m_tissueSurfMesh->getVertexPositions();
        VecDataArray<double, 3>&                 tissueVertices    = *tissueVerticesPtr;

        // Only calculate the center point once
        if (m_stitch == false)
        {
            // Calculate the average position of the points punctured by thread
            for (size_t pPointId = 0; pPointId < m_punctureData.size(); pPointId++)
            {
                Vec3d pPoint = tissueVertices[m_punctureData[pPointId].triVertIds[0]] * m_punctureData[pPointId].triBaryPuncturePoint[0]
                               + tissueVertices[m_punctureData[pPointId].triVertIds[1]] * m_punctureData[pPointId].triBaryPuncturePoint[1]
                               + tissueVertices[m_punctureData[pPointId].triVertIds[2]] * m_punctureData[pPointId].triBaryPuncturePoint[2];

                for (int i = 0; i < 3; i++)
                {
                    m_stitchCenter[i] += pPoint[i] / static_cast<double>(m_punctureData.size());
                }
            }
        }

        m_stitch = true;

        // Create constraints to pull the puncture points to the center location
        for (int pPointId = 0; pPointId < m_punctureData.size(); pPointId++)
        {
            // Now create values for the central point
            const PbdParticleId& stitchCenterPt = m_pbdTissueObj->getPbdModel()->addVirtualParticle(m_stitchCenter, 0.0);

            const int           bodyId = m_pbdTissueObj->getPbdBody()->bodyHandle;
            const PbdParticleId p0     = { bodyId, m_punctureData[pPointId].triVertIds[0] };
            const PbdParticleId p1     = { bodyId, m_punctureData[pPointId].triVertIds[1] };
            const PbdParticleId p2     = { bodyId, m_punctureData[pPointId].triVertIds[2] };

            auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
            constraint->initConstraint(
                { p0, p1, p2 },
                { m_punctureData[pPointId].triBaryPuncturePoint[0],
                  m_punctureData[pPointId].triBaryPuncturePoint[1],
                  m_punctureData[pPointId].triBaryPuncturePoint[2] },
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