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

#include "imstkLooseOctree.h"

#include "imstkGeometry.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkDebugRenderGeometry.h"

namespace imstk
{
OctreeNode::OctreeNode(LooseOctree* const tree, OctreeNode* const pParent, const Vec3r& nodeCenter,
                       const Real halfWidth, const uint32_t depth) :
    m_pTree(tree),
    m_pParent(pParent),
    m_Center(nodeCenter),
    m_LowerBound(nodeCenter - Vec3r(halfWidth, halfWidth, halfWidth)),
    m_UpperBound(nodeCenter + Vec3r(halfWidth, halfWidth, halfWidth)),
    m_LowerExtendedBound(nodeCenter - 2.0 * Vec3r(halfWidth, halfWidth, halfWidth)),
    m_UpperExtendedBound(nodeCenter + 2.0 * Vec3r(halfWidth, halfWidth, halfWidth)),
    m_HalfWidth(halfWidth),
    m_Depth(depth),
    m_MaxDepth(tree->m_MaxDepth),
    m_bIsLeaf(true)
{
    // Must initialize primitive linked lists and counters
    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
    {
        clearPrimitiveData(static_cast<OctreePrimitiveType>(type));
    }
}

OctreeNode*
OctreeNode::getChildNode(const uint32_t childIdx) const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (!m_pChildren)) << "Children node block is nullptr";
#endif
    return &m_pChildren->m_Nodes[childIdx];
}

void
OctreeNode::clearPrimitiveData(const OctreePrimitiveType type)
{
    m_pPrimitiveListHeads[type] = nullptr;
    m_PrimitiveCounts[type]     = 0;

    if (!isLeaf())
    {
        for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
        {
            m_pChildren->m_Nodes[childIdx].clearPrimitiveData(type);
        }
    }
}

void
OctreeNode::split()
{
    if (!isLeaf() || m_Depth == m_MaxDepth)
    {
        return;
    }

    m_NodeSplitingLock.lock();
    if (isLeaf())
    {
        //--------------------------------------------------------
        //
        //           6-------7
        //          /|      /|
        //         2-+-----3 |
        //         | |     | |   y
        //         | 4-----+-5   | z
        //         |/      |/    |/
        //         0-------1     +--x
        //
        //         0   =>   0, 0, 0
        //         1   =>   0, 0, 1
        //         2   =>   0, 1, 0
        //         3   =>   0, 1, 1
        //         4   =>   1, 0, 0
        //         5   =>   1, 0, 1
        //         6   =>   1, 1, 0
        //         7   =>   1, 1, 1
        //
        //--------------------------------------------------------

        m_pChildren = m_pTree->requestChildrenFromPool();

        const auto childHalfWidth = m_HalfWidth * static_cast<Real>(0.5);
        for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
        {
            auto newCenter = m_Center;
            newCenter[0] += (childIdx & 1) ? childHalfWidth : -childHalfWidth;
            newCenter[1] += (childIdx & 2) ? childHalfWidth : -childHalfWidth;
            newCenter[2] += (childIdx & 4) ? childHalfWidth : -childHalfWidth;

            OctreeNode* const pChildNode = &m_pChildren->m_Nodes[childIdx];

            // Placement new: re-use existing memory block, just call constructor to re-initialize data
            new(pChildNode) OctreeNode(m_pTree, this, newCenter, childHalfWidth, m_Depth + 1u);
        }

        // Must explicitly mark as non-leaf node, and must do this after all children nodes are ready
        m_bIsLeaf = false;
    }
    m_NodeSplitingLock.unlock();
}

void
OctreeNode::removeAllDescendants()
{
    if (isLeaf())
    {
        return;
    }

    // Must explicitly mark as leaf node
    m_bIsLeaf = true;

    for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
    {
        auto& pChildNode = m_pChildren->m_Nodes[childIdx];
        pChildNode.removeAllDescendants();
    }
    m_pTree->returnChildrenToPool(m_pChildren);
}

void
OctreeNode::removeEmptyDescendants()
{
    if (isLeaf())
    {
        return;
    }

    bool bAllEmpty  = true;
    bool bAllLeaves = true;
    for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
    {
        auto& pChildNode = m_pChildren->m_Nodes[childIdx];
        pChildNode.removeEmptyDescendants();
        bAllLeaves &= pChildNode.isLeaf();

        for (int i = 0; i < OctreePrimitiveType::NumPrimitiveTypes; ++i)
        {
            bAllEmpty &= (pChildNode.m_PrimitiveCounts[i] == 0);
        }
    }

    // Remove all 8 children nodes iff they are all leaf nodes and all empty nodes
    if (bAllEmpty && bAllLeaves)
    {
        m_pTree->returnChildrenToPool(m_pChildren);
        m_bIsLeaf = true;
    }
}

