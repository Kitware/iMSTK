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

namespace imstk
{
enum VulkanMemoryType
{
    TEXTURE,
    STAGING_TEXTURE,
    VERTEX,
    STAGING_VERTEX,
    INDEX,
    STAGING_INDEX,
    UNIFORM,
    STAGING_UNIFORM,
    FRAMEBUFFER
};

class VulkanInternalMemory
{
protected:
    friend class VulkanMemoryManager;
    friend class VulkanInternalBuffer;
    friend class VulkanInternalImage;
    friend class VulkanInternalResource;

    VulkanInternalMemory()
    {
        m_memory = new VkDeviceMemory();
    }

    VkDeviceSize    m_size;
    VkDeviceSize    m_capacity;
    VkDeviceSize    m_lastOffset;
    VkDeviceMemory* m_memory;

    VkDeviceMemory* getMemory()
    {
        return m_memory;
    }
};

class VulkanInternalResource
{
public:
    VulkanInternalMemory* getMemory()
    {
        return m_memory;
    }

    void unmapMemory(VkDevice& device)
    {
        vkUnmapMemory(device, *m_memory->m_memory);
    }

protected:
    VulkanInternalMemory* m_memory;
    VkDeviceSize          m_size;
};

class VulkanInternalImage : public VulkanInternalResource
{
public:
    VulkanInternalImage(VkImage* image = nullptr)
    {
        if (image)
        {
            m_image = image;
        }
        else
        {
            m_image = new VkImage();
        }
    }

    VkImage* getImage()
    {
        return m_image;
    }

    void mapMemory(VkDevice& device, void** data)
    {
        vkMapMemory(device, *m_memory->m_memory, m_memoryOffset, m_size, 0, data);
    }

    void setImageLayout(const VkImageLayout imageLayout)
    {
        m_imageLayout = imageLayout;
    }

    const VkImageLayout getImageLayout()
    {
        return m_imageLayout;
    }

protected:
    friend class VulkanMemoryManager;

    VkImage*      m_image;
    VkDeviceSize  m_memoryOffset;
    VkImageLayout m_imageLayout;
};

class VulkanInternalBufferGroup : public VulkanInternalResource
{
public:
    VulkanInternalBufferGroup(VulkanMemoryType type)
    {
        m_type   = type;
        m_buffer = new VkBuffer();
    }

    VkBuffer* getBuffer()
    {
        return m_buffer;
    }

    VkDeviceSize getSize()
    {
        return m_size;
    }

protected:
    friend class VulkanMemoryManager;
    friend class VulkanInternalBuffer;

    VkBuffer*        m_buffer;
    char*            m_data;
    VulkanMemoryType m_type;
    bool m_mapped = false;
};

class VulkanInternalBuffer
{
public:
    VulkanInternalBuffer(VulkanInternalBufferGroup* bufferGroup)
    {
        m_bufferGroup = bufferGroup;
    }

    VkBuffer* getBuffer()
    {
        return m_bufferGroup->getBuffer();
    }

    VulkanInternalMemory* getMemory()
    {
        return m_bufferGroup->getMemory();
    }

    void* getMemoryData(VkDevice& device)
    {
        void* data;

        if (!m_bufferGroup->m_mapped)
        {
            auto result = vkMapMemory(device,
                                *m_bufferGroup->getMemory()->m_memory,
                                0,
                                m_bufferGroup->getMemory()->m_capacity,
                                0,
                                &data);
            m_bufferGroup->m_data   = (char*)data;
            m_bufferGroup->m_mapped = true;
        }

        return &m_bufferGroup->m_data[m_offset];
    }

    void unmapMemory(VkDevice& device)
    {
        vkUnmapMemory(device, *m_bufferGroup->getMemory()->m_memory);
    }

    VkDeviceSize getSize()
    {
        return m_size;
    }

    VkDeviceSize getOffset()
    {
        return m_offset;
    }

protected:
    friend class VulkanMemoryManager;

    VulkanInternalBufferGroup* m_bufferGroup;
    VkDeviceSize m_size;
    VkDeviceSize m_offset;
};
} // imstk
