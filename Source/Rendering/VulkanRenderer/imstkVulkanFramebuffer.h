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

#include "imstkVulkanMemoryManager.h"

#include <array>
#include <vector>

namespace imstk
{
class VulkanFramebuffer
{
public:
    VulkanFramebuffer(
        VulkanMemoryManager& memoryManager,
        unsigned int width,
        unsigned int height,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    void setColor(VulkanInternalImage * image,
                  VkImageView * imageView,
                  VkFormat format,
                  VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    void setSpecular(VulkanInternalImage * image,
                     VkImageView * imageView,
                     VkFormat format,
                     VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    void setDepth(VulkanInternalImage * image,
                  VkImageView * imageView,
                  VkFormat format,
                  VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    void setNormal(VulkanInternalImage * image,
                   VkImageView * imageView,
                   VkFormat format,
                   VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    void initializeFramebuffer(VkRenderPass * renderPass);

    ~VulkanFramebuffer(){};

    void changeImageLayout(VkCommandBuffer& commandBuffer,
                           VkImage& image,
                           VkImageLayout layout1,
                           VkImageLayout layout2,
                           VkAccessFlags sourceFlags,
                           VkAccessFlags destinationFlags,
                           VkImageSubresourceRange range);

    void clear(VkDevice * device);

private:
    friend class VulkanRenderer;
    friend class VulkanPostProcess;
    friend class VulkanPostProcessingChain;

    uint32_t m_width;
    uint32_t m_height;
    bool m_lastPass = false;
    VkSampleCountFlagBits m_samples;
    VkDevice m_renderDevice;

    // Depth buffer
    VulkanInternalImage * m_depthImage;
    VkImageView * m_depthImageView;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_depthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Normal buffer
    VulkanInternalImage * m_normalImage;
    VkImageView * m_normalImageView;
    VkFormat m_normalFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_normalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Color accumulation buffer
    VulkanInternalImage * m_colorImage;
    VkImageView * m_colorImageView;
    VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_colorLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Specular accumulation buffer
    VulkanInternalImage * m_specularImage;
    VkImageView * m_specularImageView;
    VkFormat m_specularFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_specularLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Attachments
    std::vector<VkAttachmentDescription> m_attachments;

    VkFramebuffer m_framebuffer;

    // Renderpass
    VkRenderPass * m_renderPass;
};
}
