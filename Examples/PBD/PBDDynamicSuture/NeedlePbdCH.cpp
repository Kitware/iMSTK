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

#include "imstkPbdObject.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkCollisionUtils.h"
#include "imstkPointwiseMap.h"

#include "NeedlePbdCH.h"

#include <cmath>

// using namespace imstk;
namespace imstk
{
// Initialize interaction data
void
NeedlePbdCH::init(std::shared_ptr<PbdObject> threadObj)
{
    // Setup pbd tissue object
    m_pbdTissueObj = std::dynamic_pointer_cast<PbdObject>(getInputObjectA());
    auto physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdTissueObj->getPhysicsGeometry());
    m_meshVerticesPtr = physMesh->getVertexPositions();

    // Get surface mesh
    m_surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_pbdTissueObj->getCollidingGeometry());

    // set up thread mesh
    m_threadObj  = threadObj;
    m_threadMesh = std::dynamic_pointer_cast<LineMesh>(m_threadObj->getCollidingGeometry());
    m_threadVerticesPtr = m_threadMesh->getVertexPositions();

    // Create storage for puncture states
    m_isNeedlePunctured.resize(m_surfMesh->getNumTriangles());
    m_isThreadPunctured.resize(m_surfMesh->getNumTriangles());

    // Initialize to false
    for (int triangleId = 0; triangleId < m_surfMesh->getNumTriangles(); triangleId++)
    {
        m_isNeedlePunctured[triangleId] = false;
        m_isThreadPunctured[triangleId] = false;
    }
}