void
OctreeNode::keepPrimitive(OctreePrimitive* const pPrimitive, const OctreePrimitiveType type)
{
    pPrimitive->m_pNode  = this;
    pPrimitive->m_bValid = true;

    m_PrimitiveLock[type].lock();
    pPrimitive->m_pNext = m_pPrimitiveListHeads[type];
    m_pPrimitiveListHeads[type] = pPrimitive;
    m_PrimitiveCounts[type]    += 1u;
    m_PrimitiveLock[type].unlock();
}

void
OctreeNode::insertPoint(OctreePrimitive* const pPrimitive)
{
    // Type alias, to reduce copy/past errors
    static const auto type = OctreePrimitiveType::Point;

    if (m_Depth == m_MaxDepth)
    {
        keepPrimitive(pPrimitive, type);
        return;
    }

    // Split node if this is a leaf node
    split();

    // Compute the index of the child node that contains this point
    uint32_t childIdx = 0;
    for (uint32_t dim = 0; dim < 3; ++dim)
    {
        if (m_Center[dim] < pPrimitive->m_Position[dim])
        {
            childIdx |= (1 << dim);
        }
    }

    m_pChildren->m_Nodes[childIdx].insertPoint(pPrimitive);
}

void
OctreeNode::insertNonPointPrimitive(OctreePrimitive* const pPrimitive, const OctreePrimitiveType type)
{
    const auto  lowerCorner = pPrimitive->m_LowerCorner;
    const auto  upperCorner = pPrimitive->m_UpperCorner;
    const Vec3r priCenter(
        (lowerCorner[0] + upperCorner[0]) * 0.5,
        (lowerCorner[1] + upperCorner[1]) * 0.5,
        (lowerCorner[2] + upperCorner[2]) * 0.5);

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (this != m_pTree->m_pRootNode && !looselyContains(lowerCorner, upperCorner)))
        << "Invalid primitive data (a non-root node must loosely contain primitives)";
#endif

    if (m_Depth == m_MaxDepth)
    {
        keepPrimitive(pPrimitive, type);
        return;
    }

    uint32_t childIdx  = 0;
    bool     bStraddle = false;

    for (uint32_t dim = 0; dim < 3; ++dim)
    {
        if (m_Center[dim] < priCenter[dim])
        {
            if (m_Center[dim] - (m_HalfWidth * 0.5) > lowerCorner[dim]
                || m_Center[dim] + (m_HalfWidth * 1.5) < upperCorner[dim])
            {
                bStraddle = true;
                break;
            }
            else
            {
                childIdx |= (1 << dim);
            }
        }
        else
        {
            if (m_Center[dim] + (m_HalfWidth * 0.5) < upperCorner[dim]
                || m_Center[dim] - (m_HalfWidth * 1.5) > lowerCorner[dim])
            {
                bStraddle = true;
                break;
            }
        }
    }

    // If the primive straddles over multiple children nodes, we must keep it at the current node
    if (bStraddle)
    {
        keepPrimitive(pPrimitive, type);
        return;
    }

    // Split node if this is a leaf node
    split();

    // Insert the primitive to the child node that loosely contains it
    m_pChildren->m_Nodes[childIdx].insertNonPointPrimitive(pPrimitive, type);
}

