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

#include "imstkLooseOctree.h"
#include "imstkCollisionDetection.h"

namespace imstk
{
struct CollisionData;

///
/// \brief Class OctreeBasedCD, a subclass of LooseOctree which supports collision detection between octree primitives
///
class OctreeBasedCD : public LooseOctree
{
friend class OctreeBasedCDTest;
public:
    OctreeBasedCD(const OctreeBasedCD&) = delete;
    OctreeBasedCD& operator=(const OctreeBasedCD&) = delete;

    ///
    /// \brief OctreeBasedCD
    /// \param center The center of the tree, which also is the center of the root node
    /// \param width Width of the octree bounding box
    /// \param minWidth Minimum allowed width of the tree nodes, valid only if there are only points primitives
    /// \param minWidthRatio If there is primitive that is not a point, minWidth will be recomputed as minWidth = min(width of all non-point primitives) * minWidthRatio
    /// \param name Name of the octree
    ///
    explicit OctreeBasedCD(const Vec3r& center, const Real width, const Real minWidth,
                           const Real minWidthRatio = 1.0, const std::string name = "OctreeBasedCD") :
        LooseOctree(center, width, minWidth, minWidthRatio, name) {}

    ///
    /// \brief Clear all primitive and geometry and collision data, but still keep nodes data in memory pool
    ///
    virtual void clear() override;

    ///
    /// \brief Get number of collision pairs that have been added to the octree
    ///
    size_t getNumCollisionPairs() const { return m_vCollidingGeomPairs.size(); }

    ///
    /// \brief Check if the collision pair has previously been added to the octree
    ///
    bool hasCollisionPair(const uint32_t geomIdx1, const uint32_t geomIdx2);

    ///
    /// \brief Define a collision pair between two geometry objects
    /// The collisionType parameter must be valid (no check), otherwise will result in undefined behaviors
    ///
    void addCollisionPair(const std::shared_ptr<Geometry>& geom1, const std::shared_ptr<Geometry>& geom2,
                          const CollisionDetection::Type collisionType,
                          const std::shared_ptr<CollisionData>& collisionData = std::make_shared<CollisionData>());

    ///
    /// \brief Get pairs of geometries from the added collision pairs
    /// \return List of pairs of geometries, each pair corresponds to an added collision pair
    ///
    const std::vector<std::pair<Geometry*, Geometry*>> getCollidingGeometryPairs() const { return m_vCollidingGeomPairs; }

    ///
    /// \brief Check for collision between pritimives in the tree, based on the provided collision pairs
    ///
    void detectCollision();

    ///
    /// \brief Get CollisionData for a collision pair between two geometries
    /// (That collision pair must be added before accessing collision data by this function)
    /// For performance reason, to avoid casting between geomery pointers, the function only accepts global indices of geometries
    /// \param geomeIdx1 Global index of the first geometry
    /// \param geomeIdx2 Global index of the second geometry
    ///
    const std::shared_ptr<CollisionData>& getCollisionPairData(const uint32_t geomIdx1, const uint32_t geomIdx2);

private:
    ///
    /// \brief The CollisionPairAssociatedData struct
    /// For each collision pair, map it with a collision type and collision data
    ///
    struct CollisionPairAssociatedData
    {
        CollisionDetection::Type m_Type;
        std::shared_ptr<CollisionData> m_CollisionData;
    };

    ///
    /// \brief Check for collisions of the given point primitive with primitives in the subtree rooting at the given tree node
    /// The collision checks are not brute-force, since the (loose) bounding boxes of the tree nodes are use to prune unnecessary checks
    ///
    void checkPointWithSubtree(OctreeNode* const pNode, OctreePrimitive* const pPrimitive, const uint32_t geomIdx);

    ///
    /// \brief Check for collisions of the given non-point primitive with primitives in the subtree rooting at the given tree node
    /// The collision checks are not brute-force, since the (loose) bounding boxes of the tree nodes are use to prune unnecessary checks
    ///
    void checkNonPointWithSubtree(OctreeNode* const pNode, OctreePrimitive* const pPrimitive, const uint32_t geomIdx,
                                  const std::array<Real, 3>& lowerCorner, const std::array<Real, 3>& upperCorner,
                                  const OctreePrimitiveType type);

