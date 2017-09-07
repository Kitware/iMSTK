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

VkDeviceMemory *
VulkanMemoryManager::allocateMemory(
    const VkMemoryRequirements& memoryRequirements,
    uint32_t location)
{
    auto memory = new VkDeviceMemory();

    // Get device memory types
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &deviceMemoryProperties);

    uint32_t memoryIndex = 0;
    bool foundIndex = false;

    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
    {
        // Determine if the memory type is valid for this resource
        if ((1 << i) & memoryRequirements.memoryTypeBits)
        {
            // Determine if the memory type has the correct properties
            if (deviceMemoryProperties.memoryTypes[i].propertyFlags & location)
            {
                memoryIndex = i;
                foundIndex = true;
                break;
            }
        }
    }

    if (!foundIndex)
    {
        LOG(FATAL) << "Couldn't find correct memory object";
    }

    VkMemoryAllocateInfo memoryInfo;
    memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.pNext = nullptr;
    memoryInfo.allocationSize = memoryRequirements.size;
    memoryInfo.memoryTypeIndex = memoryIndex;

    vkAllocateMemory(m_device, &memoryInfo, nullptr, memory);

    m_memoryAllocations.push_back(memory);

    return memory;
}

void
VulkanMemoryManager::clear()
{
    for (unsigned int i = 0; i < m_memoryAllocations.size(); i++)
    {
        vkFreeMemory(m_device, *m_memoryAllocations[i], nullptr);
    }
}
};