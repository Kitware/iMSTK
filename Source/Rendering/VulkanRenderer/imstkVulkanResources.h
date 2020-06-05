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
/// \brief TODO
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

/// \brief TODO
class VulkanInternalMemory
{
protected:
    friend class VulkanMemoryManager;
    friend class VulkanInternalBuffer;
    friend class VulkanInternalImage;
    friend class VulkanInternalResource;

    /// \brief TODO
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

/// \brief TODO
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
    VkDeviceSize m_size;
};

/// \brief TODO
class VulkanInternalImage : public VulkanInternalResource
{
public:

    /// \brief Constructor
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
    };

    /// \brief Get the image
    VkImage* getImage()
    {
        return m_image;
    }

    /// \brief TODO
    void mapMemory(VkDevice& device, void** data)
    {
        vkMapMemory(device, *m_memory->m_memory, m_memoryOffset, m_size, 0, data);
    }

    ///
    /// \brief Set imange layout
    ///
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

/// \brief
class VulkanInternalBufferGroup : public VulkanInternalResource
{
public:
    ///
    /// \brief Constructor
    ///
    explicit VulkanInternalBufferGroup(VulkanMemoryType type)
    {
        m_type   = type;
        m_buffer = new VkBuffer();
    };

    ///
    /// \brief TODO
    ///
    VkBuffer* getBuffer()
    {
        return m_buffer;
    }

    ///
    /// \brief TODO
    ///
    VkDeviceSize getSize()
    {
        return m_size;
    }

protected:
    friend class VulkanMemoryManager;
    friend class VulkanInternalBuffer;

    VkBuffer* m_buffer;
    char*     m_data;
    VulkanMemoryType m_type;
    bool m_mapped = false;
};

class VulkanInternalBuffer
{
public:
    /// \brief TODO
    explicit VulkanInternalBuffer(VulkanInternalBufferGroup* bufferGroup)
    {
        m_bufferGroup = bufferGroup;
    };

    /// \brief TODO
    VkBuffer* getBuffer()
    {
        return m_bufferGroup->getBuffer();
    }

    /// \brief TODO
    VulkanInternalMemory* getMemory()
    {
        return m_bufferGroup->getMemory();
    }

    /// \brief TODO
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

    /// \brief TODO
    void unmapMemory(VkDevice& device)
    {
        vkUnmapMemory(device, *m_bufferGroup->getMemory()->m_memory);
    }

    /// \brief TODO
    VkDeviceSize getSize()
    {
        return m_size;
    }

    /// \brief TODO
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