    ///
    /// \brief Check for narrow-phase collision between a point primitive with another primitive
    ///
    void checkPointWithPrimitive(OctreePrimitive* const pPrimitive1, OctreePrimitive* const pPrimitive2,
                                 const CollisionPairAssociatedData& collisionAssociatedData);
    ///
    /// \brief Check for narrow-phase collision between a non-point primitive with another primitive
    ///
    void checkNonPointWithPrimitive(OctreePrimitive* const pPrimitive1, OctreePrimitive* const pPrimitive2,
                                    const CollisionPairAssociatedData& collisionAssociatedData);

    ///
    /// \brief Compute the hash value for a collision pair between two geometry objects
    /// The hash value is computed as concatenation of the two objects' global indices
    /// \param objIdx1 Global index of the first geometry
    /// \param objIdx2 Global index of the second geometry
    ///
    uint64_t computeCollisionPairHash(const uint32_t objIdx1, const uint32_t objIdx2);

    ///
    /// \brief Get associated data for a given collision pair
    /// \param collisionPair The hash value of the given collision pair
    ///
    const CollisionPairAssociatedData& getCollisionPairAssociatedData(const uint64_t collisionPair) const;

    ///
    /// \brief Return true if any of the added collision pairs contains primitives of the given type
    /// This is used to avoid unnecessary collision check
    /// For example, if there was pointset(s) added but there is only collision pair between triangle meshes
    /// then we can totally ignore all point primitives during collision detection
    ///
    bool hasCollidingPrimitive(const int type) const { return (m_sCollidingPrimitiveTypes & (1 << type)) != 0; }

    ///
    /// \brief Return true if the collision between a point and a triangle mesh is still valid
    /// This is applied specifically for point primitive
    /// When a point P is detected as above surface of a triangle ABC ( dot(P-A, n) > 0, where n is triangle normal pointing outward of the mesh)
    /// then that point is obviously outside of the mesh containiing the triangle ABC
    /// Thus, collisions between point P and all triangles of that mesh should be mark as invalid and discarded
    /// \param primitiveIdx Index of the given point in the pointset
    /// \param geometryIdx Global index of the parent pointset of the given point
    /// \param otherGeometryIdx Global index of the surface (triangle) mesh that the given point is colliding
    ///
    bool pointStillColliding(const uint32_t primitiveIdx, const uint32_t geometryIdx, const uint32_t otherGeometryIdx);

    ///
    /// \brief Mark all the collisions between a point and triangles of a surface mesh is invalid
    /// \param primitiveIdx Index of the given point in the pointset
    /// \param geometryIdx Global index of the parent pointset of the given point
    /// \param otherGeometryIdx Global index of the surface (triangle) mesh that the given point is colliding
    ///
    void setPointMeshCollisionInvalid(const uint32_t primitiveIdx, const uint32_t geometryIdx, const uint32_t otherGeometryIdx);

    /// For each collision pair, related primitives need to be marked as colliding
    /// (for example, for pointset-surface mesh collision pair, 'point' and 'triangle' are now 'colliding primitives')
    /// This variable is used to avoid unnecessary collision check
    /// For example, if there was pointset(s) added but there is only collision pair between triangle meshes
    /// then we can totally ignore all point primitives during collision detection
    uint32_t m_sCollidingPrimitiveTypes = 0u;

    using ThreadSafeInvalidCollidingSet    = tbb::concurrent_unordered_map<uint64_t, tbb::concurrent_unordered_set<uint32_t>>;
    using CollisionPairToAssociatedDataMap = tbb::concurrent_unordered_map<uint64_t, CollisionPairAssociatedData>;

    /// Map to check and mark all collisions from a given point to triangles of a surface mesh as invalid
    ThreadSafeInvalidCollidingSet m_mInvalidPointMeshCollisions;

    /// Contains pairs of geometries from the added collision pairs
    std::vector<std::pair<Geometry*, Geometry*>> m_vCollidingGeomPairs;

    /// Map from collision pair to collision type and collision data
    CollisionPairToAssociatedDataMap m_mCollisionPair2AssociatedData;
};
} // end namespace imstk
