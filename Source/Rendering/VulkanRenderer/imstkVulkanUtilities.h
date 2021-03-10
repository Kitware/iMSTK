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

#include <vector>
#include <memory>
#include <fstream>
#include <string>

#include "vulkan/vulkan.h"

#include "imstkVulkanResources.h"

#include "g3log/g3log.hpp"

namespace imstk
{
namespace VulkanShaderPath
{
const std::string Mesh("../data/shaders/VulkanShaders/mesh/");
const std::string PostProcessing("../data/shaders/VulkanShaders/PostProcessing/");
}

/// \brief TODO
class VulkanShaderLoader
{
public:
    /// \brief TODO
    VulkanShaderLoader(std::string filename, VkDevice& device, VkShaderModule& module)
    {
        std::ifstream file(filename, std::ios_base::binary);
        m_data = std::make_shared<std::vector<char>>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
        file.close();

        VkShaderModuleCreateInfo vertexShaderInfo;
        vertexShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderInfo.pNext    = nullptr;
        vertexShaderInfo.flags    = 0;
        vertexShaderInfo.codeSize = this->getShaderLength();
        vertexShaderInfo.pCode    = this->getShaderData();
        if (vkCreateShaderModule(device, &vertexShaderInfo, nullptr, &module) != VK_SUCCESS)
        {
            LOG(FATAL) << "Unable to build vertex shader : " << filename;
        }
    };

    /// \brief TODO
    uint32_t getShaderLength() { return (uint32_t)m_data->size(); };

    /// \brief TODO
    uint32_t* getShaderData() { return (uint32_t*)m_data->data(); };

protected:
    std::shared_ptr<std::vector<char>> m_data;
};

/// \brief TODO
class VulkanAttachmentBarriers
{
public:
    /// \brief TODO
    static void changeImageLayout(
        VkCommandBuffer*     commandBuffer,
        uint32_t             queueFamilyIndex,
        VulkanInternalImage* image,
        VkImageLayout        oldLayout,
        VkImageLayout        newLayout,
        const uint32_t       numViews)
    {
        // Don't change layout if already there
        if (image->getImageLayout() == newLayout)
        {
            return;
        }

        // Get corresponding access and stage flags
        VkAccessFlags        srcAccess = getAccessFlags(oldLayout);
        VkAccessFlags        dstAccess = getAccessFlags(newLayout);
        VkPipelineStageFlags srcPipelineStage = getPipelineStageFlags(oldLayout);
        VkPipelineStageFlags dstPipelineStage = getPipelineStageFlags(newLayout);

        VkImageSubresourceRange range;
        range.aspectMask = ((oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
                            || (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) ?
                           VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel   = 0;
        range.levelCount     = 1;
        range.baseArrayLayer = 0;
        range.layerCount     = numViews;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask       = srcAccess;
        barrier.dstAccessMask       = dstAccess;
        barrier.oldLayout           = oldLayout;
        barrier.newLayout           = newLayout;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image->getImage();
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            srcPipelineStage,
            dstPipelineStage,
            0,
            0, nullptr,     // general memory barriers
            0, nullptr,     // buffer barriers
            1, &barrier);   // image barriers

        // For keeping track
        image->setImageLayout(newLayout);
    };

    /// \brief TODO
    static const VkAccessFlags getAccessFlags(VkImageLayout imageLayout)
    {
        switch (imageLayout)
        {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        default:
            LOG(WARNING) << "Unsupported image layout";
            return VK_ACCESS_SHADER_READ_BIT; // This might not be a great default
        }
        ;
    }

    /// \brief TODO
    static const VkPipelineStageFlags getPipelineStageFlags(VkImageLayout imageLayout)
    {
        switch (imageLayout)
        {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        default:
            LOG(WARNING) << "Unsupported image layout";
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // This might not be a great default
        }
        ;
    }

    /// \brief TODO
    static void addDepthAttachmentBarrier(VkCommandBuffer* commandBuffer, uint32_t queueFamilyIndex, VkImage* image)
    {
        VkImageSubresourceRange range;
        range.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.baseMipLevel   = 0;
        range.levelCount     = 1;
        range.baseArrayLayer = 0;
        range.layerCount     = 1;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image;
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,     // general memory barriers
            0, nullptr,     // buffer barriers
            1, &barrier);   // image barriers
    };

    /// \brief TODO
    static void addShadowAttachmentBarrier(VkCommandBuffer* commandBuffer, uint32_t queueFamilyIndex, VkImage* image)
    {
        VkImageSubresourceRange range;
        range.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.baseMipLevel   = 0;
        range.levelCount     = 1;
        range.baseArrayLayer = 0;
        range.layerCount     = 1;

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = queueFamilyIndex;
        barrier.dstQueueFamilyIndex = queueFamilyIndex;
        barrier.image = *image;
        barrier.subresourceRange = range;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,     // general memory barriers
            0, nullptr,     // buffer barriers
            1, &barrier);   // image barriers
    };
};

/// \brief TODO
class VulkanDefaults
{
public:
    static const VkComponentMapping getDefaultComponentMapping()
    {
        VkComponentMapping componentMapping;
        componentMapping.r = VK_COMPONENT_SWIZZLE_R;
        componentMapping.g = VK_COMPONENT_SWIZZLE_G;
        componentMapping.b = VK_COMPONENT_SWIZZLE_B;
        componentMapping.a = VK_COMPONENT_SWIZZLE_A;
        return componentMapping;
    }
};

namespace VulkanFormats
{
static const VkFormat FINAL_FORMAT      = VK_FORMAT_B8G8R8A8_SRGB;       /// Linear color space
static const VkFormat HDR_FORMAT        = VK_FORMAT_R16G16B16A16_SFLOAT; // HDR internal format
static const VkFormat NORMAL_SSS_FORMAT = VK_FORMAT_R8G8B8A8_SNORM;      // Normal/SSS format
static const VkFormat AO_FORMAT        = VK_FORMAT_R8_UNORM;             // AO format
static const VkFormat DEPTH_FORMAT     = VK_FORMAT_D32_SFLOAT;           // Depth buffer
static const VkFormat SHADOW_FORMAT    = VK_FORMAT_D32_SFLOAT;           // Format for shadow maps
static const VkFormat DEPTH_MIP_FORMAT = VK_FORMAT_R32_SFLOAT;           // Depth mip buffer
};
}
