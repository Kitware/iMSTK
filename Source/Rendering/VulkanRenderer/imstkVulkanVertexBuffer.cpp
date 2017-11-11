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

#include "imstkVulkanVertexBuffer.h"

namespace imstk
{
VulkanVertexBuffer::VulkanVertexBuffer(VulkanMemoryManager& memoryManager,
                                       unsigned int numVertices,
                                       unsigned int vertexSize,
                                       unsigned int numTriangles,
                                       double loadFactor,
                                       VulkanVertexBufferMode mode)
{
    m_mode = mode;
    m_renderDevice = memoryManager.m_device;
    m_vertexBufferSize = (uint32_t)(numVertices * vertexSize * loadFactor);
    m_numIndices = numTriangles;
    m_indexBufferSize = (uint32_t)(m_numIndices * 3 * sizeof(uint32_t) * loadFactor);

    // Vertex buffer
    {
        VkBufferCreateInfo vertexBufferInfo;
        vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.pNext = nullptr;
        vertexBufferInfo.flags = 0;
        vertexBufferInfo.size = m_vertexBufferSize;
        vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vertexBufferInfo.queueFamilyIndexCount = 0;
        vertexBufferInfo.pQueueFamilyIndices = nullptr;

        auto vertexStagingBufferInfo = vertexBufferInfo;
        vertexStagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        vkCreateBuffer(m_renderDevice, &vertexBufferInfo, nullptr, &m_vertexBuffer);
        vkCreateBuffer(m_renderDevice, &vertexStagingBufferInfo, nullptr, &m_vertexStagingBuffer);
    }

    // Index buffer
    {
        VkBufferCreateInfo indexBufferInfo;
        indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferInfo.pNext = nullptr;
        indexBufferInfo.flags = 0;
        indexBufferInfo.size = m_indexBufferSize;
        indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        indexBufferInfo.queueFamilyIndexCount = 0;
        indexBufferInfo.pQueueFamilyIndices = nullptr;

        auto indexStagingBufferInfo = indexBufferInfo;
        indexStagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        vkCreateBuffer(m_renderDevice, &indexBufferInfo, nullptr, &m_indexBuffer);
        vkCreateBuffer(m_renderDevice, &indexStagingBufferInfo, nullptr, &m_indexStagingBuffer);
    }

    // Allocate vertex memory
    {
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_renderDevice, m_vertexStagingBuffer, &memoryRequirements);
        m_vertexStagingMemory = *memoryManager.allocateMemory(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        vkGetBufferMemoryRequirements(m_renderDevice, m_vertexBuffer, &memoryRequirements);
        m_vertexMemory = *memoryManager.allocateMemory(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    // Allocate index memory
    {
        VkMemoryRequirements memoryRequirements;
        m_numIndices = numTriangles * 3;
        vkGetBufferMemoryRequirements(m_renderDevice, m_indexStagingBuffer, &memoryRequirements);
        m_indexStagingMemory = *memoryManager.allocateMemory(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        vkGetBufferMemoryRequirements(m_renderDevice, m_indexBuffer, &memoryRequirements);
        m_indexMemory = *memoryManager.allocateMemory(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
}

void *
VulkanVertexBuffer::mapVertices()
{
    void * vertexData;
    vkMapMemory(m_renderDevice, m_vertexStagingMemory, 0, m_vertexBufferSize, 0, &vertexData);

    return vertexData;
}

void
VulkanVertexBuffer::unmapVertices()
{
    m_vertexBufferModified = true;
    vkUnmapMemory(m_renderDevice, m_vertexStagingMemory);
}

void *
VulkanVertexBuffer::mapTriangles()
{
    void * indexData;
    vkMapMemory(m_renderDevice, m_indexStagingMemory, 0, m_indexBufferSize, 0, &indexData);
    return indexData;
}

void
VulkanVertexBuffer::unmapTriangles()
{
    m_indexBufferModified = true;
    vkUnmapMemory(m_renderDevice, m_indexStagingMemory);
}

void
VulkanVertexBuffer::bind()
{
    // Testing
}

void
VulkanVertexBuffer::updateVertexBuffer(std::vector<VulkanBasicVertex> * vertices,
                                       std::vector<std::array<uint32_t, 3>> * triangles)
{
    auto local_vertices = (VulkanBasicVertex *)this->mapVertices();

    for (unsigned i = 0; i < vertices->size(); i++)
    {
        local_vertices[i].position = glm::vec3(
            (*vertices)[i].position.x,
            (*vertices)[i].position.y,
            (*vertices)[i].position.z);

        local_vertices[i].normal = glm::vec3(
            (*vertices)[i].normal.x,
            (*vertices)[i].normal.y,
            (*vertices)[i].normal.z);
    }

    this->unmapVertices();

    if (triangles != nullptr)
    {
        auto local_triangles = (std::array<uint32_t, 3> *) this->mapTriangles();

        for (unsigned i = 0; i < triangles->size(); i++)
        {
            local_triangles[i][0] = (*triangles)[i][0];
            local_triangles[i][1] = (*triangles)[i][1];
            local_triangles[i][2] = (*triangles)[i][2];
        }

        this->unmapTriangles();
    }
}

void
VulkanVertexBuffer::uploadBuffers(VkCommandBuffer& commandBuffer)
{
    if (m_vertexBufferModified)
    {
        VkBufferCopy copyInfo;
        copyInfo.size = m_vertexBufferSize;
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;

        vkCmdCopyBuffer(commandBuffer, m_vertexStagingBuffer, m_vertexBuffer, 1, &copyInfo);
    }
    if (m_indexBufferModified)
    {
        VkBufferCopy copyInfo;
        copyInfo.size = m_indexBufferSize;
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;

        vkCmdCopyBuffer(commandBuffer, m_indexStagingBuffer, m_indexBuffer, 1, &copyInfo);
    }
}

void
VulkanVertexBuffer::initializeBuffers(VulkanMemoryManager& memoryManager)
{
    vkBindBufferMemory(m_renderDevice, m_vertexBuffer, m_vertexMemory, 0);
    vkBindBufferMemory(m_renderDevice, m_indexBuffer, m_indexMemory, 0);
    vkBindBufferMemory(m_renderDevice, m_vertexStagingBuffer, m_vertexStagingMemory, 0);
    vkBindBufferMemory(m_renderDevice, m_indexStagingBuffer, m_indexStagingMemory, 0);

    // Start transfer commands
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(*memoryManager.m_transferCommandBuffer, &commandBufferBeginInfo);
    this->uploadBuffers(*memoryManager.m_transferCommandBuffer);
    vkEndCommandBuffer(*memoryManager.m_transferCommandBuffer);

    VkCommandBuffer commandBuffers[] = { *memoryManager.m_transferCommandBuffer };

    VkPipelineStageFlags stageWaitFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[1];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount = 0;
    submitInfo[0].pWaitSemaphores = nullptr;
    submitInfo[0].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = commandBuffers;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = nullptr;

    vkQueueSubmit(*memoryManager.m_transferQueue, 1, submitInfo, nullptr);
    vkDeviceWaitIdle(memoryManager.m_device);
}

void
VulkanVertexBuffer::setNumIndices(uint32_t numIndices)
{
    m_numIndices = numIndices;
}
}