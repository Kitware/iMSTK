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

#include "vulkan/vulkan.h"

#include "imstkVulkanBuffer.h"
#include "imstkVulkanMemoryManager.h"

#include <array>
#include <vector>

namespace imstk
{

/// \brief TODO
enum VulkanVertexBufferMode
{
    VERTEX_BUFFER_STATIC,
    VERTEX_BUFFER_DYNAMIC
};

/// \brief TODO
struct VulkanBasicVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
    glm::vec3 color;
};

/// \brief TODO
class VulkanVertexBuffer : public VulkanBuffer
{
public:
    /// \brief TODO
    VulkanVertexBuffer(VulkanMemoryManager&   memoryManager,
                       unsigned int           numVertices,
                       unsigned int           vertexSize,
                       unsigned int           numTriangles,
                       double                 loadFactor = 1.0,
                       VulkanVertexBufferMode mode = VERTEX_BUFFER_STATIC);

    /// \brief TODO
    void* getVertexMemory(uint32_t frameIndex = 0);

    /// \brief TODO
    void* getIndexMemory(uint32_t frameIndex = 0);

    ~VulkanVertexBuffer() = default;

    ///
    /// \brief Utility function to update buffers
    ///
    void updateVertexBuffer(std::vector<VulkanBasicVertex>* vertices,
                            std::vector<std::array<uint32_t, 3>>* triangles);

    /// \brief TODO
    void uploadBuffers(VkCommandBuffer& commandBuffer);

    /// \brief TODO
    void initializeBuffers(VulkanMemoryManager& memoryManager);

    /// \brief TODO
    void setNumIndices(uint32_t numIndices);

    /// \brief TODO
    void bindBuffers(VkCommandBuffer* commandBuffer, uint32_t frameIndex);

    ///
    /// \brief Sets buffer states to modified
    /// Triggers updates to vertex and index buffers
    ///
    void setModified();

    /// \brief TODO
    VulkanVertexBufferMode getMode();

private:
    friend class VulkanRenderer;

    VulkanInternalBuffer* m_vertexBuffer;
    VulkanInternalBuffer* m_vertexStagingBuffer;

    uint32_t m_numIndices;

    VulkanInternalBuffer* m_indexBuffer;
    VulkanInternalBuffer* m_indexStagingBuffer;

    VkDevice m_renderDevice;
    uint32_t m_bufferMemoryIndex;

    uint32_t m_vertexBufferSize     = 0;
    uint32_t m_indexBufferSize      = 0;
    bool     m_vertexBufferModified = true;
    bool     m_indexBufferModified  = true;
    VulkanVertexBufferMode m_mode   = VulkanVertexBufferMode::VERTEX_BUFFER_STATIC;

    static const uint32_t maxBufferSize = 1024 * 1024;
};
}
