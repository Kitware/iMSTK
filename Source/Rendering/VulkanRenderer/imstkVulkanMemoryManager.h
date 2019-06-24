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

#include <vector>

#include "g3log/g3log.hpp"

#include "imstkVulkanResources.h"

namespace imstk
{
class VulkanMemoryManager
{
public:
    VulkanMemoryManager();
    void setup(VkPhysicalDevice * device);
    void clear();

    ///
    /// \brief Request a Vulkan buffer object
    /// \param device Vulkan device
    /// \param info Buffer information struct
    /// \param type Memory type needed
    /// \param offsetAlignment Alignment information
    /// \returns Vulkan buffer object
    ///
    VulkanInternalBuffer * requestBuffer(VkDevice& device,
                                         VkBufferCreateInfo& info,
                                         VulkanMemoryType type,
                                         VkDeviceSize offsetAlignment = 0);

    ///
    /// \brief Request a Vulkan image object
    /// \param device Vulkan device
    /// \param info Image information struct
    /// \param type Memory type needed
    /// \returns Vulkan image object
    ///
    VulkanInternalImage * requestImage(VkDevice& device,
                                       VkImageCreateInfo& info,
                                       VulkanMemoryType type);

    VkPhysicalDevice * m_physicalDevice;
    VkDevice m_device;

    VkPhysicalDeviceProperties m_deviceProperties;
    VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;

    uint32_t m_queueFamilyIndex;
    VkCommandBuffer * m_transferCommandBuffer;
    VkQueue * m_transferQueue;
    uint32_t m_buffering = 3;

protected:
    ///
    /// \brief Request a Vulkan memory allocation. If none exist, then a new allocation is created.
    /// \param memoryRequirements Memory requirements struct
    /// \param type Memory type
    /// \param maxAllocationSize Maximum allocation size
    /// \param offsetAlignment Alignment information for the offset
    /// \returns Vulkan memory allocation
    ///
    VulkanInternalMemory * requestMemoryAllocation(
        const VkMemoryRequirements& memoryRequirements,
        VulkanMemoryType type,
        VkDeviceSize maxAllocationSize,
        VkDeviceSize offsetAlignment = 0);

    static const VkDeviceSize c_bufferAllocationSize = 16 * 1024 * 1024; // 16 MiB
    static const VkDeviceSize c_imageAllocationSize = 128 * 1024 * 1024; // 128 MiB
    static VkDeviceSize getAlignedSize(VkDeviceSize size, VkDeviceSize alignment);

    std::map<VulkanMemoryType, std::vector<VulkanInternalMemory*>> m_memoryAllocations;
    std::vector<VulkanInternalBufferGroup*> m_buffers;
    std::vector<VulkanInternalImage*> m_images;
};
};

