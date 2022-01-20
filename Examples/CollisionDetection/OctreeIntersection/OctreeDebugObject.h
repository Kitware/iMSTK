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

#include "imstkDebugGeometryObject.h"

namespace imstk
{
class LooseOctree;
class OctreeNode;

///
/// \class OctreeDebugObject
///
/// \brief DebugGeometryObject for debug visualizing an octree
///
class OctreeDebugObject : public DebugGeometryObject
{
public:
    OctreeDebugObject() : DebugGeometryObject(),
        m_looseOctree(nullptr),
        m_maxDisplayDepth(5),
        m_drawNonEmptyParents(true) { }
    virtual ~OctreeDebugObject() override = default;

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

    int  m_maxDisplayDepth; ///> Maximum level of nodes that will be rendered during debug rendering
    bool m_drawNonEmptyParents;
};
} // namespace imstk