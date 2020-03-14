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

#include "imstkVulkanFramebuffer.h"

namespace imstk
{
VulkanFramebuffer::VulkanFramebuffer(
    VulkanMemoryManager&  memoryManager,
    uint32_t              width,
    uint32_t              height,
    VkSampleCountFlagBits samples)
{
    m_renderDevice = memoryManager.m_device;
    m_width   = width;
    m_height  = height;
    m_samples = samples;
}

void
VulkanFramebuffer::initializeFramebuffer(VkRenderPass* renderPass)
{
    m_renderPass = renderPass;
    std::vector<VkImageView> framebufferAttachments;

    // Color attachment
    if (m_colorFormat != VK_FORMAT_UNDEFINED)
    {
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_colorFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout    = m_colorLayout;
        m_attachments.push_back(attachment);
        framebufferAttachments.push_back(*m_colorImageView);
    }

    // Depth attachment
    if (m_depthFormat != VK_FORMAT_UNDEFINED)
    {
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_depthFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout    = m_depthLayout;
        m_attachments.push_back(attachment);
        framebufferAttachments.push_back(*m_depthImageView);
    }

    // Normal attachment
    if (m_normalFormat != VK_FORMAT_UNDEFINED)
    {
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_normalFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout    = m_normalLayout;
        m_attachments.push_back(attachment);
        framebufferAttachments.push_back(*m_normalImageView);
    }

    // Specular attachment
    if (m_specularFormat != VK_FORMAT_UNDEFINED)
    {
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_specularFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout    = m_specularLayout;
        m_attachments.push_back(attachment);
        framebufferAttachments.push_back(*m_specularImageView);
    }

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext           = nullptr;
    framebufferInfo.flags           = 0;
    framebufferInfo.renderPass      = *m_renderPass;
    framebufferInfo.attachmentCount = (uint32_t)framebufferAttachments.size();
    framebufferInfo.pAttachments    = &framebufferAttachments[0];
    framebufferInfo.height          = m_height;
    framebufferInfo.width           = m_width;
    framebufferInfo.layers          = 1;

    vkCreateFramebuffer(m_renderDevice, &framebufferInfo, nullptr, &m_framebuffer);
}

void
VulkanFramebuffer::setColor(VulkanInternalImage* image,
                            VkImageView*         imageView,
                            VkFormat             format,
                            VkImageLayout        layout)
{
    m_colorImage     = image;
    m_colorImageView = imageView;
    m_colorFormat    = format;
    m_colorLayout    = layout;
}

void
VulkanFramebuffer::setDepth(VulkanInternalImage* image,
                            VkImageView*         imageView,
                            VkFormat             format,
                            VkImageLayout        layout)
{
    m_depthImage     = image;
    m_depthImageView = imageView;
    m_depthFormat    = format;
    m_depthLayout    = layout;
}

void
VulkanFramebuffer::setNormal(VulkanInternalImage* image,
                             VkImageView*         imageView,
                             VkFormat             format,
                             VkImageLayout        layout)
{
    m_normalImage     = image;
    m_normalImageView = imageView;
    m_normalFormat    = format;
    m_normalLayout    = layout;
}

void
VulkanFramebuffer::setSpecular(VulkanInternalImage* image,
                               VkImageView*         imageView,
                               VkFormat             format,
                               VkImageLayout        layout)
{
    m_specularImage     = image;
    m_specularImageView = imageView;
    m_specularFormat    = format;
    m_specularLayout    = layout;
}

void
VulkanFramebuffer::clear(VkDevice* device)
{
    vkDestroyFramebuffer(*device, m_framebuffer, nullptr);
}
}