void
NeedlePbdCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)         //override
{
    // Unpack needle data

    // Get rigid object needle
    auto needleRigid = std::dynamic_pointer_cast<RigidBody>(getInputObjectB());

    // Setup needle and get collision state
    auto needleObj  = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());
    auto needleMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getCollidingGeometry());

    std::shared_ptr<VecDataArray<double, 3>> needleVerticesPtr = needleMesh->getVertexPositions();
    VecDataArray<double, 3>&                 needleVertices    = *needleVerticesPtr;

    // Unpack thread data

    // Get thread vertex positions
    VecDataArray<double, 3>& threadVertices = *m_threadVerticesPtr;

    // Get thread velocities
    std::shared_ptr<PointSet> pointSetA = std::dynamic_pointer_cast<PointSet>(m_threadObj->getPhysicsGeometry());
    auto                      threadVelocitiesPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetA->getVertexAttribute("Velocities"));
    VecDataArray<double, 3>&  threadVelocity      = *threadVelocitiesPtr;

    // Unpack tissue data

    // Get vertex positions
    VecDataArray<double, 3>& meshVertices = *m_meshVerticesPtr;

    // For something to be a PbdObject it must have a pointset, it must also have invMasses defined
    std::shared_ptr<PointSet>                pointSetB = std::dynamic_pointer_cast<PointSet>(m_pbdTissueObj->getPhysicsGeometry());
    const std::shared_ptr<DataArray<double>> triangleInvMassesAPtr = std::dynamic_pointer_cast<DataArray<double>>(pointSetB->getVertexAttribute("InvMass"));
    const DataArray<double>&                 triangleInvMasses     = *triangleInvMassesAPtr;

    // Get velocities
    std::shared_ptr<VecDataArray<double, 3>> velocitiesAPtr = std::dynamic_pointer_cast<VecDataArray<double, 3>>(pointSetB->getVertexAttribute("Velocities"));
    VecDataArray<double, 3>&                 meshVelocity   = *velocitiesAPtr;

    // Save the direction of the tip of the needle. NOTE: Needle indices are backwards
    m_needleDirection = (needleVertices[35] - needleVertices[34]).normalized();

    // Create one to one map between the physics mesh and the surface mesh
    auto one2one = std::dynamic_pointer_cast<PointwiseMap>(m_pbdTissueObj->getPhysicsToCollidingMap());
    CHECK(one2one != nullptr) << "Failed to generate one to one map in NeedlePbdCH";

    // Check to make sure that the mesh size has not changed (no cutting is allowed currently)

    if (m_isNeedlePunctured.size() != m_surfMesh->getNumTriangles())
    {
        LOG(FATAL) << "Surface Mesh has changed size";
    }

    // Do it the normal way if not inserted or touching
    if (needleObj->getCollisionState() == NeedleObject::CollisionState::REMOVED || needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
    {
        PbdCollisionHandling::handle(elementsA, elementsB);     // (PBD Object, Needle Object)
    }

    // If inserted, find intersections and constrain to insertion points
    if (needleObj->getCollisionState() == NeedleObject::CollisionState::INSERTED)
    {
        // Scope for needle
        {
            // First, find new penetration points using the tip of the needle (needle mesh is reversed)
            int tipSegmentId = needleMesh->getNumLines() - 1;

            Vec2i       nodeIds = needleMesh->getLineIndices(tipSegmentId);
            const Vec3d tip1    = needleVertices[nodeIds[0]];
            const Vec3d tip2    = needleVertices[nodeIds[1]];

            // Loop over all triangles in the surface mesh
            for (int triangleId = 0; triangleId < m_surfMesh->getNumTriangles(); triangleId++)
            {
                auto surfTriIds = m_surfMesh->getTriangleIndices(triangleId);

                // Indices of the vertices on the physics mesh
                Vec3i physTriIds;
                physTriIds[0] = one2one->getParentVertexId(surfTriIds[0]);
                physTriIds[1] = one2one->getParentVertexId(surfTriIds[1]);
                physTriIds[2] = one2one->getParentVertexId(surfTriIds[2]);

                Vec3d a = meshVertices[physTriIds[0]];
                Vec3d b = meshVertices[physTriIds[1]];
                Vec3d c = meshVertices[physTriIds[2]];

                // Barycentric coordinates of interseciton point
                Vec3d uvw = Vec3d::Zero();

                // If this triangle has not already been punctured
                if (m_isNeedlePunctured[triangleId] == false)
                {
                    // Check for intersection
                    if (CollisionUtils::testSegmentTriangle(tip1, tip2, a, b, c, uvw))
                    {
                        m_isNeedlePunctured[triangleId] = true;

                        SuturePenetrationData newPuncture;

                        newPuncture.triId = triangleId;

                        newPuncture.triVerts[0] = &meshVertices[physTriIds[0]];
                        newPuncture.triVerts[1] = &meshVertices[physTriIds[1]];
                        newPuncture.triVerts[2] = &meshVertices[physTriIds[2]];

                        newPuncture.triVertIds[0] = physTriIds[0];
                        newPuncture.triVertIds[1] = physTriIds[1];
                        newPuncture.triVertIds[2] = physTriIds[2];

                        newPuncture.triBaryPuncturePoint = uvw;

                        // Create and save the puncture point
                        m_needlePData.push_back(newPuncture);
                        LOG(DEBUG) << "Punctured triangle: " << triangleId;
                    }     // end if intersection
                }         // end if punctured
            }             // end loop over triangles

            // Loop over penetration points and find nearest point on the needle
            // Note: Nearest point will likely be the point between two segments,
            // its dualy defined, but thats ok
            for (int pPointId = 0; pPointId < m_needlePData.size(); pPointId++)
            {
                // Start with large value
                constexpr double maxVal       = IMSTK_DOUBLE_MAX; //  std::numeric_limits<double>::max();
                Vec3d            closestPoint = { maxVal, maxVal, maxVal };
                double           closestDist  = IMSTK_DOUBLE_MAX;

                Vec3d a = meshVertices[m_needlePData[pPointId].triVertIds[0]];
                Vec3d b = meshVertices[m_needlePData[pPointId].triVertIds[1]];
                Vec3d c = meshVertices[m_needlePData[pPointId].triVertIds[2]];

                Vec3d baryPoint  = m_needlePData[pPointId].triBaryPuncturePoint;
                auto  puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

                for (int segmentId = 0; segmentId < needleMesh->getNumLines(); segmentId++)
                {
                    Vec2i       needleSegNodeIds = needleMesh->getLineIndices(segmentId);
                    const Vec3d x1 = needleVertices[needleSegNodeIds[0]];
                    const Vec3d x2 = needleVertices[needleSegNodeIds[1]];

                    int caseType = -1;

                    // Find the closest point on this segment
                    Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

                    double newDist = (segClosestPoint - puncturePt).squaredNorm();
                    if (newDist < closestDist)
                    {
                        closestDist  = newDist;
                        closestPoint = segClosestPoint;
                    }
                }     // end loop over segments

                // Check and see if the closest point is at the tips of the needle
                // Note: Needle mesh is backwards
                Vec3d diffTail = closestPoint - needleVertices[0];
                Vec3d diffTip  = closestPoint - needleVertices[needleMesh->getNumVertices() - 1];

                // If the closest point is sufficiently close to the tip or tail then unpuncture can occur
                const double unpunctureEpsilon = 1e-8;
                if (diffTail.norm() < unpunctureEpsilon || diffTip.norm() < unpunctureEpsilon)
                {
                    // If the tip of the needle has been removed,
                    // this triangle is no longer punctured by the needle
                    if (diffTip.norm() < unpunctureEpsilon)
                    {
                        m_isNeedlePunctured[m_needlePData[pPointId].triId] = false;
                    }

                    m_needlePData.erase(m_needlePData.begin() + pPointId);
                    continue;
                }

                // Now that we have the closest point on the needle to this penetration point, we can
                // generate and solve the constraint

                auto pointTriangleConstraint = std::make_shared<SurfaceInsertionConstraint>();

                VertexMassPair ptB1;
                VertexMassPair ptB2;
                VertexMassPair ptB3;

                ptB1.vertex = &meshVertices[m_needlePData[pPointId].triVertIds[0]];
                ptB2.vertex = &meshVertices[m_needlePData[pPointId].triVertIds[1]];
                ptB3.vertex = &meshVertices[m_needlePData[pPointId].triVertIds[2]];

                ptB1.invMass = triangleInvMasses[m_needlePData[pPointId].triVertIds[0]];
                ptB2.invMass = triangleInvMasses[m_needlePData[pPointId].triVertIds[1]];
                ptB3.invMass = triangleInvMasses[m_needlePData[pPointId].triVertIds[2]];

                ptB1.velocity = &meshVelocity[m_needlePData[pPointId].triVertIds[0]];
                ptB2.velocity = &meshVelocity[m_needlePData[pPointId].triVertIds[1]];
                ptB3.velocity = &meshVelocity[m_needlePData[pPointId].triVertIds[2]];

                pointTriangleConstraint->initConstraint(
                        puncturePt,
                        ptB1, ptB2, ptB3,
                        closestPoint,
                        baryPoint,
                        0.0, 0.9 // stiffness parameters
                    );

                pointTriangleConstraint->solvePosition();
            } // end loop over penetration points
        }     // end scope for needle

        {     // Scope for thread
              // Now on to the thread!!!
              // use the tip of the the same way as the tip of the needle
              // to set up thread penetration points

            Vec3d threadTip = threadVertices[0];

            // First, find new penetration points
            Vec2i       nodeIds    = m_threadMesh->getLineIndices(0);
            const Vec3d threadTip1 = threadVertices[nodeIds[0]];
            const Vec3d threadTip2 = threadVertices[nodeIds[1]];

            // Loop over all triangles in the surface mesh
            for (int triangleId = 0; triangleId < m_surfMesh->getNumTriangles(); triangleId++)
            {
                auto surfTriIds = m_surfMesh->getTriangleIndices(triangleId);

                // Indices of the vertices on the physics mesh
                Vec3i physTriIds;
                physTriIds[0] = one2one->getParentVertexId(surfTriIds[0]);
                physTriIds[1] = one2one->getParentVertexId(surfTriIds[1]);
                physTriIds[2] = one2one->getParentVertexId(surfTriIds[2]);

                Vec3d a = meshVertices[physTriIds[0]];
                Vec3d b = meshVertices[physTriIds[1]];
                Vec3d c = meshVertices[physTriIds[2]];

                // Barycentric coordinates of interseciton point
                Vec3d uvw = Vec3d::Zero();

                // If this triangle has already been punctured by the needle
                if (m_isNeedlePunctured[triangleId] == true)
                {
                    // If it has not yet been punctured by thread
                    if (m_isThreadPunctured[triangleId] == false)
                    {
                        // Check for intersection
                        if (CollisionUtils::testSegmentTriangle(threadTip1, threadTip2, a, b, c, uvw) == true)
                        {
                            m_isThreadPunctured[triangleId] = true;

                            // Find matching needle puncture point
                            int needlePuncturePointId = -1;
                            for (int needlePPts = 0; needlePPts < m_needlePData.size(); needlePPts++)
                            {
                                if (m_needlePData[needlePPts].triId == triangleId)
                                {
                                    needlePuncturePointId = needlePPts;
                                }
                            }

                            // Create thread puncture point and copy in data from needle puncture point
                            SuturePenetrationData newPuncture;

                            newPuncture.triId = m_needlePData[needlePuncturePointId].triId;

                            newPuncture.triVerts[0] = &meshVertices[physTriIds[0]];
                            newPuncture.triVerts[1] = &meshVertices[physTriIds[1]];
                            newPuncture.triVerts[2] = &meshVertices[physTriIds[2]];

                            newPuncture.triVertIds[0] = physTriIds[0];
                            newPuncture.triVertIds[1] = physTriIds[1];
                            newPuncture.triVertIds[2] = physTriIds[2];

                            newPuncture.triBaryPuncturePoint = uvw;

                            // Create and save the puncture point
                            m_threadPData.push_back(newPuncture);
                        } // end if intersecting
                    }     // end if punctured by thread
                }         // end if punctured by needle
            }             // end loop over surface mesh

            // Loop over thread penetration points and find nearest point on the thread
            // Note: Nearest point will likely be the point between two segments, its dualy defined
            for (int pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
            {
                // Start with arbitrary large value
                constexpr double maxVal       = IMSTK_DOUBLE_MAX;
                Vec3d            closestPoint = { maxVal, maxVal, maxVal };

                Vec3d a = meshVertices[m_threadPData[pPointId].triVertIds[0]];
                Vec3d b = meshVertices[m_threadPData[pPointId].triVertIds[1]];
                Vec3d c = meshVertices[m_threadPData[pPointId].triVertIds[2]];

                Vec3d baryPoint = m_threadPData[pPointId].triBaryPuncturePoint;

                auto puncturePt = baryPoint[0] * a + baryPoint[1] * b + baryPoint[2] * c;

                int closestSegmentId = -1;

                // Note: stopping before last segment for visualization
                for (int segmentId = 0; segmentId < m_threadMesh->getNumLines() - 1; segmentId++)
                {
                    Vec2i       threadSegNodeIds = m_threadMesh->getLineIndices(segmentId);
                    const Vec3d x1 = threadVertices[threadSegNodeIds[0]];
                    const Vec3d x2 = threadVertices[threadSegNodeIds[1]];

                    int caseType = -1;

                    Vec3d segClosestPoint = CollisionUtils::closestPointOnSegment(puncturePt, x1, x2, caseType);

                    Vec3d newDist = segClosestPoint - puncturePt;
                    Vec3d oldDist = closestPoint - puncturePt;

                    if (newDist.norm() <= oldDist.norm())
                    {
                        closestPoint     = segClosestPoint;
                        closestSegmentId = segmentId;
                    }
                }     // end loop over thread segments

                // Check and see if the closest point is at the tips of the thread
                Vec3d diffTip  = closestPoint - threadVertices[0];
                Vec3d diffTail = closestPoint - threadVertices[m_threadMesh->getNumVertices() - 1];

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
                Vec2i       nearestSegNodeIds = m_threadMesh->getLineIndices(closestSegmentId);
                const Vec3d p = threadVertices[nearestSegNodeIds[0]];
                const Vec3d q = threadVertices[nearestSegNodeIds[1]];

                VertexMassPair ptA1;
                ptA1.vertex   = &threadVertices[nearestSegNodeIds[0]];
                ptA1.invMass  = 1.0;    // threadInvMasses[nearestSegNodeIds[0]];
                ptA1.velocity = &threadVelocity[nearestSegNodeIds[0]];

                VertexMassPair ptA2;
                ptA2.vertex   = &threadVertices[nearestSegNodeIds[1]];
                ptA2.invMass  = 1.0;    // threadInvMasses[nearestSegNodeIds[1]];
                ptA2.velocity = &threadVelocity[nearestSegNodeIds[1]];

                // Thread barycentric intersection point
                Vec2d segBary = baryCentric(closestPoint, p, q);

                // Set of VM pairs for triangle
                VertexMassPair ptB1;
                VertexMassPair ptB2;
                VertexMassPair ptB3;

                ptB1.vertex = &meshVertices[m_threadPData[pPointId].triVertIds[0]];
                ptB2.vertex = &meshVertices[m_threadPData[pPointId].triVertIds[1]];
                ptB3.vertex = &meshVertices[m_threadPData[pPointId].triVertIds[2]];

                ptB1.invMass = triangleInvMasses[m_threadPData[pPointId].triVertIds[0]];
                ptB2.invMass = triangleInvMasses[m_threadPData[pPointId].triVertIds[1]];
                ptB3.invMass = triangleInvMasses[m_threadPData[pPointId].triVertIds[2]];

                ptB1.velocity = &meshVelocity[m_threadPData[pPointId].triVertIds[0]];
                ptB2.velocity = &meshVelocity[m_threadPData[pPointId].triVertIds[1]];
                ptB3.velocity = &meshVelocity[m_threadPData[pPointId].triVertIds[2]];

                threadTriangleConstraint->initConstraint(
                        ptA1, ptA2, segBary,
                        ptB1, ptB2, ptB3, m_threadPData[pPointId].triBaryPuncturePoint,
                        0.2, 0.0);

                threadTriangleConstraint->solvePosition();
            }     // end loop over penetration points for thread
        }         // end scope for thread

        // Solve stitching constraint
        if (m_stitch)
        {
            for (size_t i = 0; i < m_stitchConstraints.size(); i++)
            {
                m_stitchConstraints[i]->solvePosition();
            }
        }
    }     // end needle state puncture check

    // If there are no penetration points, the needle is removed
    if (m_needlePData.size() == 0 && m_threadPData.size() == 0)
    {
        needleObj->setCollisionState(NeedleObject::CollisionState::REMOVED);
    }
}

// Create stitching constraints
void
NeedlePbdCH::stitch()
{
    // First, verify that at least 4 points have been penetrated by the thread
    if (m_threadPData.size() < 4)
    {
        LOG(INFO) << "Cant stitch less than 4 points";
        return;
    }

    if (m_threadPData.size() >= 4)
    {
        LOG(INFO) << "Stitching!";

        // Only calculate the center point once
        if (m_stitch == false)
        {
            // Calculate the average position of the points punctured by thread
            for (size_t pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
            {
                auto pPoint = (*m_threadPData[pPointId].triVerts[0]) * m_threadPData[pPointId].triBaryPuncturePoint[0]
                              + (*m_threadPData[pPointId].triVerts[1]) * m_threadPData[pPointId].triBaryPuncturePoint[1]
                              + (*m_threadPData[pPointId].triVerts[2]) * m_threadPData[pPointId].triBaryPuncturePoint[2];

                for (int i = 0; i < 3; i++)
                {
                    m_stitchCenter[i] += pPoint[i] / (double)m_threadPData.size();
                }
            }
        }

        m_stitch = true;

        // Create constraints to pull the puncture points to the center location
        for (int pPointId = 0; pPointId < m_threadPData.size(); pPointId++)
        {
            VertexMassPair ptA1;
            VertexMassPair ptA2;
            VertexMassPair ptA3;

            ptA1.vertex = m_threadPData[pPointId].triVerts[0];
            ptA2.vertex = m_threadPData[pPointId].triVerts[1];
            ptA3.vertex = m_threadPData[pPointId].triVerts[2];

            ptA1.invMass = 1.0;              //  triangleInvMasses[m_threadPData[pPointId].triertIds[0]];
            ptA2.invMass = 1.0;              //  triangleInvMasses[m_threadPData[pPointId].triVertIds[0]];
            ptA3.invMass = 1.0;              // triangleInvMasses[m_threadPData[pPointId].triVertIds[0]];

            ptA1.velocity = &m_fakeVelocity; //  &meshVelocity[m_threadPData[pPointId].triVertIds[0]];
            ptA2.velocity = &m_fakeVelocity; //  &meshVelocity[m_threadPData[pPointId].triVertIds[1]];
            ptA3.velocity = &m_fakeVelocity; //  &meshVelocity[m_threadPData[pPointId].triVertIds[2]];

            std::vector<VertexMassPair> ptsA = { ptA1, ptA2, ptA3 };

            std::vector<double> weightsA = { m_threadPData[pPointId].triBaryPuncturePoint[0],
                                             m_threadPData[pPointId].triBaryPuncturePoint[1],
                                             m_threadPData[pPointId].triBaryPuncturePoint[2] };

            // Now create values for the central point
            VertexMassPair centerPt;

            centerPt.vertex  = &m_stitchCenter;
            centerPt.invMass = 0.0;

            centerPt.velocity = &m_fakeVelocity;

            std::vector<VertexMassPair> ptsB     = { centerPt };
            std::vector<double>         weightsB = { 1.0 };

            auto constraint = std::make_shared<PbdBaryPointToPointConstraint>();
            constraint->initConstraint(
                    ptsA, weightsA,
                    ptsB, weightsB,
                    0.2, 0.0);

            // Add to list of constraints to be solved together in the handler
            m_stitchConstraints.push_back(constraint);
        }
    }
}

///
/// \brief Add a vertex-triangle constraint
///
void
NeedlePbdCH::addVTConstraint(
    VertexMassPair ptA,
    VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
    double stiffnessA, double stiffnessB)
{
    auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectB());

    // If removed and we are here, we must now be touching
    if (needleObj->getCollisionState() == NeedleObject::CollisionState::REMOVED)
    {
        needleObj->setCollisionState(NeedleObject::CollisionState::TOUCHING);
    }

    // If touching we may test for insertion
    // Calculate the surface normal using the set of vertices associated with the triangle and normalize
    // use dot product to project onto the needle stabing direction, if close to 1 assume its inserted
    // Possibly add contact time or pseudo force calculation to know if penetration occurs

    Vec3d surfNormal = Vec3d::Zero();

    // Note: assumes closed mesh

    // Assuming traingle has points a,b,c
    Vec3d ab = *ptB2.vertex - *ptB1.vertex;
    Vec3d ac = *ptB3.vertex - *ptB1.vertex;

    // Calculate surface normal
    surfNormal = (ac.cross(ab)).normalized();

    // Get vector pointing in direction of needle
    // Use absolute value to ignore direction issues
    auto dotProduct = fabs(m_needleDirection.dot(surfNormal));

    // Arbitrary threshold
    double threshold = 0.9;

    if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
    {
        // If the needle is close to perpindicular to the face if may insert
        // Note: This is a short term solution
        if (dotProduct > threshold)
        {
            needleObj->setCollisionState(NeedleObject::CollisionState::INSERTED);
            needleObj->setPrevCollisionState(NeedleObject::PrevCollisionState::INSERTED);
        }
    }

    if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
    {
        PbdCollisionHandling::addVTConstraint(ptA, ptB1, ptB2, ptB3, stiffnessA, stiffnessB);
    }
}
} // namespace imstk