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

#include "imstkVulkanUniformBuffer.h"

namespace imstk
{
VulkanUniformBuffer::VulkanUniformBuffer(VulkanMemoryManager& memoryManager, uint32_t uniformSize)
{
    m_renderDevice = memoryManager.m_device;

    // Uniform buffer
    VkBufferCreateInfo uniformBufferInfo;
    uniformBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniformBufferInfo.pNext = nullptr;
    uniformBufferInfo.flags = 0;
    uniformBufferInfo.size = uniformSize;
    uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uniformBufferInfo.queueFamilyIndexCount = 0;
    uniformBufferInfo.pQueueFamilyIndices = nullptr;

    vkCreateBuffer(m_renderDevice, &uniformBufferInfo, nullptr, &m_uniformBuffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_renderDevice, m_uniformBuffer, &memoryRequirements);

    // Allocate vertex memory
    m_uniformBufferSize = uniformSize;

    m_uniformMemory = *memoryManager.allocateMemory(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    this->Bind();
}

void
VulkanUniformBuffer::updateUniforms(uint32_t uniformSize, void * uniformData)
{
    auto uniformMemory = this->mapUniforms();

    memcpy(uniformMemory, uniformData, uniformSize);

    this->unmapUniforms();
}

VkBuffer *
VulkanUniformBuffer::getUniformBuffer()
{
    return &m_uniformBuffer;
}

void *
VulkanUniformBuffer::mapUniforms()
{
    void * uniformData;
    vkMapMemory(m_renderDevice, m_uniformMemory, 0, m_uniformBufferSize, 0, &uniformData);

    return uniformData;
}

void
VulkanUniformBuffer::unmapUniforms()
{
    vkUnmapMemory(m_renderDevice, m_uniformMemory);
}

void
VulkanUniformBuffer::Bind()
{
    vkBindBufferMemory(m_renderDevice, m_uniformBuffer, m_uniformMemory, 0);
}
}