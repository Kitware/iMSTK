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

#include "imstkOctreeBasedCD.h"
#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkGeometry.h"
#include "imstkNarrowPhaseCD.h"

namespace imstk
{
void
OctreeBasedCD::clear()
{
    LooseOctree::clear();
    m_mCollisionPair2AssociatedData.clear();
    m_vCollidingGeomPairs.clear();
    m_sCollidingPrimitiveTypes = 0u;
}

bool
OctreeBasedCD::hasCollisionPair(const uint32_t geomIdx1, const uint32_t geomIdx2)
{
    const auto collisionPair = computeCollisionPairHash(geomIdx1, geomIdx2);
    return m_mCollisionPair2AssociatedData.find(collisionPair) != m_mCollisionPair2AssociatedData.end();
}

void
OctreeBasedCD::addCollisionPair(const std::shared_ptr<Geometry>& geom1, const std::shared_ptr<Geometry>& geom2,
                                const CollisionDetection::Type collisionType,
                                const std::shared_ptr<CollisionData>& collisionData)
{
    // Collision pairs are encoded as 64 bit unsigned integer
    // The first 32 bit is obj1Idx, following by 32 bit of obj2Idx
    const auto objIdx1       = geom1->getGlobalIndex();
    const auto objIdx2       = geom2->getGlobalIndex();
    const auto collisionPair = computeCollisionPairHash(objIdx1, objIdx2);

    LOG_IF(FATAL, (m_mCollisionPair2AssociatedData.find(collisionPair) != m_mCollisionPair2AssociatedData.end()))
        << "Collision pair has previously been added";

    m_mCollisionPair2AssociatedData[collisionPair] = { collisionType, collisionData };
    m_vCollidingGeomPairs.push_back({ geom1.get(), geom2.get() });

    const auto geomType1 = geom1->getType();
    const auto geomType2 = geom2->getType();

    if (geomType1 == Geometry::Type::PointSet
        || geomType2 == Geometry::Type::PointSet)
    {
        const uint32_t mask = 1 << static_cast<int>(OctreePrimitiveType::Point);
        m_sCollidingPrimitiveTypes |= mask;
    }

    if (geomType1 == Geometry::Type::SurfaceMesh
        || geomType2 == Geometry::Type::SurfaceMesh)
    {
        const uint32_t mask = 1 << static_cast<int>(OctreePrimitiveType::Triangle);
        m_sCollidingPrimitiveTypes |= mask;
    }

    if (geomType1 != Geometry::Type::PointSet
        || geomType2 != Geometry::Type::PointSet
        || geomType1 != Geometry::Type::SurfaceMesh
        || geomType2 != Geometry::Type::SurfaceMesh)
    {
        const uint32_t mask = 1 << static_cast<int>(OctreePrimitiveType::AnalyticalGeometry);
        m_sCollidingPrimitiveTypes |= mask;
    }

    LOG(INFO) << m_Name << ":: Add collision pair between objects '"
              << geom1->getName() << "' (ID = " << objIdx1 << ") and '"
              << geom2->getName() << "' (ID = " << objIdx2 << ")";
}

const std::shared_ptr<CollisionData>&
OctreeBasedCD::getCollisionPairData(const uint32_t geomIdx1, const uint32_t geomIdx2)
{
    const auto collisionPair = computeCollisionPairHash(geomIdx1, geomIdx2);
    const auto it = m_mCollisionPair2AssociatedData.find(collisionPair);

    LOG_IF(FATAL, (it == m_mCollisionPair2AssociatedData.end())) << "Collision pair does not exist";
    return it->second.m_CollisionData;
}

void
OctreeBasedCD::detectCollision()
{
    for (auto& kv : m_mCollisionPair2AssociatedData)
    {
        // Clear all collision data
        kv.second.m_CollisionData->clearAll();
    }

    // Clear invalid flags for point-mesh collision pairs
    m_mInvalidPointMeshCollisions.clear();

    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
    {
        const auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
        if (vPrimitivePtrs.size() > 0 && hasCollidingPrimitive(type))
        {
            ParallelUtils::parallelFor(vPrimitivePtrs.size(),
                [&](const size_t idx)
                {
                    const auto pPrimitive = vPrimitivePtrs[idx];
                    if (type == OctreePrimitiveType::Point)
                    {
                        checkPointWithSubtree(m_pRootNode, pPrimitive, pPrimitive->m_GeomIdx);
                    }
                    else
                    {
                        const auto& lowerCorner = pPrimitive->m_LowerCorner;
                        const auto& upperCorner = pPrimitive->m_UpperCorner;
                        const Vec3r center(
                            (lowerCorner[0] + upperCorner[0]) * 0.5,
                            (lowerCorner[1] + upperCorner[1]) * 0.5,
                            (lowerCorner[2] + upperCorner[2]) * 0.5);
                        checkNonPointWithSubtree(m_pRootNode, pPrimitive, pPrimitive->m_GeomIdx,
                                                 lowerCorner, upperCorner, static_cast<OctreePrimitiveType>(type));
                    }
                });
        }
    }

    // Remove all invalid collision between point-mesh
    for (auto& geoPair: m_vCollidingGeomPairs)
    {
        // Ignore the collision pair if it is not a PointSet-SurfaceMesh pair
        if (geoPair.first->getType() != Geometry::Type::PointSet || (geoPair.second->getType() != Geometry::Type::SurfaceMesh))
        {
            continue;
        }

        auto& collisionData = getCollisionPairData(geoPair.first->getGlobalIndex(), geoPair.second->getGlobalIndex());
        if (collisionData->VTColData.getSize() == 0)
        {
            continue;
        }

        // Sort the vertex-triangle collision data by verter index
        // If one vertex collides with many triangles, sort the data of those collisions by closest distance
        collisionData->VTColData.sort(
            [](const VertexTriangleCollisionDataElement& x,
               const VertexTriangleCollisionDataElement& y)
            {
                return (x.vertexIdx < y.vertexIdx)
                || ((x.vertexIdx == y.vertexIdx) && x.closestDistance < y.closestDistance);
            });

        const auto geomIdxPointSet = geoPair.first->getGlobalIndex();
        const auto geomIdxMesh     = geoPair.second->getGlobalIndex();
        size_t     writeIdx = 0;

        // Check if the first data elemnt is valid
        if (pointStillColliding(collisionData->VTColData[0].vertexIdx, geomIdxPointSet, geomIdxMesh))
        {
            ++writeIdx;
        }

        // From the second data element, check for valid and duplication
        for (size_t readIdx = 1; readIdx < collisionData->VTColData.getSize(); ++readIdx)
        {
            const auto& vt = collisionData->VTColData[readIdx];
            if (pointStillColliding(vt.vertexIdx, geomIdxPointSet, geomIdxMesh)
                && (writeIdx == 0 || collisionData->VTColData[writeIdx - 1].vertexIdx != vt.vertexIdx))
            {
                if (readIdx != writeIdx)
                {
                    collisionData->VTColData.setElement(writeIdx, collisionData->VTColData[readIdx]);
                }
                ++writeIdx;
            }
        }
        collisionData->VTColData.resize(writeIdx);
    }
}

void
OctreeBasedCD::checkPointWithSubtree(OctreeNode* const pNode, OctreePrimitive* const pPrimitive, const uint32_t geomIdx)
{
    if (!pNode->looselyContains(pPrimitive->m_Position))
    {
        return;
    }

    if (!pNode->isLeaf())
    {
        for (uint32_t childIdx = 0; childIdx < 8u; childIdx++)
        {
            OctreeNode* const pChildNode = &pNode->m_pChildren->m_Nodes[childIdx];
            checkPointWithSubtree(pChildNode, pPrimitive, geomIdx);
        }
    }

    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
    {
        // Points do not collide with points
        if (type == OctreePrimitiveType::Point)
        {
            continue;
        }

        auto pIter = pNode->m_pPrimitiveListHeads[type];
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        uint32_t count = 0;
#endif

        while (pIter)
        {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
            ++count;
#endif

            if (pPrimitive != pIter)
            {
                const auto geomIdxOther = pIter->m_GeomIdx;
                if (pointStillColliding(pPrimitive->m_Idx, geomIdx, geomIdxOther))
                {
                    const auto  collisionPair = computeCollisionPairHash(geomIdx, geomIdxOther);
                    const auto& collisionAssociatedData = getCollisionPairAssociatedData(collisionPair);
                    if (collisionAssociatedData.m_CollisionData != nullptr)
                    {
                        checkPointWithPrimitive(pPrimitive, pIter, collisionAssociatedData);
                    }
                }
            }
            pIter = pIter->m_pNext;
        }

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (count != pNode->m_PrimitiveCounts[type])) << "Internal data corrupted";
#endif
    }
}