bool
OctreeNode::updateDebugGeometry()
{
    if (m_Depth > m_pTree->m_MaxLevelDebugRender)
    {
        return false;
    }

    int   renderCount = 0;
    Vec3r vertices[8];
    bool  rendered[8] { false, false, false, false,
                        false, false, false, false };

    // Also call add lines recursively
    for (uint32_t i = 0; i < 8; i++)
    {
        vertices[i]     = m_Center;
        vertices[i][0] += (i & 1) ? m_HalfWidth : -m_HalfWidth;
        vertices[i][1] += (i & 2) ? m_HalfWidth : -m_HalfWidth;
        vertices[i][2] += (i & 4) ? m_HalfWidth : -m_HalfWidth;

        if (!isLeaf())
        {
            rendered[i] = m_pChildren->m_Nodes[i].updateDebugGeometry();
            if (rendered[i])
            {
                ++renderCount;
            }
        }
    }

    //--------------------------------------------------------
    //
    //           6-------7
    //          /|      /|
    //         2-+-----3 |
    //         | |     | |   y
    //         | 4-----+-5   | z
    //         |/      |/    |/
    //         0-------1     +--x
    //
    //         0   =>   0, 0, 0
    //         1   =>   0, 0, 1
    //         2   =>   0, 1, 0
    //         3   =>   0, 1, 1
    //         4   =>   1, 0, 0
    //         5   =>   1, 0, 1
    //         6   =>   1, 1, 0
    //         7   =>   1, 1, 1
    //
    //--------------------------------------------------------

    // No primitive in this node
    if (m_PrimitiveCounts[OctreePrimitiveType::Point] == 0
        && m_PrimitiveCounts[OctreePrimitiveType::Triangle] == 0
        && m_PrimitiveCounts[OctreePrimitiveType::AnalyticalGeometry] == 0)
    {
        if (!m_pTree->m_bDrawNonEmptyParent)
        {
            return (renderCount > 0);
        }

        if (renderCount == 0                  // Children did not render
            && m_pTree->m_pRootNode != this)  // Not root node, and no data in this node)
        {
            return false;
        }
    }

    if (renderCount < 8)  // If renderCount == 8 then no need to render this node
    {
        const auto& debugLines = m_pTree->m_DebugGeometry;
        for (int i = 0; i < 8; ++i)
        {
            if ((i & 1) && (!rendered[i] || !rendered[i - 1]))
            {
                debugLines->appendVertex(vertices[i]);
                debugLines->appendVertex(vertices[i - 1]);
            }
            if ((i & 2) && (!rendered[i] || !rendered[i - 2]))
            {
                debugLines->appendVertex(vertices[i]);
                debugLines->appendVertex(vertices[i - 2]);
            }
            if ((i & 4) && (!rendered[i] || !rendered[i - 4]))
            {
                debugLines->appendVertex(vertices[i]);
                debugLines->appendVertex(vertices[i - 4]);
            }
        }
    }
    return true;
}

LooseOctree::LooseOctree(const Vec3r& center, const Real width, const Real minWidth,
                         const Real minWidthRatio /*= 1.0*/, const std::string name /*= "LooseOctree"*/) :
    m_Name(name),
    m_Center(center),
    m_Width(width),
    m_MinWidthRatio(minWidthRatio),
    m_MinWidth(minWidth),
    m_pRootNode(new OctreeNode(this, nullptr, center, width * static_cast<Real>(0.5), 1u)),
    m_NumAllocatedNodes(1u)
{
}

LooseOctree::~LooseOctree()
{
    // Firstly clear data recursively
    clear();

    // Deallocate memory pool
    deallocateMemoryPool();

    // Remove root node
    delete m_pRootNode;
}

void
LooseOctree::clear()
{
    // Return all tree nodes to memory pool except the root node
    m_pRootNode->removeAllDescendants();

    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
    {
        clearPrimitive(static_cast<OctreePrimitiveType>(type));
    }
    // Remove all geometry pointers
    m_sGeometryIndices.clear();

    // Set state to imcomplete
    m_bCompleteBuild = false;
}

void
LooseOctree::clearPrimitive(const OctreePrimitiveType type)
{
    // Recursively clear primitive data
    m_pRootNode->clearPrimitiveData(type);

    // Remove primitives from tree
    if (m_vPrimitivePtrs[type].size() > 0)
    {
        for (const auto& pPrimitive: m_vPrimitivePtrs[type])
        {
            removeGeometry(pPrimitive->m_GeomIdx);
        }
        m_vPrimitivePtrs[type].resize(0);
    }

    // Deallocate primitive memory blocks
    for (const auto pPrimitiveBlock: m_pPrimitiveBlocks[type])
    {
        delete[] pPrimitiveBlock;
    }
    m_pPrimitiveBlocks[type].resize(0);
}

uint32_t
LooseOctree::getMaxNumPrimitivesInNodes() const
{
    return tbb::parallel_reduce(m_sActiveTreeNodeBlocks.range(),
                    0u,
        [&](decltype(m_sActiveTreeNodeBlocks)::const_range_type& r, uint32_t prevResult) -> uint32_t
        {
            for (auto it = r.begin(), iEnd = r.end(); it != iEnd; ++it)
            {
                const auto pNodeBlock = *it;
                for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
                {
                    const auto& pNode = pNodeBlock->m_Nodes[childIdx];
                    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
                    {
                        if (prevResult < pNode.m_PrimitiveCounts[type])
                        {
                            prevResult = pNode.m_PrimitiveCounts[type];
                        }
                    }
                }
            }
            return prevResult;
        },
        [](const uint32_t x, const uint32_t y) -> uint32_t
        {
            return x > y ? x : y;
        });
}

