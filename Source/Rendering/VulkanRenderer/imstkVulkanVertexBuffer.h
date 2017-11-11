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

#ifndef imstkVulkanVertexBuffer_h
#define imstkVulkanVertexBuffer_h

#include "vulkan/vulkan.h"

#include "imstkVulkanBuffer.h"
#include "imstkVulkanMemoryManager.h"

#include <array>
#include <vector>

namespace imstk
{
enum VulkanVertexBufferMode
{
    VERTEX_BUFFER_STATIC = 0,
    VERTEX_BUFFER_DYNAMIC,
    VERTEX_BUFFER_DYNAMIC_RESIZEABLE
};

struct VulkanBasicVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
};

class VulkanVertexBuffer : public VulkanBuffer
{
public:
    VulkanVertexBuffer(VulkanMemoryManager& memoryManager,
                       unsigned int numVertices,
                       unsigned int vertexSize,
                       unsigned int numTriangles,
                       double loadFactor = 1.0,
                       VulkanVertexBufferMode mode = VERTEX_BUFFER_STATIC);

    void * mapVertices();
    void unmapVertices();

    void * mapTriangles();
    void unmapTriangles();

    ~VulkanVertexBuffer() = default;

    ///
    /// \brief Utility function to update buffers
    ///
    void updateVertexBuffer(std::vector<VulkanBasicVertex> * vertices,
                            std::vector<std::array<uint32_t, 3>> * triangles);

    ///
    /// \brief Binds the vertex buffer to memory
    ///
    void bind();

    void uploadBuffers(VkCommandBuffer& commandBuffer);

    void initializeBuffers(VulkanMemoryManager& memoryManager);

    void setNumIndices(uint32_t numIndices);

private:
    friend class VulkanRenderer;

    VkBuffer m_vertexBuffer;
    VkBuffer m_vertexStagingBuffer;
    VkDeviceMemory m_vertexMemory;
    VkDeviceMemory m_vertexStagingMemory;

    uint32_t m_numIndices;

    VkBuffer m_indexBuffer;
    VkBuffer m_indexStagingBuffer;
    VkDeviceMemory m_indexMemory;
    VkDeviceMemory m_indexStagingMemory;

    VkDevice m_renderDevice;
    uint32_t m_bufferMemoryIndex;

    VulkanVertexBufferMode m_mode;

    uint32_t m_vertexBufferSize = 0;
    uint32_t m_indexBufferSize = 0;
    bool m_vertexBufferModified = true;
    bool m_indexBufferModified = true;

    static const uint32_t maxBufferSize = 1024 * 1024;
};
}

#endif