void
OctreeBasedCD::checkNonPointWithSubtree(OctreeNode* const pNode, OctreePrimitive* const pPrimitive,
                                        const uint32_t geomIdx, const std::array<Real, 3>& lowerCorner, const std::array<Real, 3>& upperCorner,
                                        const OctreePrimitiveType type)
{
    if (!pNode->looselyOverlaps(lowerCorner, upperCorner))
    {
        return;
    }

    if (!pNode->isLeaf())
    {
        for (uint32_t childIdx = 0; childIdx < 8u; childIdx++)
        {
            OctreeNode* const pChildNode = &pNode->m_pChildren->m_Nodes[childIdx];
            checkNonPointWithSubtree(pChildNode, pPrimitive, geomIdx, lowerCorner, upperCorner, type);
        }
    }

    for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
    {
        auto pIter = pNode->m_pPrimitiveListHeads[i];
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        uint32_t count = 0;
#endif

        while (pIter)
        {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
            ++count;
#endif

            if (pPrimitive != pIter)
            {
                // \todo: this is necessary but not help
                const auto geomIdxIter   = pIter->m_GeomIdx;
                const auto collisionPair = computeCollisionPairHash(geomIdx, geomIdxIter);
                const auto collisionAssociatedData = getCollisionPairAssociatedData(collisionPair);
                if (collisionAssociatedData.m_CollisionData != nullptr)     // Has collision pair
                {
                    const auto& lowerCornerIter = pIter->m_LowerCorner;
                    const auto& upperCornerIter = pIter->m_UpperCorner;
                    const Vec3r centerIter(
                        (lowerCorner[0] + upperCorner[0]) * 0.5,
                        (lowerCorner[1] + upperCorner[1]) * 0.5,
                        (lowerCorner[2] + upperCorner[2]) * 0.5);

                    if (CollisionUtils::testAABBToAABB(lowerCorner[0], upperCorner[0],
                                           lowerCorner[1], upperCorner[1],
                                           lowerCorner[2], upperCorner[2],
                                           lowerCornerIter[0], upperCornerIter[0],
                                           lowerCornerIter[1], upperCornerIter[1],
                                           lowerCornerIter[2], upperCornerIter[2]))
                    {
                        checkNonPointWithPrimitive(pPrimitive, pIter, collisionAssociatedData);
                    }
                }
            }
            pIter = pIter->m_pNext;
        }

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        LOG_IF(FATAL, (count != pNode->m_PrimitiveCounts[i])) << "Internal data corrupted";
#endif
    }
}