uint32_t
LooseOctree::addPointSet(const std::shared_ptr<PointSet>& pointset)
{
    // Type alias, to reduce copy/past errors
    static const auto type = static_cast<int>(OctreePrimitiveType::Point);

    const auto pGeometry = static_cast<Geometry*>(pointset.get());
    const auto geomIdx   = pGeometry->getGlobalIndex();
    addGeometry(geomIdx);

    const auto numNewPrimitives = static_cast<uint32_t>(pointset->getNumVertices());
    const auto pPrimitiveBlock  = new OctreePrimitive[numNewPrimitives];
    m_pPrimitiveBlocks[type].push_back(pPrimitiveBlock);

    auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
    vPrimitivePtrs.reserve(vPrimitivePtrs.size() + numNewPrimitives);
    for (uint32_t idx = 0; idx < numNewPrimitives; ++idx)
    {
        const auto pPrimitive = &pPrimitiveBlock[idx];
        new(pPrimitive) OctreePrimitive(pGeometry, geomIdx, idx); // Placement new
        vPrimitivePtrs.push_back(pPrimitive);
    }

    LOG(INFO) << "Added " << numNewPrimitives << " points to " << m_Name;
    return numNewPrimitives;
}

uint32_t
LooseOctree::addTriangleMesh(const std::shared_ptr<SurfaceMesh>& surfMesh)
{
    // Type alias, to reduce copy/past errors
    static const auto type = static_cast<int>(OctreePrimitiveType::Triangle);

    const auto pGeometry = static_cast<Geometry*>(surfMesh.get());
    const auto geomIdx   = pGeometry->getGlobalIndex();
    addGeometry(geomIdx);

    const auto numNewPrimitives = static_cast<uint32_t>(surfMesh->getNumTriangles());
    const auto pPrimitiveBlock  = new OctreePrimitive[numNewPrimitives];
    m_pPrimitiveBlocks[type].push_back(pPrimitiveBlock);

    auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
    vPrimitivePtrs.reserve(vPrimitivePtrs.size() + numNewPrimitives);
    for (uint32_t triIdx = 0; triIdx < numNewPrimitives; ++triIdx)
    {
        const auto pPrimitive = &pPrimitiveBlock[triIdx];
        new(pPrimitive) OctreePrimitive(pGeometry, geomIdx, triIdx); // Placement new
        vPrimitivePtrs.push_back(pPrimitive);
    }

    LOG(INFO) << "Added " << numNewPrimitives << " triangles to " << m_Name;
    return numNewPrimitives;
}

uint32_t
LooseOctree::addAnalyticalGeometry(const std::shared_ptr<Geometry>& geometry)
{
    // Type alias, to reduce copy/past errors
    static const auto type = static_cast<int>(OctreePrimitiveType::AnalyticalGeometry);

    const auto pGeometry = geometry.get();
    const auto geomIdx   = pGeometry->getGlobalIndex();
    addGeometry(geomIdx);

    const auto pPrimitiveBlock = new OctreePrimitive[1];
    m_pPrimitiveBlocks[type].push_back(pPrimitiveBlock);

    const auto pPrimitive = &pPrimitiveBlock[0];
    new(pPrimitive) OctreePrimitive(pGeometry, geomIdx, 0); // Placement new
    m_vPrimitivePtrs[type].push_back(pPrimitive);

    LOG(INFO) << "Added a new analytical geometry to " << m_Name;
    return 1u;
}

void
LooseOctree::addGeometry(const uint32_t geomIdx)
{
    LOG_IF(FATAL, (hasGeometry(geomIdx))) << "Geometry has previously been added";
    m_sGeometryIndices.insert(geomIdx);
}

void
LooseOctree::removeGeometry(const uint32_t geomIdx)
{
    const auto it = m_sGeometryIndices.find(geomIdx);
    if (it != m_sGeometryIndices.end())
    {
        m_sGeometryIndices.erase(it);
    }
}

