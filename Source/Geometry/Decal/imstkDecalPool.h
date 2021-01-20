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
#include "imstkGeometry.h"
#include <array>
#include <queue>

#pragma warning( push )
#pragma warning( disable : 4201 )
#include "glm/glm.hpp"
#pragma warning( pop )

namespace imstk
{
class Decal;

class DecalPool : public Geometry
{
public:
    DecalPool(unsigned int maxNumDecals = 128, const std::string& name = std::string(""));

    ///
    /// \brief Add a \ref Decal object to this pool
    ///
    std::shared_ptr<Decal> addDecal();

    ///
    /// \brief Remove a \ref Decal object to this pool
    ///
    void removeDecal();

    ///
    /// \brief Get all decals
    ///
    std::deque<std::shared_ptr<Decal>>& getDecals();

    ///
    /// \brief Set/Get recyle
    ///
    void setRecycle(bool recycle);
    bool getRecycle();

    ///
    /// \brief Get the number of decals
    ///
    unsigned int getNumDecals();

    ///
    /// \brief Get the max number of decals
    ///
    unsigned int getMaxNumDecals();

protected:
    friend class VulkanDecalRenderDelegate;

    std::deque<std::shared_ptr<Decal>> m_orderedDecals;
    std::deque<std::shared_ptr<Decal>> m_freeDecals;

    void applyTransform(const Mat4d& imstkNotUsed(m)) override { }
    virtual void updatePostTransformData() const override { }

    unsigned int m_maxNumDecals;
    unsigned int m_numDecals = 0;
    bool m_recycle = true;

    glm::vec3  m_vertexPositions[8];
    glm::ivec3 m_triangles[12];
};
}
