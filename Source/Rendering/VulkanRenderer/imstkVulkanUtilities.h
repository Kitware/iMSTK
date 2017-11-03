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

#ifndef imstkVulkanUtilities_h
#define imstkVulkanUtilities_h

#include <vector>
#include <memory>
#include <fstream>

#include "vulkan/vulkan.h"

namespace imstk
{

class VulkanShaderLoader
{
public:
    VulkanShaderLoader(std::string filename, VkDevice &device, VkShaderModule &module)
    {
        std::ifstream file(filename, std::ios_base::binary);
        m_data = std::make_shared<std::vector<char>>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
        file.close();

        VkShaderModuleCreateInfo vertexShaderInfo;
        vertexShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderInfo.pNext = nullptr;
        vertexShaderInfo.flags = 0;
        vertexShaderInfo.codeSize = this->getShaderLength();
        vertexShaderInfo.pCode = this->getShaderData();
        if (vkCreateShaderModule(device, &vertexShaderInfo, nullptr, &module) != VK_SUCCESS)
        {
            LOG(FATAL) << "Unable to build vertex shader : " << filename;
        }
    };

    uint32_t getShaderLength() { return (uint32_t)m_data->size(); };

    uint32_t * getShaderData() { return (uint32_t*)m_data->data(); };

protected:
    std::shared_ptr<std::vector<char>> m_data;
};

}

#endif