void
LooseOctree::build()
{
    if (m_sGeometryIndices.size() == 0)
    {
        LOG(WARNING) << "There was not any geometry added in the tree named '" << m_Name << "'";
        return;
    }

    // Compute the minimum bounding box of non-point primitives
    if (m_vPrimitivePtrs[OctreePrimitiveType::Point].size() == 0
        && (m_vPrimitivePtrs[OctreePrimitiveType::Triangle].size() > 0
            || m_vPrimitivePtrs[OctreePrimitiveType::AnalyticalGeometry].size() > 0))
    {
        Real minWidth = MAX_REAL;
        for (int type = OctreePrimitiveType::Triangle; type <= OctreePrimitiveType::AnalyticalGeometry; ++type)
        {
            const auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
            if (vPrimitivePtrs.size() == 0)
            {
                continue;
            }
            const auto primitiveMinWidth = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, vPrimitivePtrs.size()),
                                                                MAX_REAL,
                [&](const tbb::blocked_range<size_t>& r, Real prevResult) -> Real {
                    for (auto i = r.begin(), iEnd = r.end(); i != iEnd; ++i)
                    {
                        const auto pPrimitive = vPrimitivePtrs[i];
                        computePrimitiveBoundingBox(pPrimitive, static_cast<OctreePrimitiveType>(type));

                        Vec3r widths;
                        for (uint32_t dim = 0; dim < 3; ++dim)
                        {
                            widths[dim] = pPrimitive->m_UpperCorner[dim] - pPrimitive->m_LowerCorner[dim];
                        }
                        auto maxBoxWidth = widths[0];
                        maxBoxWidth      = maxBoxWidth < widths[1] ? widths[1] : maxBoxWidth;
                        maxBoxWidth      = maxBoxWidth < widths[2] ? widths[2] : maxBoxWidth;
                        prevResult       = prevResult > maxBoxWidth ? maxBoxWidth : prevResult;
                    }
                    return prevResult;
                },
                [](const Real x, const Real y) -> Real {
                    return x < y ? x : y;
                });

            minWidth = minWidth < primitiveMinWidth ? minWidth : primitiveMinWidth;
        }

        if (minWidth < 1e-8)
        {
            LOG(WARNING) << "Object/triangles have too small size";
        }
        else
        {
            m_MinWidth = m_MinWidthRatio * minWidth;
        }
    }

    // Compute max depth that the tree can reach
    m_MaxDepth = 1u;
    uint32_t numLevelNodes   = 1u;
    uint32_t maxNumTreeNodes = 1u;
    Real     nodeWidth       = m_Width;

    while (nodeWidth * static_cast<Real>(0.5) > m_MinWidth) {
        ++m_MaxDepth;
        numLevelNodes   *= 8u;
        maxNumTreeNodes += numLevelNodes;
        nodeWidth       *= static_cast<Real>(0.5);
    }
    m_pRootNode->m_MaxDepth = m_MaxDepth;
    rebuild();
    m_bCompleteBuild = true;

    LOG(INFO) << m_Name << " generated, center = [" << m_Center[0] << ", " << m_Center[1] << ", " << m_Center[2]
              << "], width = " << m_Width << ", min width = " << m_MinWidth
              << ", max depth = " << m_MaxDepth << ", max num. nodes = " << maxNumTreeNodes;
}

void
LooseOctree::update()
{
    if (!m_bCompleteBuild)
    {
        build();
    }
    (!m_bAlwaysRebuild) ? incrementalUpdate() : rebuild();
}

void
LooseOctree::rebuild()
{
    // Recursively remove all tree nodes other than root node
    m_pRootNode->removeAllDescendants();

    // Clear root node data
    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
    {
        m_pRootNode->clearPrimitiveData(static_cast<OctreePrimitiveType>(type));
    }

    // Populate all primitives to tree nodes in a top-down manner
    populatePointPrimitives();
    populateNonPointPrimitives(OctreePrimitiveType::Triangle);
    populateNonPointPrimitives(OctreePrimitiveType::AnalyticalGeometry);
}

void
LooseOctree::populatePointPrimitives()
{
    const auto& vPrimitivePtrs = m_vPrimitivePtrs[OctreePrimitiveType::Point];
    if (vPrimitivePtrs.size() == 0)
    {
        return;
    }
    ParallelUtils::parallelFor(vPrimitivePtrs.size(),
        [&](const size_t idx) {
            const auto pPrimitive  = vPrimitivePtrs[idx];
            const auto pointset    = static_cast<PointSet*>(pPrimitive->m_pGeometry);
            const auto point       = pointset->getVertexPosition(pPrimitive->m_Idx);
            pPrimitive->m_Position = { point[0], point[1], point[2] };
            m_pRootNode->insertPoint(pPrimitive);
        });
}

