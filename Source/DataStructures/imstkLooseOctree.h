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

#include "imstkMath.h"
#include "imstkParallelUtils.h"
#include "imstkLogUtility.h"

#include <array>
#include <cstdint>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace imstk
{
class OctreeNode;
class LooseOctree;
class Geometry;
class PointSet;
class SurfaceMesh;
class DebugRenderGeometry;

///
/// \brief The OctreePrimitiveType enum
/// Type of primitive stored in the octree
///
enum OctreePrimitiveType
{
    Point = 0,
    Triangle,
    AnalyticalGeometry,
    NumPrimitiveTypes
};

///
/// \brief The OctreePrimitiveData struct
/// For each octree primitive (point/triangle/analytical geometry), store its relevant data
///
struct OctreePrimitiveData
{
    OctreePrimitiveData() : m_pGeometry(nullptr), m_GeomIdx(0), m_Idx(0) {}
    OctreePrimitiveData(Geometry* const pGeometry, const uint32_t geomIdx, const uint32_t idx = 0) :
        m_pGeometry(pGeometry), m_GeomIdx(geomIdx), m_Idx(idx) {}

    Geometry* const m_pGeometry;            ///> Pointer to the parent geometry that the primitive belong to
    const uint32_t m_GeomIdx;               ///> Global index of the parent geometry
    const uint32_t m_Idx;                   ///> Index of the primitive in the parent geometry (such as index of the triangle in a mesh)

    OctreeNode* m_pNode          = nullptr; ///> Pointer to the octree node containing the primitive
    OctreePrimitiveData* m_pNext = nullptr; ///> Pointer to the next node in the primitive list of the octree node

    union
    {
        std::array<Real, 3> m_Position; ///> For a point primitive, store its position
        struct
        {
            std::array<Real, 3> m_LowerCorner; ///> For a non-point primitive, store its AABB's lower corner
            std::array<Real, 3> m_UpperCorner; ///> For a non-point primitive, store its AABB's upper corner
        };
    };

    /// Flag to keep track of primitive validity
    /// During tree update, set it to true if the primitive is still contained in the tree node that it has previously been inserted to
    /// and:
    ///     1) The depth of current node reaches maxDepth, or
    ///     1) The primitive straddles over multiple children nodes thus it cannot be passed down further to any child node
    bool m_bValid = true;
};

/// Forward declaration
struct OctreeNodeBlock;

///
/// \brief The OctreeNode class
///
class OctreeNode
{
friend class LooseOctree;
friend class LooseOctreeTest;
public:
    ///
    /// \brief Dummy constructor, called only during memory allocation in memory pool
    ///
    OctreeNode() :
        m_pTree(nullptr),
        m_pParent(nullptr),
        m_Center(Vec3r(0, 0, 0)),
        m_HalfWidth(0),
        m_Depth(0),
        m_MaxDepth(0),
        m_bIsLeaf(true) {}

    ///
    /// \brief OctreeNode constructor, called during node splitting when initializing children node
    ///
    explicit OctreeNode(LooseOctree* const tree, OctreeNode* const pParent, const Vec3r& nodeCenter,
                        const Real halfWidth, const uint32_t depth);

    ///
    /// \brief Check if this node is a leaf node
    ///
    bool isLeaf() const { return m_bIsLeaf; }

    ///
    /// \brief Get a child node
    /// \param childIdx The index of child node (from 0 to 7)
    ///
    OctreeNode* getChildNode(const uint32_t childIdx) const;

    ///
    /// \brief For the given primitive type, return the head node of the primitive list of that type
    ///
    OctreePrimitiveData* getPrimitiveList(const OctreePrimitiveType type) const { return m_pPrimitiveListHeads[type]; }

    ///
    /// \brief Get the number of primitives of the given type in this node
    ///
    uint32_t getPrimitiveCount(const OctreePrimitiveType type) const { return m_PrimitiveCounts[type]; }

    ///
    /// \brief Recursively clear primitive data (linked lists and counters)
    /// Note that the primitives are still exist in the octree primitive list, they are just removed from the node
    /// \param type The type of primitives that will be clear
    ///
    void clearPrimitiveData(const OctreePrimitiveType type);

    ///
    /// \brief Split node (requesting 8 children nodes from memory pool)
    ///
    void split();

    ///
    /// \brief Recursively remove all descendant nodes (return them back to memory pool)
    /// As a result, after calling to this function, the current node will become a leaf node
    ///
    void removeAllDescendants();

    ///
    /// \brief Recursively remove all descendant nodes that do not contain primitives (all 8 children of a node are removed at the same time)
    ///
    void removeEmptyDescendants();

    ///
    /// \brief Keep the primitive at this node as cannot pass it down further to any child node
    ///
    void keepPrimitive(OctreePrimitiveData* const pPrimitive, const OctreePrimitiveType type);

    ///
    /// \brief Insert a point primitive into the subtree in a top-down manner
    ///
    void insertPoint(OctreePrimitiveData* const pPrimitive);

    ///
    /// \brief Insert a non-point primitive into the subtree in a top-down manner
    ///
    void insertNonPointPrimitive(OctreePrimitiveData* const pPrimitive, const OctreePrimitiveType type);

    ///
    /// \brief Check if the given point is contained exactly in the node boundary (bounding box)
    ///
    bool contains(const Vec3r& point) { return contains(point[0], point[1], point[2]); }
    bool contains(const std::array<Real, 3>& point) { return contains(point[0], point[1], point[2]); }
    bool contains(const Real x, const Real y, const Real z)
    {
        return x >= m_LowerBound[0]
               && y >= m_LowerBound[1]
               && z >= m_LowerBound[2]
               && x <= m_UpperBound[0]
               && y <= m_UpperBound[1]
               && z <= m_UpperBound[2];
    }

    ///
    /// \brief Check if the given non-point primitive (triangle/analytical geometry) is exactly contained in the node boundary (bounding box)
    /// \param lowerCorner The AABB's lower corner of the primitive
    /// \param upperCorner The AABB's upper corner of the primitive
    ///
    bool contains(const std::array<Real, 3>& lowerCorner, const std::array<Real, 3>& upperCorner)
    {
        return lowerCorner[0] >= m_LowerBound[0]
               && lowerCorner[1] >= m_LowerBound[1]
               && lowerCorner[2] >= m_LowerBound[2]
               && upperCorner[0] <= m_UpperBound[0]
               && upperCorner[1] <= m_UpperBound[1]
               && upperCorner[2] <= m_UpperBound[2];
    }

    ///
    /// \brief Check if the given point is contained in the node loose boundary (which is 2X bigger than the bounding box)
    ///
    bool looselyContains(const Vec3r& point) { return looselyContains(point[0], point[1], point[2]); }
    bool looselyContains(const std::array<Real, 3>& point) { return looselyContains(point[0], point[1], point[2]); }
    bool looselyContains(const Real x, const Real y, const Real z)
    {
        return x >= m_LowerExtendedBound[0]
               && y >= m_LowerExtendedBound[1]
               && z >= m_LowerExtendedBound[2]
               && x <= m_UpperExtendedBound[0]
               && y <= m_UpperExtendedBound[1]
               && z <= m_UpperExtendedBound[2];
    }

    ///
    /// \brief Check if the given non-point primitive (triangle/analytical geometry) is contained in the node loose boundary
    /// (which is 2X bigger than the bounding box)
    /// \param lowerCorner The AABB's lower corner of the primitive
    /// \param upperCorner The AABB's upper corner of the primitive
    ///
    bool looselyContains(const std::array<Real, 3>& lowerCorner, const std::array<Real, 3>& upperCorner)
    {
        return lowerCorner[0] >= m_LowerExtendedBound[0]
               && lowerCorner[1] >= m_LowerExtendedBound[1]
               && lowerCorner[2] >= m_LowerExtendedBound[2]
               && upperCorner[0] <= m_UpperExtendedBound[0]
               && upperCorner[1] <= m_UpperExtendedBound[1]
               && upperCorner[2] <= m_UpperExtendedBound[2];
    }

    ///
    /// \brief Check if the bounding box of the given primitive (triangle/analytical geometry) overlaps with the loose boundary of this tree node
    /// (which is 2X bigger than the bounding box)
    /// \param lowerCorner The AABB's lower corner of the primitive
    /// \param upperCorner The AABB's upper corner of the primitive
    ///
    bool looselyOverlaps(const std::array<Real, 3>& lowerCorner, const std::array<Real, 3>& upperCorner)
    {
        return upperCorner[0] >= m_LowerExtendedBound[0]
               && upperCorner[1] >= m_LowerExtendedBound[1]
               && upperCorner[2] >= m_LowerExtendedBound[2]
               && lowerCorner[0] <= m_UpperExtendedBound[0]
               && lowerCorner[1] <= m_UpperExtendedBound[1]
               && lowerCorner[2] <= m_UpperExtendedBound[2];
    }

    ///
    /// \brief Recursively update debug geometry by adding lines drawing bounding boxes of the active nodes
    /// \return True if debug lines have been added to visualize the bounding box of the current node
    ///
    bool updateDebugGeometry();

private:
    LooseOctree*     m_pTree;               ///> Pointer to the octree, used to request children from memory pool during splitting node
    OctreeNode*      m_pParent;             ///> Pointer to the parent node
    OctreeNodeBlock* m_pChildren = nullptr; ///> Pointer to a memory block containing 8 children nodes

    const Vec3r    m_Center;                ///> Center of this node
    const Vec3r    m_LowerBound;            ///> The AABB's lower corner of the node
    const Vec3r    m_UpperBound;            ///> The AABB's upper corner of the node
    const Vec3r    m_LowerExtendedBound;    ///> The extended AABB's lower corner of the node, which is 2X bigger than the exact AABB
    const Vec3r    m_UpperExtendedBound;    ///> The extended AABB's upper corner of the node, which is 2X bigger than the exact AABB
    const Real     m_HalfWidth;             ///> Half width of the node AABB
    const uint32_t m_Depth;                 ///> Depth of this node (depth > 0, depth = 1 starting at the root node)
    uint32_t       m_MaxDepth;              ///> Cache the max depth of the tree (maximum depth level possible)
    bool           m_bIsLeaf = true;        ///> True if this node does not have any child node (a node should have either 0 or 8 children)

    /// Heads of the link lists storing (Classified) primitives
    OctreePrimitiveData* m_pPrimitiveListHeads[OctreePrimitiveType::NumPrimitiveTypes];

    /// Count the number of (classified) primitives stored in this node
    uint32_t m_PrimitiveCounts[OctreePrimitiveType::NumPrimitiveTypes];

    ParallelUtils::SpinLock m_PrimitiveLock[OctreePrimitiveType::NumPrimitiveTypes]; ///> Mutex lock for thread-safe primitive list modification
    ParallelUtils::SpinLock m_NodeSplitingLock;                                      ///> Mutex lock for thread-safe splitting node
};

///
/// \brief The OctreeNodeBlock struct
/// This is a data structure to store a memory block of 8 tree node at a time
/// Using a block of 8 nodes at a time can reduce node allocation/merging/slitting overhead
///
struct OctreeNodeBlock
{
    OctreeNode m_Nodes[8];
    OctreeNodeBlock* m_NextBlock = nullptr; ///> Pointer to the next block in the memory pool
};

///
/// \brief Class LooseOctree, where each tree node has a loose boundary which is exactly twice big as its exact, tight boundary
/// During tree update, a primitive is moved around from node to node
/// If removed from a node, the primitive is moving up to find the lowest node that tightly contains it,
/// then it is inserted again from top-down, to a lowest level possible, stopping at a node that loosely contains it
/// Pointer variables in tree and tree node are all raw pointers, not smart pointers, for fast operations
///
class LooseOctree
{
friend class OctreeNode;
friend class LooseOctreeTest;
public:
    ///
    /// \brief Octree constructor
    /// \param center The center of the tree, which also is the center of the root node
    /// \param width Width of the octree bounding box
    /// \param minWidth Minimum allowed width of the tree nodes, valid only if there are only points primitives
    /// \param minWidthRatio If there is primitive that is not a point, minWidth will be recomputed as minWidth = min(width of all non-point primitives) * minWidthRatio
    /// \param name Name of the octree
    ///
    explicit LooseOctree(const Vec3r& center, const Real width, const Real minWidth,
                         const Real minWidthRatio = 1.0, const std::string name = "LooseOctree");

    ///
    /// Destructor, doing memory cleanup
    ///
    virtual ~LooseOctree();

    ///
    /// \brief Clear all primitive and geometry data, but still keep allocated nodes in memory pool to recycle
    ///
    virtual void clear();

    ///
    /// \brief Completely remove all data of the given primitive type in the tree
    ///
    void clearPrimitive(const OctreePrimitiveType type);

    ///
    /// \brief Return center of the tree
    ///
    const Vec3r getCenter() const { return m_Center; }

    ///
    /// \brief Return width of the tree
    ///
    Real getWidth() const { return m_Width; }

    ///
    /// \brief Return width of the lowest level tree nodes
    ///
    Real getMinWidth() const { return m_MinWidth; }

    ///
    /// \brief Get the maximum depth in this tree, which is computed based on the minWidth value
    ///
    uint32_t getMaxDepth() const { return m_MaxDepth; }

    ///
    /// \brief Get the total number of tree nodes that have been allocated in memory
    ///
    uint32_t getNumAllocatedNodes() const { return m_NumAllocatedNodes; }

    ///
    /// \brief Get the number of active nodes in the tree (the non-leaf nodes or leaf nodes that contain primitives)
    ///
    uint32_t getNumActiveNodes() const { return m_NumAllocatedNodes - m_NumAvaiableBlocksInPool * 8u; }

    ///
    /// \brief Get the root node
    ///
    OctreeNode* getRootNode() const { return m_pRootNode; }

    ///
    /// \brief Get the number of primitives of the given type
    ///
    size_t getPrimitiveCount(const OctreePrimitiveType type) const { return m_vPrimitivePtrs[type].size(); }

    ///
    /// \brief Count the maximum number of primitives stored in a tree node
    ///
    uint32_t getMaxNumPrimitivesInNodes() const;

    ///
    /// \brief Add a PointSet geometry into the tree
    /// (the points will not be populated to tree nodes until calling to build())
    ///
    uint32_t addPointSet(const std::shared_ptr<PointSet>& pointset);

    ///
    /// \brief Add a triangle mesh into the tree
    /// (the triangles of the mesh will not be populated to tree nodes until calling to build())
    ///
    uint32_t addTriangleMesh(const std::shared_ptr<SurfaceMesh>& surfMesh);

    ///
    /// \brief Add an analytical geometry (such as plane/sphere/cube etc) into the tree
    /// (it will not be populated to tree nodes until calling to build())
    ///
    uint32_t addAnalyticalGeometry(const std::shared_ptr<Geometry>& geometry);

    ///
    /// \brief Set the alwaysRebuild flag (true: rebuilt the tree from scratch in every update, false: incrementally update from the current state)
    ///
    void setAlwaysRebuild(const bool bAlwaysRebuild) { m_bAlwaysRebuild = bAlwaysRebuild; }

    ///
    /// \brief Build octree from the provided geometries
    ///
    void build();

    ///
    /// \brief Update tree (the tree is rebuilt from scratch if m_bAlwaysRebuild is true, otherwise it is incrementally updated)
    ///
    void update();

    ///
    /// \brief Generate the debug geometry for debug rendering (a bounding box for each node)
    /// \param maxLevel The tree will be visualized up to maxLevel levels
    ///
    std::shared_ptr<DebugRenderGeometry> getDebugGeometry(const uint32_t maxLevel, bool bDrawNonEmptyParent = true);

    ///
    /// \brief Update the debug geometry (bounding boxes) after updated tree (debug geometry must be previously initialized by getDebugGeometry())
    ///
    void updateDebugGeometry();

protected:
    ///
    /// \brief Add geometry to the internal geometry list to check for duplication
    ///
    void addGeometry(const uint32_t geomIdx);

    ///
    /// \brief Remove geometry from the internal geometry list (does nothing if the geometry does not exist, or has been removed before)
    ///
    void removeGeometry(const uint32_t geomIdx);

    ///
    /// \brief Rebuild the tree from scratch
    ///
    void rebuild();

    ///
    /// \brief Populate point primitive to tree nodes, from top (root node) down to leaf nodes
    ///
    void populatePointPrimitives();

    ///
    /// \brief Populate non-point primitive (triangle/analytical geometry) to tree nodes, from top (root node) down to leaf nodes
    /// \param type Type of primitive (must not point, but triangle/analytical geometry)
    ///
    void populateNonPointPrimitives(const OctreePrimitiveType type);

    ///
    /// \brief Incrementally update octree from current state
    ///
    void incrementalUpdate();

    ///
    /// \brief For each point primitive, update its position from its parent geometry and check if it is still loosely contained in the tree node
    /// If the primitive is not loosely contained in tree node, set it to invalid state and set m_pNode to the lowest ancestor node that tightly contains it
    ///
    void updatePositionAndCheckValidity();

    ///
    /// \brief For each non-point primitive, update its bounding box from its parent geometry and check if it is still loosely contained in the tree node
    /// If the primitive is not loosely contained in tree node, set it to invalid state and set m_pNode to the lowest ancestor node that tightly contains it
    ///
    void updateBoundingBoxAndCheckValidity(const OctreePrimitiveType type);

    ///
    /// \brief Remove all invalid primitives from the tree nodes previously contained them
    ///
    void removeInvalidPrimitivesFromNodes();

    ///
    /// \brief For each invalid primitive, insert it back to the tree in a top-down manner
    /// starting from the lowest ancestor node that tightly contains it (that node was found during validity check)
    ///
    void reinsertInvalidPrimitives(const OctreePrimitiveType type);

    ///
    /// \brief Compute the AABB bounding box of a non-point primitive
    ///
    void computePrimitiveBoundingBox(OctreePrimitiveData* const pPrimitive, const OctreePrimitiveType type);

    ///
    /// \brief Request a block of 8 tree nodes from memory pool (this is called only during splitting node)
    /// If the memory pool is exhausted, 64 more blocks will be allocated from the system memory
    ///
    OctreeNodeBlock* requestChildrenFromPool();

    ///
    /// \brief Return 8 children nodes to memory pool (this is called only during destroying descendant nodes)
    ///
    void returnChildrenToPool(OctreeNodeBlock* const pNodeBlock);

    ///
    /// \brief Pre-allocate a given number of node blocks (each block contains 8 nodes) and add them to the memory pool
    ///
    void allocateMoreNodeBlock(const uint32_t numBlocks);

    ///
    /// \brief Deallocate all node block in memory pool, called only during octree destructor
    ///
    void deallocateMemoryPool();

    const std::string m_Name;   ///> Name of the tree
    const Vec3r       m_Center; ///> Center of the tree
    const Real        m_Width;  ///> Width of the tree bounding box

    /// If there is no point primitive, minWidth will be recomputed as minWidth = min(width of all non-point primitives) * minWidthRatio
    const Real m_MinWidthRatio;

    Real     m_MinWidth;                                         ///> Minimum width allowed for the tree nodes
    uint32_t m_MaxDepth;                                         ///> Max depth of the tree, which is computed based on m_MinWidth

    OctreeNode* const       m_pRootNode;                         ///> Root node, should not be reassigned throughout the existence of the tree
    OctreeNodeBlock*        m_pNodeBlockPoolHead      = nullptr; ///> The pool of tree nodes, storing pre-allocated nodes as a linked list
    uint32_t                m_NumAvaiableBlocksInPool = 0;       ///> Count the number of nodes available in memory pool
    uint32_t                m_NumAllocatedNodes;                 ///> Count the total number of allocated nodes so far
    ParallelUtils::SpinLock m_PoolLock;                          ///> Atomic lock for multi-threading modification of the memory pool

    /// Set of node blocks that are in use (node blocks that have been taken from memory pool)
    tbb::concurrent_unordered_set<OctreeNodeBlock*> m_sActiveTreeNodeBlocks;

    /// During memory allocation for tree nodes, multiple node blocks are allocated at the same time from a big memory block
    /// This variable store the first address of such big memory block, used during memory pool deallocation
    std::vector<OctreeNodeBlock*> m_pNodeBigBlocks;

    /// Store pointers of primitives created from geometry elements, such as points, triangles, analytical geometries
    std::vector<OctreePrimitiveData*> m_vPrimitivePtrs[OctreePrimitiveType::NumPrimitiveTypes];

    /// During memory allocation for primitives, multiple primitives are allocated at the same time from a big memory block
    /// This variable store the first address of such big memory block, used during primitive deallocation
    std::vector<OctreePrimitiveData*> m_pPrimitiveBlocks[OctreePrimitiveType::NumPrimitiveTypes];

    /// List of all indices of the added geometries, to check for duplication such that one geometry cannot be mistakenly added multiple times
    std::unordered_set<uint32_t> m_sGeometryIndices;

    bool m_bAlwaysRebuild = false;                        ///> If true, the octree is always be rebuit from scratch every time calling to update()
    bool m_bCompleteBuild = false;                        ///> This is set to true after tree has been built, otherwise false

    std::shared_ptr<DebugRenderGeometry> m_DebugGeometry; ///> Debug geometry, for debug rendering only
    uint32_t m_MaxLevelDebugRender;                       ///> Maximum level of nodes that will be rendered during debug rendering

    /// If true, all non-empty nodes are rendered during debug rendering (including nodes containing primitives and all other non-leaf nodes)
    /// otherwise only nodes containing primitives are rendered
    bool m_bDrawNonEmptyParent = true;
};
} // end namespace imstk
