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
VulkanVertexBuffer::VulkanVertexBuffer(VulkanMemoryManager&   memoryManager,
                                       unsigned int           numVertices,
                                       unsigned int           vertexSize,
                                       unsigned int           numTriangles,
                                       double                 loadFactor,
                                       VulkanVertexBufferMode mode)
{
    m_mode = mode;
    m_renderDevice     = memoryManager.m_device;
    m_buffering        = (m_mode == VulkanVertexBufferMode::VERTEX_BUFFER_STATIC) ? 1 : memoryManager.m_buffering;
    m_vertexBufferSize = (uint32_t)(numVertices * vertexSize * loadFactor) * m_buffering;
    m_numIndices       = numTriangles * 3;
    m_indexBufferSize  = (uint32_t)(m_numIndices * sizeof(uint32_t) * loadFactor) * m_buffering;

    // Vertex buffer
    {
        VkBufferCreateInfo vertexBufferInfo;
        vertexBufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.pNext       = nullptr;
        vertexBufferInfo.flags       = 0;
        vertexBufferInfo.size        = m_vertexBufferSize;
        vertexBufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vertexBufferInfo.queueFamilyIndexCount = 0;
        vertexBufferInfo.pQueueFamilyIndices   = nullptr;

        auto vertexStagingBufferInfo = vertexBufferInfo;
        vertexStagingBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (m_mode == VERTEX_BUFFER_STATIC)
        {
            vertexStagingBufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        m_vertexStagingBuffer = memoryManager.requestBuffer(m_renderDevice,
                        vertexStagingBufferInfo,
                        VulkanMemoryType::STAGING_VERTEX);

        if (m_mode == VERTEX_BUFFER_STATIC)
        {
            vertexBufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            m_vertexBuffer = memoryManager.requestBuffer(m_renderDevice,
                            vertexBufferInfo,
                            VulkanMemoryType::VERTEX);
        }
        else
        {
            m_vertexBuffer = m_vertexStagingBuffer;
        }
    }

    // Index buffer
    {
        VkBufferCreateInfo indexBufferInfo;
        indexBufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferInfo.pNext       = nullptr;
        indexBufferInfo.flags       = 0;
        indexBufferInfo.size        = m_indexBufferSize;
        indexBufferInfo.usage       = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        indexBufferInfo.queueFamilyIndexCount = 0;
        indexBufferInfo.pQueueFamilyIndices   = nullptr;

        auto indexStagingBufferInfo = indexBufferInfo;
        indexStagingBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (m_mode == VERTEX_BUFFER_STATIC)
        {
            indexStagingBufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        m_indexStagingBuffer = memoryManager.requestBuffer(m_renderDevice,
                        indexStagingBufferInfo,
                        VulkanMemoryType::STAGING_INDEX);

        if (m_mode == VERTEX_BUFFER_STATIC)
        {
            indexBufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            m_indexBuffer = memoryManager.requestBuffer(m_renderDevice,
                            indexBufferInfo,
                            VulkanMemoryType::INDEX);
        }
        else
        {
            m_indexBuffer = m_indexStagingBuffer;
        }
    }
}

void*
VulkanVertexBuffer::getVertexMemory(uint32_t frameIndex)
{
    auto memory = (char*)m_vertexStagingBuffer->getMemoryData(m_renderDevice);
    auto offset = frameIndex * m_vertexStagingBuffer->getSize() / m_buffering;
    return (void*)&memory[offset];
}

void*
VulkanVertexBuffer::getIndexMemory(uint32_t frameIndex)
{
    auto memory = (char*)m_indexStagingBuffer->getMemoryData(m_renderDevice);
    auto offset = frameIndex * m_indexStagingBuffer->getSize() / m_buffering;
    return (void*)&memory[offset];
}

void
VulkanVertexBuffer::updateVertexBuffer(std::vector<VulkanBasicVertex>* vertices,
                                       std::vector<std::array<uint32_t, 3>>* triangles)
{
    auto local_vertices = (VulkanBasicVertex*)this->getVertexMemory();

    for (unsigned int i = 0; i < vertices->size(); i++)
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

    if (triangles != nullptr)
    {
        auto local_triangles = (std::array<uint32_t, 3>*) this->getIndexMemory();

        for (unsigned int i = 0; i < triangles->size(); i++)
        {
            local_triangles[i][0] = (*triangles)[i][0];
            local_triangles[i][1] = (*triangles)[i][1];
            local_triangles[i][2] = (*triangles)[i][2];
        }
    }
}

void
VulkanVertexBuffer::uploadBuffers(VkCommandBuffer& commandBuffer)
{
    if (m_mode != VERTEX_BUFFER_STATIC)
    {
        return;
    }

    if (m_vertexBufferModified)
    {
        VkBufferCopy copyInfo;
        copyInfo.size      = m_vertexBufferSize;
        copyInfo.srcOffset = m_vertexStagingBuffer->getOffset();
        copyInfo.dstOffset = m_vertexBuffer->getOffset();

        vkCmdCopyBuffer(commandBuffer,
                        *m_vertexStagingBuffer->getBuffer(),
                        *m_vertexBuffer->getBuffer(),
                        1,
                        &copyInfo);

        m_vertexBufferModified = false;
    }
    if (m_indexBufferModified)
    {
        VkBufferCopy copyInfo;
        copyInfo.size      = m_indexBufferSize;
        copyInfo.srcOffset = m_indexStagingBuffer->getOffset();
        copyInfo.dstOffset = m_indexBuffer->getOffset();

        vkCmdCopyBuffer(commandBuffer,
                        *m_indexStagingBuffer->getBuffer(),
                        *m_indexBuffer->getBuffer(),
                        1,
                        &copyInfo);

        m_indexBufferModified = false;
    }
}

void
VulkanVertexBuffer::initializeBuffers(VulkanMemoryManager& memoryManager)
{
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
    VkSubmitInfo         submitInfo[1];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount   = 0;
    submitInfo[0].pWaitSemaphores      = nullptr;
    submitInfo[0].pWaitDstStageMask    = &stageWaitFlags;
    submitInfo[0].commandBufferCount   = 1;
    submitInfo[0].pCommandBuffers      = commandBuffers;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores    = nullptr;

    vkQueueSubmit(*memoryManager.m_transferQueue, 1, submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(memoryManager.m_device);
}

void
VulkanVertexBuffer::setNumIndices(uint32_t numIndices)
{
    m_numIndices = numIndices;
}

void
VulkanVertexBuffer::bindBuffers(VkCommandBuffer* commandBuffer, uint32_t frameIndex)
{
    auto index = (m_mode == VulkanVertexBufferMode::VERTEX_BUFFER_STATIC) ? 0 : frameIndex;
    auto vertexOffset = m_vertexBuffer->getOffset() + (index * m_vertexBuffer->getSize() / m_buffering);
    auto indexOffset  = m_indexBuffer->getOffset() + (index * m_indexBuffer->getSize() / m_buffering);
    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, m_vertexBuffer->getBuffer(), &vertexOffset);
    vkCmdBindIndexBuffer(*commandBuffer, *m_indexBuffer->getBuffer(), indexOffset, VK_INDEX_TYPE_UINT32);
}

void
VulkanVertexBuffer::setModified()
{
    m_vertexBufferModified = true;
    m_indexBufferModified  = true;
}

VulkanVertexBufferMode
VulkanVertexBuffer::getMode()
{
    return m_mode;
}
}