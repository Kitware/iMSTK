/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDebugGeometryModel.h"

namespace imstk
{
class LooseOctree;
class OctreeNode;

///
/// \class OctreeDebugModel
///
/// \brief OctreeDebugModel for debug visualizing an octree
///
class OctreeDebugModel : public DebugGeometryModel
{
public:
    OctreeDebugModel() : DebugGeometryModel(),
        m_looseOctree(nullptr),
        m_maxDisplayDepth(5),
        m_drawNonEmptyParents(true) { }
    ~OctreeDebugModel() override = default;

public:
    ///
    /// \brief Update the geometry
    /// \param Max depth to display
    ///
    void debugUpdate(int maxDisplayDepth, bool drawNonEmptyParents = true);

    void setInputOctree(std::shared_ptr<LooseOctree> octree) { m_looseOctree = octree; }

protected:
    ///
    /// \brief Recursively update debug geometry by adding lines drawing bounding boxes of the active nodes
    /// \return True if debug lines have been added to visualize the bounding box of the current node
    ///
    bool updateGeom(OctreeNode* node);

protected:
    std::shared_ptr<LooseOctree> m_looseOctree;

    int  m_maxDisplayDepth; ///< Maximum level of nodes that will be rendered during debug rendering
    bool m_drawNonEmptyParents;
};
} // namespace imstk