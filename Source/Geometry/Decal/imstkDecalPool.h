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

#include <queue>
#include <array>

#include "imstkDecal.h"
#include "imstkGeometry.h"
#include "imstkRenderMaterial.h"

namespace imstk
{
class DecalPool : public Geometry
{
public:
    DecalPool(unsigned int maxNumDecals = 128, const std::string name = std::string(""));

    std::shared_ptr<Decal> addDecal();

    void removeDecal();

    std::deque<std::shared_ptr<Decal>>& getDecals();

    void setRecycle(bool recycle);
    bool getRecycle();

    unsigned int getNumDecals();
    unsigned int getMaxNumDecals();

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() const override { return 0; }

protected:
    friend class VulkanDecalRenderDelegate;

    std::deque<std::shared_ptr<Decal>> m_orderedDecals;
    std::deque<std::shared_ptr<Decal>> m_freeDecals;

    void applyTranslation(const Vec3d t) override {}
    void applyRotation(const Mat3d r) override {}
    void applyScaling(const double s) override {}
    virtual void updatePostTransformData() override {}

    unsigned int m_maxNumDecals;
    unsigned int m_numDecals = 0;
    bool         m_recycle   = true;

    glm::vec3  m_vertexPositions[8];
    glm::ivec3 m_triangles[12];
};
}
