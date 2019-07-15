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

#include "imstkVulkanMemoryManager.h"

namespace imstk
{
VulkanMemoryManager::VulkanMemoryManager()
{
}

void
VulkanMemoryManager::setup(VkPhysicalDevice* device)
{
    m_physicalDevice = device;
    vkGetPhysicalDeviceProperties(*device, &m_deviceProperties);
    vkGetPhysicalDeviceMemoryProperties(*device, &m_deviceMemoryProperties);
}

VulkanInternalMemory*
VulkanMemoryManager::requestMemoryAllocation(
    const VkMemoryRequirements& memoryRequirements,
    VulkanMemoryType            type,
    VkDeviceSize                maxAllocationSize,
    VkDeviceSize                offsetAlignment)
{
    VkDeviceSize resourceSize = getAlignedSize(memoryRequirements.size, memoryRequirements.alignment);

    // If allocation with space exists, return that
    for (auto memoryAllocation : m_memoryAllocations[type])
    {
        auto alignedSize = this->getAlignedSize(memoryAllocation->m_size, offsetAlignment);
        if (memoryAllocation->m_capacity - alignedSize >= resourceSize)
        {
            memoryAllocation->m_lastOffset = alignedSize;
            memoryAllocation->m_size       = alignedSize + resourceSize;
            return memoryAllocation;
        }
    }

    // Get device memory types
    uint32_t memoryIndex = 0;
    bool     foundIndex  = false;

    uint32_t location;

    // Native resources get memory allocations on the GPU for faster performance
    switch (type)
    {
    case TEXTURE:
    case VERTEX:
    case INDEX:
    case UNIFORM:
    case FRAMEBUFFER:
        location = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case STAGING_TEXTURE:
    case STAGING_VERTEX:
    case STAGING_INDEX:
    case STAGING_UNIFORM:
        location = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    default:
        break;
    }

    for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
    {
        // Determine if the memory type is valid for this resource
        if ((1 << i) & memoryRequirements.memoryTypeBits)
        {
            // Determine if the memory type has the correct properties
            if (m_deviceMemoryProperties.memoryTypes[i].propertyFlags & location)
            {
                memoryIndex = i;
                foundIndex  = true;
                break;
            }
        }
    }

    if (!foundIndex)
    {
        LOG(FATAL) << "Couldn't find correct memory object";
    }

    VkDeviceSize allocationSize = std::max(resourceSize, maxAllocationSize);

    VkMemoryAllocateInfo memoryInfo;
    memoryInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.pNext           = nullptr;
    memoryInfo.allocationSize  = allocationSize;
    memoryInfo.memoryTypeIndex = memoryIndex;

    VulkanInternalMemory* internalMemory = new VulkanInternalMemory();
    internalMemory->m_capacity   = memoryInfo.allocationSize;
    internalMemory->m_size       = resourceSize;
    internalMemory->m_lastOffset = 0;

    if (vkAllocateMemory(m_device, &memoryInfo, nullptr, internalMemory->m_memory) != VK_SUCCESS)
    {
        LOG(INFO) << "Cannot allocate memory";
    }

    m_memoryAllocations[type].push_back(internalMemory);

    return internalMemory;
}

VulkanInternalBuffer*
VulkanMemoryManager::requestBuffer(VkDevice&           device,
                                   VkBufferCreateInfo& info,
                                   VulkanMemoryType    type,
                                   VkDeviceSize        offsetAlignment)
{
    VulkanInternalBuffer* newBuffer;

    VkDeviceSize allocationSize;
    switch (type)
    {
    case UNIFORM:
    case STAGING_UNIFORM:
        allocationSize = 64 * 1024;
        info.size      = getAlignedSize(info.size, m_deviceProperties.limits.minUniformBufferOffsetAlignment);
        break;
    default:
        allocationSize = c_bufferAllocationSize;
        break;
    }

    // Check for existing buffer with space
    for (auto buffer : m_buffers)
    {
        auto tempSize = this->getAlignedSize(buffer->m_size, offsetAlignment);
        if (buffer->m_type == type
            && info.size <= buffer->m_memory->m_capacity - tempSize)
        {
            newBuffer           = new VulkanInternalBuffer(buffer);
            newBuffer->m_offset = tempSize;
            newBuffer->m_size   = info.size;
            buffer->m_size      = tempSize + info.size;

            return newBuffer;
        }
    }

    // If room not found, allocate a new buffer
    auto bufferGroup = new VulkanInternalBufferGroup(type);

    auto bufferSize = info.size;
    info.size = std::max(info.size, allocationSize);
    m_buffers.push_back(bufferGroup);
    bufferGroup->m_size = bufferSize;

    vkCreateBuffer(device, &info, nullptr, bufferGroup->m_buffer);

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, *bufferGroup->getBuffer(), &requirements);
    auto memoryAllocation = this->requestMemoryAllocation(requirements, type, allocationSize);
    vkBindBufferMemory(device, *bufferGroup->getBuffer(), *memoryAllocation->getMemory(), 0);
    bufferGroup->m_memory = memoryAllocation;

    newBuffer           = new VulkanInternalBuffer(bufferGroup);
    newBuffer->m_size   = bufferSize;
    newBuffer->m_offset = 0;

    return newBuffer;
}

// Needs work
VulkanInternalImage*
VulkanMemoryManager::requestImage(VkDevice&          device,
                                  VkImageCreateInfo& info,
                                  VulkanMemoryType   type)
{
    // Always allocate a new image
    auto newImage = new VulkanInternalImage();
    vkCreateImage(device, &info, nullptr, newImage->getImage());
    newImage->setImageLayout(info.initialLayout);

    m_images.push_back(newImage);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, *newImage->getImage(), &requirements);

    newImage->m_memoryOffset = 0;
    newImage->m_size         = requirements.size;

    auto memoryAllocation = this->requestMemoryAllocation(requirements, type, c_imageAllocationSize, requirements.alignment);
    newImage->m_memoryOffset = this->getAlignedSize(memoryAllocation->m_lastOffset, requirements.alignment);
    vkBindImageMemory(device, *newImage->getImage(), *memoryAllocation->getMemory(), newImage->m_memoryOffset);

    return newImage;
}

void
VulkanMemoryManager::clear()
{
    for (unsigned int i = 0; i < m_buffers.size(); i++)
    {
        vkDestroyBuffer(m_device, *m_buffers[i]->m_buffer, nullptr);
    }

    for (unsigned int i = 0; i < m_images.size(); i++)
    {
        vkDestroyImage(m_device, *m_images[i]->m_image, nullptr);
    }

    for (auto i : m_memoryAllocations)
    {
        for (auto j : i.second)
        {
            vkFreeMemory(m_device, *j->getMemory(), nullptr);
        }
    }
}

VkDeviceSize
VulkanMemoryManager::getAlignedSize(VkDeviceSize size, VkDeviceSize alignment)
{
    if (alignment == 0)
    {
        return size;
    }

    VkDeviceSize newSize = (size / alignment) * alignment;

    if (size % alignment != 0)
    {
        newSize += alignment;
    }

    return newSize;
}
}