void
LooseOctree::populateNonPointPrimitives(const OctreePrimitiveType type)
{
    const auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
    if (vPrimitivePtrs.size() == 0)
    {
        return;
    }
    ParallelUtils::parallelFor(vPrimitivePtrs.size(),
        [&](const size_t idx) {
            const auto pPrimitive = vPrimitivePtrs[idx];
            computePrimitiveBoundingBox(pPrimitive, type);
            m_pRootNode->insertNonPointPrimitive(pPrimitive, type);
        });
}

void
LooseOctree::incrementalUpdate()
{
    // For all primitives, update their positions (if point) or bounding box (if non-point)
    // Then, check their validity (valid primitive = it is still loosely contained in the node's bounding box)
    updatePositionAndCheckValidity();
    updateBoundingBoxAndCheckValidity(OctreePrimitiveType::Triangle);
    updateBoundingBoxAndCheckValidity(OctreePrimitiveType::AnalyticalGeometry);

    // Remove all invalid primitives from tree nodes
    removeInvalidPrimitivesFromNodes();

    // Insert the invalid primitives back to the tree
    reinsertInvalidPrimitives(OctreePrimitiveType::Point);
    reinsertInvalidPrimitives(OctreePrimitiveType::Triangle);
    reinsertInvalidPrimitives(OctreePrimitiveType::AnalyticalGeometry);

    // Recursively remove all empty nodes, returning them to memory pool for recycling
    m_pRootNode->removeEmptyDescendants();
}

void
LooseOctree::updatePositionAndCheckValidity()
{
    const auto& vPrimitivePtrs = m_vPrimitivePtrs[OctreePrimitiveType::Point];
    if (vPrimitivePtrs.size() == 0)
    {
        return;
    }
    ParallelUtils::parallelFor(vPrimitivePtrs.size(),
        [&](const size_t idx) {
            const auto pPrimitive = vPrimitivePtrs[idx];
            const auto pointset   = static_cast<PointSet*>(pPrimitive->m_pGeometry);
            const auto point      = pointset->getVertexPosition(pPrimitive->m_Idx);

            // Cache the position
            pPrimitive->m_Position = { point[0], point[1], point[2] };

            auto pNode = pPrimitive->m_pNode;
            if (!pNode->looselyContains(point) && pNode != m_pRootNode)
            {
                // Go up, find the node tightly containing it (or stop if reached root node)
                while (pNode != m_pRootNode) {
                    pNode = pNode->m_pParent; // Go up one level
                    if (pNode->contains(point) || pNode == m_pRootNode)
                    {
                        pPrimitive->m_bValid = false;
                        pPrimitive->m_pNode  = pNode;
                        break;
                    }
                }
            }
            else
            {
                pPrimitive->m_bValid = (pNode != m_pRootNode) ? true : false;
            }
        });
}

void
LooseOctree::updateBoundingBoxAndCheckValidity(const OctreePrimitiveType type)
{
    const auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
    if (vPrimitivePtrs.size() == 0)
    {
        return;
    }
    ParallelUtils::parallelFor(vPrimitivePtrs.size(),
        [&](const size_t idx) {
            const auto pPrimitive = vPrimitivePtrs[idx];
            computePrimitiveBoundingBox(pPrimitive, type);
            const auto lowerCorner = pPrimitive->m_LowerCorner;
            const auto upperCorner = pPrimitive->m_UpperCorner;
            const Vec3r center(
                (lowerCorner[0] + upperCorner[0]) * 0.5,
                (lowerCorner[1] + upperCorner[1]) * 0.5,
                (lowerCorner[2] + upperCorner[2]) * 0.5);

            auto pNode = pPrimitive->m_pNode;
            if (!pNode->looselyContains(lowerCorner, upperCorner) && pNode != m_pRootNode)
            {
                // Go up, find the node tightly containing it (or stop if reached root node)
                while (pNode != m_pRootNode) {
                    pNode = pNode->m_pParent; // Go up one level

                    if (pNode->contains(lowerCorner, upperCorner) || pNode == m_pRootNode)
                    {
                        pPrimitive->m_bValid = false;
                        pPrimitive->m_pNode  = pNode;
                        break;
                    }
                }
            }
            // If node still contains primitive + node depth reaches maxDepth
            else if (pNode->m_Depth == m_MaxDepth)
            {
                pPrimitive->m_bValid = true;
            }
            // If node still contains primitive but node depth does not reach maxDepth,
            // then check if the primitive straddles over children nodes
            else
            {
                bool bStraddle = false;

                for (uint32_t dim = 0; dim < 3; ++dim)
                {
                    if (m_Center[dim] < center[dim])
                    {
                        if (pNode->m_Center[dim] - (pNode->m_HalfWidth * 0.5) > lowerCorner[dim]
                            || pNode->m_Center[dim] + (pNode->m_HalfWidth * 1.5) < upperCorner[dim])
                        {
                            bStraddle = true;
                            break;
                        }
                    }
                    else
                    {
                        if (pNode->m_Center[dim] + (pNode->m_HalfWidth * 0.5) < upperCorner[dim]
                            || pNode->m_Center[dim] - (pNode->m_HalfWidth * 1.5) > lowerCorner[dim])
                        {
                            bStraddle = true;
                            break;
                        }
                    }
                }

                // If the primitive straddles over children nodes, it cannot be moved down to any child node
                if (bStraddle)
                {
                    pPrimitive->m_bValid = true;
                }
                else
                {
                    // Move the primitive down to a child node
                    pPrimitive->m_bValid = false;
                    pPrimitive->m_pNode  = pNode;
                }
            }
        });
}