void
OctreeBasedCD::checkPointWithPrimitive(OctreePrimitive* const pPrimitive1, OctreePrimitive* const pPrimitive2,
                                       const CollisionPairAssociatedData& collisionAssociatedData)
{
    const auto  collisionType = collisionAssociatedData.m_Type;
    const auto& collisionData = collisionAssociatedData.m_CollisionData;
    const auto  point    = Vec3r(pPrimitive1->m_Position[0], pPrimitive1->m_Position[1], pPrimitive1->m_Position[2]);
    const auto  pointIdx = pPrimitive1->m_Idx;

    switch (collisionType)
    {
    case CollisionDetection::Type::PointSetToSurfaceMesh:
        if (!NarrowPhaseCD::pointToTriangle(point, pointIdx, pPrimitive2->m_Idx, pPrimitive2->m_pGeometry, collisionData))
        {
            setPointMeshCollisionInvalid(pointIdx, pPrimitive1->m_GeomIdx, pPrimitive2->m_GeomIdx);
        }
        break;

    case CollisionDetection::Type::PointSetToSphere:
        NarrowPhaseCD::pointToSphere(point, pointIdx, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::PointSetToPlane:
        NarrowPhaseCD::pointToPlane(point, pointIdx, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::PointSetToCapsule:
        NarrowPhaseCD::pointToCapsule(point, pointIdx, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::PointSetToSpherePicking:
        NarrowPhaseCD::pointToSpherePicking(point, pointIdx, pPrimitive2->m_pGeometry, collisionData);
        break;

    default:
        LOG(FATAL) << "Unsupported collision type!";
    }
}

void
OctreeBasedCD::checkNonPointWithPrimitive(OctreePrimitive* const pPrimitive1, OctreePrimitive* const pPrimitive2,
                                          const CollisionPairAssociatedData& collisionAssociatedData)
{
    const auto  collisionType = collisionAssociatedData.m_Type;
    const auto& collisionData = collisionAssociatedData.m_CollisionData;

    switch (collisionType)
    {
    // Mesh to mesh
    case CollisionDetection::Type::SurfaceMeshToSurfaceMesh:
        NarrowPhaseCD::triangleToTriangle(pPrimitive1->m_Idx, pPrimitive1->m_pGeometry,
                                          pPrimitive2->m_Idx, pPrimitive2->m_pGeometry,
                                          collisionData);
        break;

    // Analytical object to analytical object
    case CollisionDetection::Type::UnidirectionalPlaneToSphere:
        NarrowPhaseCD::unidirectionalPlaneToSphere(pPrimitive1->m_pGeometry, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::BidirectionalPlaneToSphere:
        NarrowPhaseCD::bidirectionalPlaneToSphere(pPrimitive1->m_pGeometry, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::SphereToCylinder:
        NarrowPhaseCD::sphereToCylinder(pPrimitive1->m_pGeometry, pPrimitive2->m_pGeometry, collisionData);
        break;
    case CollisionDetection::Type::SphereToSphere:
        NarrowPhaseCD::sphereToSphere(pPrimitive1->m_pGeometry, pPrimitive2->m_pGeometry, collisionData);
        break;

    default:
        LOG(FATAL) << "Unsupported collision type!";
    }
}

uint64_t
OctreeBasedCD::computeCollisionPairHash(const uint32_t objIdx1, const uint32_t objIdx2)
{
    const uint64_t uint64Idx1 = static_cast<uint64_t>(objIdx1);
    const uint64_t uint64Idx2 = static_cast<uint64_t>(objIdx2);
    return (uint64Idx1 << 32) | uint64Idx2;
}

const OctreeBasedCD::CollisionPairAssociatedData&
OctreeBasedCD::getCollisionPairAssociatedData(const uint64_t collisionPair) const
{
    static const auto invalidData = CollisionPairAssociatedData { CollisionDetection::Type::Custom, nullptr };
    const auto        it = m_mCollisionPair2AssociatedData.find(collisionPair);
    return (it != m_mCollisionPair2AssociatedData.end()) ?  it->second : invalidData;
}

bool
OctreeBasedCD::pointStillColliding(const uint32_t primitiveIdx, const uint32_t geometryIdx,
                                   const uint32_t otherGeometryIdx)
{
    const uint64_t uint64PrimitiveIdx = static_cast<uint64_t>(primitiveIdx);
    const uint64_t uint64GeometryIdx  = static_cast<uint64_t>(geometryIdx);
    const uint64_t source = (uint64PrimitiveIdx << 32) | uint64GeometryIdx;

    auto it = m_mInvalidPointMeshCollisions.find(source);
    if (it == m_mInvalidPointMeshCollisions.end())
    {
        return true;
    }
    const auto& invalidTargets = it->second;
    return invalidTargets.find(otherGeometryIdx) == invalidTargets.end();
}

void
OctreeBasedCD::setPointMeshCollisionInvalid(const uint32_t primitiveIdx, const uint32_t geometryIdx,
                                            const uint32_t otherGeometryIdx)
{
    const uint64_t uint64PrimitiveIdx = static_cast<uint64_t>(primitiveIdx);
    const uint64_t uint64GeometryIdx  = static_cast<uint64_t>(geometryIdx);
    const uint64_t source = (uint64PrimitiveIdx << 32) | uint64GeometryIdx;
    m_mInvalidPointMeshCollisions[source].insert(otherGeometryIdx);
}
} // end namespace imstk
