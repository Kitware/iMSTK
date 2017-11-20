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

class VulkanAttachmentBarriers
{
public:
    static void addColorAttachmentBarrier(VkCommandBuffer * commandBuffer, uint32_t queueFamilyIndex, VkImage * image)
    {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image;
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr, // general memory barriers
            0, nullptr, // buffer barriers
            1, &barrier); // image barriers
    };

    static void addDepthAttachmentBarrier(VkCommandBuffer * commandBuffer, uint32_t queueFamilyIndex, VkImage * image)
    {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image;
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr, // general memory barriers
            0, nullptr, // buffer barriers
            1, &barrier); // image barriers
    };

    static void addShadowAttachmentBarrier(VkCommandBuffer * commandBuffer, uint32_t queueFamilyIndex, VkImage * image)
    {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image;
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr, // general memory barriers
            0, nullptr, // buffer barriers
            1, &barrier); // image barriers
    };
};

}

#endif