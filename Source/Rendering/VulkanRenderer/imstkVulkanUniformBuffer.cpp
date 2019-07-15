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
    uniformBufferInfo.size  = uniformSize * memoryManager.m_buffering;
    uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBufferInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    uniformBufferInfo.queueFamilyIndexCount = 0;
    uniformBufferInfo.pQueueFamilyIndices   = nullptr;

    m_uniformBuffer = memoryManager.requestBuffer(m_renderDevice, uniformBufferInfo, VulkanMemoryType::STAGING_UNIFORM);

    // Allocate vertex memory
    m_uniformBufferSize = uniformSize;
}

void
VulkanUniformBuffer::updateUniforms(uint32_t uniformSize, void* uniformData, uint32_t frameIndex)
{
    auto uniformMemory = this->getUniformMemory();

    memcpy(uniformMemory, uniformData, uniformSize);
}

VulkanInternalBuffer*
VulkanUniformBuffer::getUniformBuffer()
{
    return m_uniformBuffer;
}

void*
VulkanUniformBuffer::getUniformMemory()
{
    return m_uniformBuffer->getMemoryData(m_renderDevice);
}
}