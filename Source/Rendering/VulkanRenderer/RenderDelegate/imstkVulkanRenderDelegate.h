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

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>

#include "imstkSceneObject.h"
#include "imstkVulkanVertexBuffer.h"
#include "imstkVulkanUniformBuffer.h"
#include "imstkVulkanMaterialDelegate.h"

#include "vtkPolyDataMapper.h"

namespace imstk
{
class VulkanRenderDelegate
{
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanRenderDelegate() = default;

    ///
    /// \brief Initialize class
    ///
    void initialize(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Creates a render delegate from visual model
    ///
    static std::shared_ptr<VulkanRenderDelegate> make_delegate(
        std::shared_ptr<VisualModel> visualModel,
        SceneObject::Type            type,
        VulkanMemoryManager&         details);

    ///
    /// \brief Get source geometry
    ///
    std::shared_ptr<VisualModel> getVisualModel() const;

    ///
    /// \brief Update render geometry. This is implemented a little differently
    ///        in that memory is directly mapped from the VisualModel to the
    ///        RenderDelegate.
    ///
    virtual void update(const uint32_t frameIndex) {}

    ///
    /// \brief Get vertex buffer
    ///
    std::shared_ptr<VulkanVertexBuffer> getBuffer();

    ///
    /// \brief Initialize data
    ///
    void initializeData(
        VulkanMemoryManager&            memoryManager,
        std::shared_ptr<RenderMaterial> material,
        VulkanVertexBufferMode          mode = VulkanVertexBufferMode::VERTEX_BUFFER_STATIC);

    ///
    /// \brief Initialize data
    ///
    void updateTransform();

    void updateUniforms(uint32_t frameIndex);

protected:
    friend class VulkanVertexBuffer;
    friend class VulkanRenderer;
    friend class VulkanUniformBuffer;

    unsigned int m_numTriangles;
    unsigned int m_numVertices;
    unsigned int m_vertexSize;
    double       m_loadFactor = 1.0;
    std::shared_ptr<VisualModel> m_visualModel;

    ///
    /// \brief Default constructor (protected)
    ///
    VulkanRenderDelegate() {}

    std::shared_ptr<VulkanVertexBuffer> m_vertexBuffer;

    std::shared_ptr<VulkanUniformBuffer> m_vertexUniformBuffer;
    std::shared_ptr<VulkanUniformBuffer> m_fragmentUniformBuffer;

    std::shared_ptr<VulkanMaterialDelegate> m_material          = nullptr;
    std::shared_ptr<VulkanMaterialDelegate> m_wireframeMaterial = nullptr;
    std::shared_ptr<VulkanMaterialDelegate> m_shadowMaterial    = nullptr;
    std::shared_ptr<VulkanMaterialDelegate> m_depthMaterial     = nullptr;

    VulkanLocalVertexUniforms   m_localVertexUniforms;
    VulkanLocalFragmentUniforms m_localFragmentUniforms;
};
}