void
LooseOctree::removeInvalidPrimitivesFromNodes()
{
    if (m_sActiveTreeNodeBlocks.size() == 0)
    {
        return;
    }
    tbb::parallel_for(m_sActiveTreeNodeBlocks.range(),
        [&](decltype(m_sActiveTreeNodeBlocks)::const_range_type& r) {
            for (auto it = r.begin(), iEnd = r.end(); it != iEnd; ++it)
            {
                const auto pNodeBlock = *it;
                for (uint32_t childIdx = 0; childIdx < 8u; ++childIdx)
                {
                    auto& pNode = pNodeBlock->m_Nodes[childIdx];
                    for (int type = 0; type < OctreePrimitiveType::NumPrimitiveTypes; ++type)
                    {
                        const auto pOldHead = pNode.m_pPrimitiveListHeads[type];
                        if (!pOldHead)
                        {
                            continue;
                        }

                        OctreePrimitive* pIter    = pOldHead;
                        OctreePrimitive* pNewHead = nullptr;
                        uint32_t count = 0;
                        while (pIter) {
                            const auto pNext = pIter->m_pNext;
                            if (pIter->m_bValid)
                            {
                                pIter->m_pNext = pNewHead;
                                pNewHead       = pIter;
                                ++count;
                            }
                            pIter = pNext;
                        }
                        pNode.m_pPrimitiveListHeads[type] = pNewHead;
                        pNode.m_PrimitiveCounts[type]     = count;
                    }
                }
            }
        });
}

void
LooseOctree::reinsertInvalidPrimitives(const OctreePrimitiveType type)
{
    const auto& vPrimitivePtrs = m_vPrimitivePtrs[type];
    if (vPrimitivePtrs.size() == 0)
    {
        return;
    }
    ParallelUtils::parallelFor(vPrimitivePtrs.size(),
        [&](const size_t idx) {
            const auto pPrimitive = vPrimitivePtrs[idx];
            if (pPrimitive->m_bValid)
            {
                return;
            }

            const auto pNode = pPrimitive->m_pNode;
            (type == OctreePrimitiveType::Point) ? pNode->insertPoint(pPrimitive) : pNode->insertNonPointPrimitive(pPrimitive, type);
        });
}

void
LooseOctree::computePrimitiveBoundingBox(OctreePrimitive* const pPrimitive, const OctreePrimitiveType type)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (type == OctreePrimitiveType::Point))
        << "Cannot compute bounding box for point primitive";
#endif

    Vec3r lowerCorner;
    Vec3r upperCorner;

    if (type == OctreePrimitiveType::Triangle)
    {
        const auto surfMesh = static_cast<SurfaceMesh*>(pPrimitive->m_pGeometry);
        const auto face     = surfMesh->getTrianglesVertices()[pPrimitive->m_Idx];

        lowerCorner = surfMesh->getVertexPosition(face[0]);
        upperCorner = lowerCorner;

        const Vec3r verts12[2] = {
            surfMesh->getVertexPosition(face[1]),
            surfMesh->getVertexPosition(face[2])
        };

        for (uint32_t dim = 0; dim < 3; ++dim)
        {
            lowerCorner[dim] = lowerCorner[dim] < verts12[0][dim] ? lowerCorner[dim] : verts12[0][dim];
            lowerCorner[dim] = lowerCorner[dim] < verts12[1][dim] ? lowerCorner[dim] : verts12[1][dim];

            upperCorner[dim] = upperCorner[dim] > verts12[0][dim] ? upperCorner[dim] : verts12[0][dim];
            upperCorner[dim] = upperCorner[dim] > verts12[1][dim] ? upperCorner[dim] : verts12[1][dim];
        }
    }
    else
    {
        pPrimitive->m_pGeometry->computeBoundingBox(lowerCorner, upperCorner);
    }

    pPrimitive->m_LowerCorner = { lowerCorner[0], lowerCorner[1], lowerCorner[2] };
    pPrimitive->m_UpperCorner = { upperCorner[0], upperCorner[1], upperCorner[2] };
}

