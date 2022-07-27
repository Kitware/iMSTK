/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "OctreeDebugObject.h"
#include "imstkLooseOctree.h"

namespace imstk
{
bool
OctreeDebugObject::updateGeom(OctreeNode* node)
{
    if (node->m_Depth > static_cast<uint32_t>(m_maxDisplayDepth))
    {
        return false;
    }

    // Compute the current nodes vertices
    // Which nodes are rendered
    // The amount of child nodes rendered
    int   renderCount = 0;
    Vec3d vertices[8];
    bool  rendered[8]{ false, false, false, false,
                       false, false, false, false };

    // Recurse down the tree so long as the current node isn't a leaf
    for (int i = 0; i < 8; i++)
    {
        vertices[i]     = node->m_Center;
        vertices[i][0] += (i & 1) ? node->m_HalfWidth : -node->m_HalfWidth;
        vertices[i][1] += (i & 2) ? node->m_HalfWidth : -node->m_HalfWidth;
        vertices[i][2] += (i & 4) ? node->m_HalfWidth : -node->m_HalfWidth;

        if (!node->isLeaf())
        {
            rendered[i]  = updateGeom(&node->m_pChildren->m_Nodes[i]);
            renderCount += static_cast<int>(rendered[i]);
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

    // If the current node is empty
    if (node->m_PrimitiveCounts[OctreePrimitiveType::Point] == 0
        && node->m_PrimitiveCounts[OctreePrimitiveType::Triangle] == 0
        && node->m_PrimitiveCounts[OctreePrimitiveType::Analytical] == 0)
    {
        // If we shouldn't draw empty parents
        if (!m_drawNonEmptyParents)
        {
            // So long as no child is rendered
            return renderCount > 0;
        }

        if (renderCount == 0                         // Children did not render
            && node->m_pTree->getRootNode() != node) // Not root node, and no data in this node)
        {
            return false;
        }
    }

    if (renderCount < 8)  // If renderCount == 8 then no need to render this node
    {
        //const auto& debugLines = node->m_pTree->m_DebugGeometry;
        for (int i = 0; i < 8; i++)
        {
            if ((i & 1) && (!rendered[i] || !rendered[i - 1]))
            {
                addLine(vertices[i], vertices[i - 1]);
            }
            if ((i & 2) && (!rendered[i] || !rendered[i - 2]))
            {
                addLine(vertices[i], vertices[i - 2]);
            }
            if ((i & 4) && (!rendered[i] || !rendered[i - 4]))
            {
                addLine(vertices[i], vertices[i - 4]);
            }
        }
    }
    return true;
}

void
OctreeDebugObject::debugUpdate(int maxDisplayDepth, bool drawNonEmptyParents)
{
    m_maxDisplayDepth     = maxDisplayDepth;
    m_drawNonEmptyParents = drawNonEmptyParents;

    // Clear the geometry
    clear();

    if (m_looseOctree != nullptr)
    {
        // Buffer the vertices and indices for lines to avoid reallocations

        // Update the geometry, recursively appending for each node
        updateGeom(m_looseOctree->getRootNode());
    }
}
} // namespace imstk