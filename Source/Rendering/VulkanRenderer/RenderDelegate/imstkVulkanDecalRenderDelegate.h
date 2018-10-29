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

#ifndef imstkVulkanDecalRenderDelegate_h
#define imstkVulkanDecalRenderDelegate_h

#include "imstkDecalPool.h"
#include "imstkCamera.h"
#include "imstkVulkanRenderDelegate.h"

namespace imstk
{
class VulkanDecalRenderDelegate : public VulkanRenderDelegate {
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanDecalRenderDelegate() = default;

    ///
    /// \brief Default constructor
    ///
    VulkanDecalRenderDelegate(std::shared_ptr<DecalPool> decalPool,
                              SceneObject::Type type,
                              VulkanMemoryManager& memoryManager);

    ///
    /// \brief Update render geometry
    ///
    void update(uint32_t frameIndex, std::shared_ptr<Camera> camera);

    ///
    /// \brief Get source geometry
    ///
    std::shared_ptr<Geometry> getGeometry() const override;

    ///
    /// \brief Fill vertex buffer
    ///
    void updateVertexBuffer();

    ///
    /// \brief Initialize data
    ///
    void initializeData(VulkanMemoryManager& memoryManager,
                        std::shared_ptr<RenderMaterial> material = nullptr);

protected:
    std::shared_ptr<DecalPool> m_geometry;

    VulkanLocalDecalVertexUniforms m_decalVertexUniforms;
    VulkanLocalDecalFragmentUniforms m_decalFragmentUniforms;
};
}

#endif