OctreeNodeBlock*
LooseOctree::requestChildrenFromPool()
{
    m_PoolLock.lock();
    if (m_NumAvaiableBlocksInPool == 0)
    {
        // Allocate 64 more node blocks and put to the pool
        allocateMoreNodeBlock(64u);
    }

    const auto pNodeBlock = m_pNodeBlockPoolHead;
    m_pNodeBlockPoolHead       = pNodeBlock->m_NextBlock;
    m_NumAvaiableBlocksInPool -= 1u;
    m_PoolLock.unlock();
    m_sActiveTreeNodeBlocks.insert(pNodeBlock);
    return pNodeBlock;
}

void
LooseOctree::returnChildrenToPool(OctreeNodeBlock* const pNodeBlock)
{
    m_PoolLock.lock();
    pNodeBlock->m_NextBlock    = m_pNodeBlockPoolHead;
    m_pNodeBlockPoolHead       = pNodeBlock;
    m_NumAvaiableBlocksInPool += 1u;
    m_sActiveTreeNodeBlocks.unsafe_erase(pNodeBlock);
    m_PoolLock.unlock();
}

void
LooseOctree::allocateMoreNodeBlock(const uint32_t numBlocks)
{
    // This is not a thead-safe function, thus it should be called only from a thread-safe function
    // And it must be called only from the requestChildrenFromPool() function
    const auto pBigBlock = new OctreeNodeBlock[numBlocks];
    m_pNodeBigBlocks.push_back(pBigBlock);
    for (uint32_t i = 0; i < numBlocks; ++i)
    {
        const auto pBlock = &pBigBlock[i];
        pBlock->m_NextBlock  = m_pNodeBlockPoolHead;
        m_pNodeBlockPoolHead = pBlock;
    }
    m_NumAvaiableBlocksInPool += numBlocks;
    m_NumAllocatedNodes       += numBlocks * 8u;
}

void
LooseOctree::deallocateMemoryPool()
{
    LOG_IF(FATAL, (m_NumAllocatedNodes != m_NumAvaiableBlocksInPool * 8u + 1u))
        << "Internal data corrupted, may be all nodes were not returned from tree";

    for (const auto pBigBlock: m_pNodeBigBlocks)
    {
        delete[] pBigBlock;
    }
    m_pNodeBigBlocks.resize(0);
    m_NumAllocatedNodes       = 1u; // root node still remains
    m_NumAvaiableBlocksInPool = 0u;
}

std::shared_ptr<DebugRenderGeometry>
LooseOctree::getDebugGeometry(const uint32_t maxLevel, bool bDrawNonEmptyParent /*= true*/)
{
    m_MaxLevelDebugRender = maxLevel;
    m_bDrawNonEmptyParent = bDrawNonEmptyParent;
    if (!m_DebugGeometry)
    {
        m_DebugGeometry.reset();
    }

    // Create debug geometry and set default rendering mateirial
    m_DebugGeometry = std::make_shared<DebugRenderLines>("OctreeDebugRendering");
    const auto material = std::make_shared<RenderMaterial>();
    material->setDebugColor(Color::Green);
    material->setLineWidth(1.0);
    m_DebugGeometry->setRenderMaterial(material);

    // Update debug rendering data (if any)
    m_pRootNode->updateDebugGeometry();
    m_DebugGeometry->setDataModified(true);

    return std::static_pointer_cast<DebugRenderGeometry>(m_DebugGeometry);
}

void
LooseOctree::updateDebugGeometry()
{
    LOG_IF(FATAL, (!m_DebugGeometry)) << "Debug geometry has not been created";
    m_DebugGeometry->clear();
    m_pRootNode->updateDebugGeometry();
    m_DebugGeometry->setDataModified(true);
}
} // end namespace imstk
