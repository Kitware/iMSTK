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

#include "imstkVulkanRenderPassGenerator.h"

namespace imstk
{
void
VulkanRenderPassGenerator::generateOpaqueRenderPass(
    VkDevice& device, VkRenderPass& renderPass, VkSampleCountFlagBits& samples)
{
    VkAttachmentDescription attachments[4];

    // Color attachment
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = samples;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth attachment
    attachments[1].flags = 0;
    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].samples = samples;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Normal attachment
    attachments[2].flags = 0;
    attachments[2].format = VK_FORMAT_R8G8B8A8_SNORM;
    attachments[2].samples = samples;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Specular attachment
    attachments[3].flags = 0;
    attachments[3].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[3].samples = samples;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Color attachment
    VkAttachmentReference diffuseReference;
    diffuseReference.attachment = 0;
    diffuseReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Normal attachment
    VkAttachmentReference normalReference;
    normalReference.attachment = 2;
    normalReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Specular attachment
    VkAttachmentReference specularReference;
    specularReference.attachment = 3;
    specularReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Render subpasses
    VkSubpassDescription subpassInfo[2];

    // First pass: geometry
    VkAttachmentReference colorAttachments[] = { diffuseReference, normalReference, specularReference };
    subpassInfo[0].flags = 0;
    subpassInfo[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassInfo[0].inputAttachmentCount = 0;
    subpassInfo[0].pInputAttachments = nullptr;
    subpassInfo[0].colorAttachmentCount = 3;
    subpassInfo[0].pColorAttachments = colorAttachments;
    subpassInfo[0].pResolveAttachments = nullptr;
    subpassInfo[0].pDepthStencilAttachment = &depthReference;
    subpassInfo[0].preserveAttachmentCount = 0;
    subpassInfo[0].pPreserveAttachments = nullptr;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo[2];
    renderPassInfo[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo[0].pNext = nullptr;
    renderPassInfo[0].flags = 0;
    renderPassInfo[0].attachmentCount = 4;
    renderPassInfo[0].pAttachments = attachments;
    renderPassInfo[0].subpassCount = 1;
    renderPassInfo[0].pSubpasses = &subpassInfo[0];
    renderPassInfo[0].dependencyCount = 2;
    renderPassInfo[0].pDependencies = dependencies;

    vkCreateRenderPass(device, &renderPassInfo[0], nullptr, &renderPass);
}

void
VulkanRenderPassGenerator::generateDecalRenderPass(
    VkDevice& device, VkRenderPass& renderPass, VkSampleCountFlagBits& samples)
{
    VkAttachmentDescription attachments[3];

    // Color attachment
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[0].samples = samples;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth attachment
    attachments[1].flags = 0;
    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].samples = samples;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // Specular attachment
    attachments[2].flags = 0;
    attachments[2].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachments[2].samples = samples;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Color attachment
    VkAttachmentReference diffuseReference;
    diffuseReference.attachment = 0;
    diffuseReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // Specular attachment
    VkAttachmentReference specularReference;
    specularReference.attachment = 2;
    specularReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Render subpasses
    VkSubpassDescription subpassInfo[2];

    // First pass: geometry
    VkAttachmentReference colorAttachments[] = { diffuseReference, specularReference };
    subpassInfo[0].flags = 0;
    subpassInfo[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassInfo[0].inputAttachmentCount = 0;
    subpassInfo[0].pInputAttachments = nullptr;
    subpassInfo[0].colorAttachmentCount = 2;
    subpassInfo[0].pColorAttachments = colorAttachments;
    subpassInfo[0].pResolveAttachments = nullptr;
    subpassInfo[0].pDepthStencilAttachment = &depthReference;
    subpassInfo[0].preserveAttachmentCount = 0;
    subpassInfo[0].pPreserveAttachments = nullptr;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo[2];
    renderPassInfo[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo[0].pNext = nullptr;
    renderPassInfo[0].flags = 0;
    renderPassInfo[0].attachmentCount = 3;
    renderPassInfo[0].pAttachments = attachments;
    renderPassInfo[0].subpassCount = 1;
    renderPassInfo[0].pSubpasses = &subpassInfo[0];
    renderPassInfo[0].dependencyCount = 2;
    renderPassInfo[0].pDependencies = dependencies;

    vkCreateRenderPass(device, &renderPassInfo[0], nullptr, &renderPass);
}

void
VulkanRenderPassGenerator::generateShadowRenderPass(
    VkDevice& device, VkRenderPass& renderPass, VkSampleCountFlagBits& samples)
{
    VkAttachmentDescription attachments[1];

    // Depth attachment
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_D32_SFLOAT;
    attachments[0].samples = samples;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentReference depthReference;
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Render subpasses
    VkSubpassDescription subpassInfo[2];

    // First pass: geometry
    subpassInfo[0].flags = 0;
    subpassInfo[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassInfo[0].inputAttachmentCount = 0;
    subpassInfo[0].pInputAttachments = nullptr;
    subpassInfo[0].colorAttachmentCount = 0;
    subpassInfo[0].pColorAttachments = nullptr;
    subpassInfo[0].pResolveAttachments = nullptr;
    subpassInfo[0].pDepthStencilAttachment = &depthReference;
    subpassInfo[0].preserveAttachmentCount = 0;
    subpassInfo[0].pPreserveAttachments = nullptr;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo[2];
    renderPassInfo[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo[0].pNext = nullptr;
    renderPassInfo[0].flags = 0;
    renderPassInfo[0].attachmentCount = 1;
    renderPassInfo[0].pAttachments = attachments;
    renderPassInfo[0].subpassCount = 1;
    renderPassInfo[0].pSubpasses = &subpassInfo[0];
    renderPassInfo[0].dependencyCount = 2;
    renderPassInfo[0].pDependencies = dependencies;

    vkCreateRenderPass(device, &renderPassInfo[0], nullptr, &renderPass);
}
}