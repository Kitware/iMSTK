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

#ifndef imstkVulkanMemoryManager_h
#define imstkVulkanMemoryManager_h

#include "vulkan/vulkan.h"

#include <vector>

#include "g3log/g3log.hpp"

namespace imstk
{
class VulkanMemoryManager
{
public:
    VulkanMemoryManager();
    void clear();

    VkDeviceMemory * allocateMemory(
        const VkMemoryRequirements& memoryRequirements,
        uint32_t location);

    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    uint32_t m_queueFamilyIndex;
    VkCommandBuffer * m_transferCommandBuffer;
    VkQueue * m_transferQueue;

protected:
    std::vector<VkDeviceMemory*> m_memoryAllocations;
};
};

#endif