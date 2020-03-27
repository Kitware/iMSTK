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

#include "imstkVulkanPostProcess.h"

#include "imstkVulkanRenderer.h"

namespace imstk
{
VulkanPostProcess::VulkanPostProcess(VulkanRenderer* renderer, uint32_t numViews, unsigned int level)
{
    m_downsampleLevels = level;
    m_numViews = numViews;
    auto width  = renderer->m_width >> level;
    auto height = renderer->m_height >> level;
    this->createFramebuffer(renderer, width, height);
}

VulkanPostProcess::VulkanPostProcess(VulkanRenderer* renderer, uint32_t numViews, unsigned int width, unsigned int height)
{
    m_downsampleLevels = 0;
    m_numViews = numViews;
    this->createFramebuffer(renderer, width, height);
}

void
VulkanPostProcess::initialize(VulkanRenderer* renderer, std::string fragmentShaderPath)
{
    this->initializeFramebuffer(renderer);
    this->createDescriptorSetLayouts(renderer);
    this->createFullscreenQuad(renderer);
    this->createPipeline(renderer, fragmentShaderPath);
    this->createDescriptors(renderer);
}

void
VulkanPostProcess::createPipeline(VulkanRenderer* renderer, std::string fragmentSource)
{
    // The vertex shader should be the same for every postprocess
    std::string   vertexShaderPath = VulkanShaderPath::PostProcessing + "postprocess_vert.spv";
    std::ifstream vertexShaderDataStream(vertexShaderPath, std::ios_base::binary);
    char          vertexShaderData[16000];
    vertexShaderDataStream.read(vertexShaderData, 15999);
    vertexShaderData[(int)vertexShaderDataStream.gcount()] = '\0';

    VkShaderModuleCreateInfo vertexShaderInfo;
    vertexShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderInfo.pNext    = nullptr;
    vertexShaderInfo.flags    = 0;
    vertexShaderInfo.codeSize = vertexShaderDataStream.gcount();
    vertexShaderInfo.pCode    = (uint32_t*)vertexShaderData;
    if (vkCreateShaderModule(renderer->m_renderDevice, &vertexShaderInfo, nullptr, &m_pipelineComponents.vertexShader) != VK_SUCCESS)
    {
        LOG(FATAL) << "Unable to build vertex shader : " << vertexShaderPath;
    }

    std::string   fragmentShaderPath = fragmentSource;
    std::ifstream fragmentShaderDataStream(fragmentShaderPath, std::ios_base::binary);
    char          fragmentShaderData[16000];
    fragmentShaderDataStream.read(fragmentShaderData, 15999);
    fragmentShaderData[(int)fragmentShaderDataStream.gcount()] = '\0';

    VkShaderModuleCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderInfo.pNext    = nullptr;
    fragmentShaderInfo.flags    = 0;
    fragmentShaderInfo.codeSize = fragmentShaderDataStream.gcount();
    fragmentShaderInfo.pCode    = (uint32_t*)fragmentShaderData;
    if (vkCreateShaderModule(renderer->m_renderDevice, &fragmentShaderInfo, nullptr, &m_pipelineComponents.fragmentShader) != VK_SUCCESS)
    {
        LOG(FATAL) << "Unable to build fragment shader: " << fragmentShaderPath;
    }

    m_pipelineComponents.shaderInfo.resize(2);

    m_pipelineComponents.fragmentSpecializationInfo.mapEntryCount = 0;
    m_pipelineComponents.fragmentSpecializationInfo.pMapEntries   = nullptr;
    m_pipelineComponents.fragmentSpecializationInfo.dataSize      = 0;
    m_pipelineComponents.fragmentSpecializationInfo.pData = nullptr;

    // Vertex Shader
    m_pipelineComponents.shaderInfo[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_pipelineComponents.shaderInfo[0].pNext  = nullptr;
    m_pipelineComponents.shaderInfo[0].flags  = 0;
    m_pipelineComponents.shaderInfo[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    m_pipelineComponents.shaderInfo[0].module = m_pipelineComponents.vertexShader;
    m_pipelineComponents.shaderInfo[0].pName  = "main";
    m_pipelineComponents.shaderInfo[0].pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;

    // Fragment Shader
    m_pipelineComponents.shaderInfo[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_pipelineComponents.shaderInfo[1].pNext  = nullptr;
    m_pipelineComponents.shaderInfo[1].flags  = 0;
    m_pipelineComponents.shaderInfo[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    m_pipelineComponents.shaderInfo[1].module = m_pipelineComponents.fragmentShader;
    m_pipelineComponents.shaderInfo[1].pName  = "main";
    m_pipelineComponents.shaderInfo[1].pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;

    // Vertex Attributes
    m_pipelineComponents.vertexBindingDescription.resize(1);

    m_pipelineComponents.vertexBindingDescription[0].binding   = 0;
    m_pipelineComponents.vertexBindingDescription[0].stride    = 4 * 5;
    m_pipelineComponents.vertexBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Vertex Attributes
    m_pipelineComponents.vertexAttributeDescription.resize(2);

    m_pipelineComponents.vertexAttributeDescription[0].location = 0;
    m_pipelineComponents.vertexAttributeDescription[0].binding  = 0;
    m_pipelineComponents.vertexAttributeDescription[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[0].offset   = 0;

    m_pipelineComponents.vertexAttributeDescription[1].location = 1;
    m_pipelineComponents.vertexAttributeDescription[1].binding  = 0;
    m_pipelineComponents.vertexAttributeDescription[1].format   = VK_FORMAT_R32G32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[1].offset   = 4 * 3;

    // Pipeline stages
    m_pipelineComponents.vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_pipelineComponents.vertexInfo.pNext = nullptr;
    m_pipelineComponents.vertexInfo.flags = 0;
    m_pipelineComponents.vertexInfo.vertexBindingDescriptionCount   = (uint32_t)m_pipelineComponents.vertexBindingDescription.size();
    m_pipelineComponents.vertexInfo.pVertexBindingDescriptions      = &m_pipelineComponents.vertexBindingDescription[0];
    m_pipelineComponents.vertexInfo.vertexAttributeDescriptionCount = (uint32_t)m_pipelineComponents.vertexAttributeDescription.size();
    m_pipelineComponents.vertexInfo.pVertexAttributeDescriptions    = &m_pipelineComponents.vertexAttributeDescription[0];

    m_pipelineComponents.inputAssemblyInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_pipelineComponents.inputAssemblyInfo.pNext    = nullptr;
    m_pipelineComponents.inputAssemblyInfo.flags    = 0;
    m_pipelineComponents.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_pipelineComponents.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    m_pipelineComponents.tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    m_pipelineComponents.tessellationInfo.pNext = nullptr;
    m_pipelineComponents.tessellationInfo.flags = 0;
    m_pipelineComponents.tessellationInfo.patchControlPoints = 1; // For now, this should be changed in the future

    m_pipelineComponents.viewports.resize(1);

    m_pipelineComponents.viewports[0].x        = 0;
    m_pipelineComponents.viewports[0].y        = 0;
    m_pipelineComponents.viewports[0].height   = m_framebuffer->m_height;
    m_pipelineComponents.viewports[0].width    = m_framebuffer->m_width;
    m_pipelineComponents.viewports[0].minDepth = 0.0;
    m_pipelineComponents.viewports[0].maxDepth = 1.0;

    m_pipelineComponents.scissors.resize(1);

    m_pipelineComponents.scissors[0].offset = { 0, 0 };
    m_pipelineComponents.scissors[0].extent = { m_framebuffer->m_width, m_framebuffer->m_height };

    m_pipelineComponents.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_pipelineComponents.viewportInfo.pNext = nullptr;
    m_pipelineComponents.viewportInfo.flags = 0;
    m_pipelineComponents.viewportInfo.viewportCount = (uint32_t)m_pipelineComponents.viewports.size();
    m_pipelineComponents.viewportInfo.pViewports    = &m_pipelineComponents.viewports[0];
    m_pipelineComponents.viewportInfo.scissorCount  = (uint32_t)m_pipelineComponents.scissors.size();
    m_pipelineComponents.viewportInfo.pScissors     = &m_pipelineComponents.scissors[0];

    m_pipelineComponents.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_pipelineComponents.rasterizationInfo.pNext = nullptr;
    m_pipelineComponents.rasterizationInfo.flags = 0;
    m_pipelineComponents.rasterizationInfo.depthClampEnable = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.polygonMode     = VK_POLYGON_MODE_FILL;
    m_pipelineComponents.rasterizationInfo.cullMode        = VK_CULL_MODE_NONE;
    m_pipelineComponents.rasterizationInfo.frontFace       = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_pipelineComponents.rasterizationInfo.depthBiasEnable = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.depthBiasConstantFactor = 0.0;
    m_pipelineComponents.rasterizationInfo.depthBiasClamp       = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.depthBiasSlopeFactor = 0.0;
    m_pipelineComponents.rasterizationInfo.lineWidth = 1.0;

    m_pipelineComponents.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_pipelineComponents.multisampleInfo.pNext = nullptr;
    m_pipelineComponents.multisampleInfo.flags = 0;
    m_pipelineComponents.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // TODO: Enable multisampling
    m_pipelineComponents.multisampleInfo.sampleShadingEnable  = VK_FALSE;
    m_pipelineComponents.multisampleInfo.minSampleShading     = 0;
    m_pipelineComponents.multisampleInfo.pSampleMask = nullptr;
    m_pipelineComponents.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    m_pipelineComponents.multisampleInfo.alphaToOneEnable      = VK_FALSE;

    VkStencilOpState states[2];
    states[0].failOp      = VK_STENCIL_OP_ZERO;
    states[0].passOp      = VK_STENCIL_OP_KEEP;
    states[0].depthFailOp = VK_STENCIL_OP_ZERO;
    states[0].compareOp   = VK_COMPARE_OP_LESS;
    states[0].compareMask = 0;
    states[0].writeMask   = 0;
    states[0].reference   = 0;

    states[1].failOp      = VK_STENCIL_OP_ZERO;
    states[1].passOp      = VK_STENCIL_OP_KEEP;
    states[1].depthFailOp = VK_STENCIL_OP_ZERO;
    states[1].compareOp   = VK_COMPARE_OP_LESS;
    states[1].compareMask = 0;
    states[1].writeMask   = 0;
    states[1].reference   = 0;

    m_pipelineComponents.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_pipelineComponents.depthStencilInfo.pNext = nullptr;
    m_pipelineComponents.depthStencilInfo.flags = 0;
    m_pipelineComponents.depthStencilInfo.depthTestEnable       = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.depthWriteEnable      = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
    m_pipelineComponents.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.stencilTestEnable     = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.front = states[0];
    m_pipelineComponents.depthStencilInfo.back  = states[1];
    m_pipelineComponents.depthStencilInfo.minDepthBounds = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.maxDepthBounds = VK_FALSE;

    m_pipelineComponents.colorBlendAttachments.resize(m_colorAttachments.size());
    for (int i = 0; i < m_pipelineComponents.colorBlendAttachments.size(); i++)
    {
        m_pipelineComponents.colorBlendAttachments[i].blendEnable = VK_TRUE;
        m_pipelineComponents.colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        m_pipelineComponents.colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_pipelineComponents.colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
        m_pipelineComponents.colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        m_pipelineComponents.colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_pipelineComponents.colorBlendAttachments[i].alphaBlendOp   = VK_BLEND_OP_ADD;
        m_pipelineComponents.colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                                       VK_COLOR_COMPONENT_G_BIT |
                                                                       VK_COLOR_COMPONENT_B_BIT |
                                                                       VK_COLOR_COMPONENT_A_BIT;
    }

    m_pipelineComponents.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_pipelineComponents.colorBlendInfo.pNext = nullptr;
    m_pipelineComponents.colorBlendInfo.flags = 0;
    m_pipelineComponents.colorBlendInfo.logicOpEnable     = VK_FALSE;
    m_pipelineComponents.colorBlendInfo.logicOp           = VK_LOGIC_OP_SET;
    m_pipelineComponents.colorBlendInfo.attachmentCount   = (uint32_t)m_pipelineComponents.colorBlendAttachments.size();
    m_pipelineComponents.colorBlendInfo.pAttachments      = &m_pipelineComponents.colorBlendAttachments[0];
    m_pipelineComponents.colorBlendInfo.blendConstants[0] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[1] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[2] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[3] = 1.0;

    VkPushConstantRange pushConstants;
    pushConstants.offset     = 0;
    pushConstants.size       = 128; // Minimum on all devices
    pushConstants.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.setLayoutCount = (uint32_t)m_descriptorSetLayouts.size();
    layoutInfo.pSetLayouts    = &m_descriptorSetLayouts[0];
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges    = &pushConstants;

    vkCreatePipelineLayout(renderer->m_renderDevice, &layoutInfo, nullptr, &m_pipelineLayout);

    m_pipelineComponents.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    m_pipelineComponents.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

    m_pipelineComponents.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_pipelineComponents.dynamicStateInfo.pNext = nullptr;
    m_pipelineComponents.dynamicStateInfo.flags = 0;
    m_pipelineComponents.dynamicStateInfo.dynamicStateCount = (uint32_t)m_pipelineComponents.dynamicStates.size();
    m_pipelineComponents.dynamicStateInfo.pDynamicStates    = &m_pipelineComponents.dynamicStates[0];

    m_graphicsPipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_graphicsPipelineInfo.pNext      = nullptr;
    m_graphicsPipelineInfo.flags      = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    m_graphicsPipelineInfo.stageCount = (uint32_t)m_pipelineComponents.shaderInfo.size();
    m_graphicsPipelineInfo.pStages    = &m_pipelineComponents.shaderInfo[0];
    m_graphicsPipelineInfo.pVertexInputState   = &m_pipelineComponents.vertexInfo;
    m_graphicsPipelineInfo.pInputAssemblyState = &m_pipelineComponents.inputAssemblyInfo;
    m_graphicsPipelineInfo.pTessellationState  = &m_pipelineComponents.tessellationInfo;
    m_graphicsPipelineInfo.pViewportState      = &m_pipelineComponents.viewportInfo;
    m_graphicsPipelineInfo.pRasterizationState = &m_pipelineComponents.rasterizationInfo;
    m_graphicsPipelineInfo.pMultisampleState   = &m_pipelineComponents.multisampleInfo;
    m_graphicsPipelineInfo.pDepthStencilState  = &m_pipelineComponents.depthStencilInfo;
    m_graphicsPipelineInfo.pColorBlendState    = &m_pipelineComponents.colorBlendInfo;
    m_graphicsPipelineInfo.pDynamicState       = &m_pipelineComponents.dynamicStateInfo;
    m_graphicsPipelineInfo.layout     = m_pipelineLayout;
    m_graphicsPipelineInfo.renderPass = m_renderPass;
    m_graphicsPipelineInfo.subpass    = 0;
    m_graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    m_graphicsPipelineInfo.basePipelineIndex  = 0;
}

void
VulkanPostProcess::createFullscreenQuad(VulkanRenderer* renderer)
{
    m_vertexBuffer = std::make_shared<VulkanVertexBuffer>
                     (renderer->m_memoryManager, 4, (unsigned int)(sizeof(float) * 5), 2);

    {
        auto data = (float*)m_vertexBuffer->getVertexMemory();
        data[0] = -1;
        data[1] = -1;
        data[2] = 0;
        data[3] = 0;
        data[4] = 0;

        data[5] = -1;
        data[6] = 1;
        data[7] = 0;
        data[8] = 0;
        data[9] = 1;

        data[10] = 1;
        data[11] = -1;
        data[12] = 0;
        data[13] = 1;
        data[14] = 0;

        data[15] = 1;
        data[16] = 1;
        data[17] = 0;
        data[18] = 1;
        data[19] = 1;
    }

    {
        auto data = (uint32_t*)m_vertexBuffer->getIndexMemory();
        data[0] = 0;
        data[1] = 1;
        data[2] = 2;
        data[3] = 1;
        data[4] = 2;
        data[5] = 3;
    }

    m_vertexBuffer->initializeBuffers(renderer->m_memoryManager);
}

void
VulkanPostProcess::createDescriptors(VulkanRenderer* renderer)
{
    this->createDescriptorPool(renderer);
    this->createDescriptorSets(renderer);
}

void
VulkanPostProcess::createDescriptorSetLayouts(VulkanRenderer* renderer)
{
    m_descriptorSets.resize(1);
    m_descriptorSetLayouts.resize(1);

    std::vector<VkDescriptorSetLayoutBinding> fragmentDescriptorSetLayoutBindings;

    // Diffuse texture
    for (int i = 0; i < m_samplers.size(); i++)
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = i;
        fragmentLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount    = 1;
        fragmentLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo[1];
    descriptorSetLayoutInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo[0].pNext = nullptr;
    descriptorSetLayoutInfo[0].flags = 0;
    descriptorSetLayoutInfo[0].bindingCount = (uint32_t)fragmentDescriptorSetLayoutBindings.size();
    descriptorSetLayoutInfo[0].pBindings    = &fragmentDescriptorSetLayoutBindings[0];

    for (int i = 0; i < m_descriptorSetLayouts.size(); i++)
    {
        vkCreateDescriptorSetLayout(renderer->m_renderDevice, &descriptorSetLayoutInfo[i], nullptr, &m_descriptorSetLayouts[i]);
    }
}

void
VulkanPostProcess::createDescriptorPool(VulkanRenderer* renderer)
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

    // Fragment shader textures
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = (uint32_t)m_samplers.size();
    descriptorPoolSizes.push_back(poolSize);

    VkDescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext         = nullptr;
    descriptorPoolInfo.flags         = 0;
    descriptorPoolInfo.maxSets       = (uint32_t)m_descriptorSets.size();
    descriptorPoolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
    descriptorPoolInfo.pPoolSizes    = &descriptorPoolSizes[0];

    vkCreateDescriptorPool(renderer->m_renderDevice, &descriptorPoolInfo, nullptr, &m_descriptorPool);
}

void
VulkanPostProcess::createDescriptorSets(VulkanRenderer* renderer)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocationInfo[1];
    descriptorSetAllocationInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocationInfo[0].pNext = nullptr;
    descriptorSetAllocationInfo[0].descriptorPool     = m_descriptorPool;
    descriptorSetAllocationInfo[0].descriptorSetCount = (uint32_t)m_descriptorSetLayouts.size();
    descriptorSetAllocationInfo[0].pSetLayouts = &m_descriptorSetLayouts[0];

    //VkDeviceSize size = { VK_WHOLE_SIZE };

    std::vector<VkDescriptorImageInfo> fragmentTextureInfo;

    // Textures
    for (int i = 0; i < m_samplers.size(); i++)
    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler     = *m_samplers[i];
        textureInfo.imageView   = *m_imageViews[i];
        textureInfo.imageLayout = m_layouts[i];
        fragmentTextureInfo.push_back(textureInfo);
    }

    vkAllocateDescriptorSets(renderer->m_renderDevice, descriptorSetAllocationInfo, &m_descriptorSets[0]);

    m_writeDescriptorSets.resize(1);

    m_writeDescriptorSets[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    m_writeDescriptorSets[0].pNext            = nullptr;
    m_writeDescriptorSets[0].dstBinding       = 0;
    m_writeDescriptorSets[0].dstArrayElement  = 0;
    m_writeDescriptorSets[0].dstSet           = m_descriptorSets[0];
    m_writeDescriptorSets[0].descriptorCount  = (uint32_t)fragmentTextureInfo.size();
    m_writeDescriptorSets[0].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    m_writeDescriptorSets[0].pBufferInfo      = nullptr;
    m_writeDescriptorSets[0].pImageInfo       = &fragmentTextureInfo[0];
    m_writeDescriptorSets[0].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(renderer->m_renderDevice, (uint32_t)m_writeDescriptorSets.size(), &m_writeDescriptorSets[0], 0, nullptr);
}

void
VulkanPostProcess::createRenderPass(VulkanRenderer* renderer)
{
    std::vector<VkAttachmentDescription> attachments;
    bool                                 depth = false;

    // Color attachment
    {
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_framebuffer->m_colorFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout    = m_framebuffer->m_colorLayout;
        attachments.push_back(attachment);
    }

    if (m_framebuffer->m_depthFormat != VK_FORMAT_UNDEFINED)
    {
        depth = true;

        // Depth attachment
        VkAttachmentDescription attachment;
        attachment.flags          = 0;
        attachment.format         = m_framebuffer->m_depthFormat;
        attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        attachment.finalLayout    = m_framebuffer->m_depthLayout;
        attachments.push_back(attachment);
    }

    // Color attachment
    VkAttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout     = m_framebuffer->m_colorLayout;

    // Depth attachment
    VkAttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout     = m_framebuffer->m_depthLayout;

    // Normal attachment
    VkAttachmentReference normalReference;
    normalReference.attachment = 2;
    normalReference.layout     = m_framebuffer->m_normalLayout;

    // First pass: geometry
    m_colorAttachments.push_back(colorReference);

    // Render subpasses
    VkSubpassDescription subpassInfo[1];
    subpassInfo[0].flags = 0;
    subpassInfo[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassInfo[0].inputAttachmentCount    = 0;
    subpassInfo[0].pInputAttachments       = nullptr;
    subpassInfo[0].colorAttachmentCount    = (uint32_t)m_colorAttachments.size();
    subpassInfo[0].pColorAttachments       = &m_colorAttachments[0];
    subpassInfo[0].pResolveAttachments     = nullptr;
    subpassInfo[0].pDepthStencilAttachment = depth ? &depthReference : nullptr;
    subpassInfo[0].preserveAttachmentCount = 0;
    subpassInfo[0].pPreserveAttachments    = nullptr;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    uint32_t viewMask = m_numViews == 2 ? 3 : 1;
    uint32_t correlationMask = m_numViews == 2 ? 3 : 1;

    VkRenderPassMultiviewCreateInfo multiviewInfo;
    VulkanRenderPassGenerator::generateRenderPassMultiviewCreateInfo(multiviewInfo, viewMask, correlationMask);

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = &multiviewInfo;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments    = &attachments[0];
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = subpassInfo;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies   = dependencies;

    vkCreateRenderPass(renderer->m_renderDevice, &renderPassInfo, nullptr, &m_renderPass);
}

void
VulkanPostProcess::initializeFramebuffer(VulkanRenderer* renderer)
{
    this->createRenderPass(renderer);
    m_framebuffer->initializeFramebuffer(&m_renderPass);
}

void
VulkanPostProcess::createFramebuffer(VulkanRenderer*    renderer,
                                     const unsigned int width,
                                     const unsigned int height)
{
    m_framebuffer = std::make_shared<VulkanFramebuffer>(
        renderer->m_memoryManager,
        width,
        height);
}

void
VulkanPostProcess::addInputImage(
    VkSampler*    sampler,
    VkImageView*  imageView,
    VkImageLayout layout)
{
    m_samplers.push_back(sampler);
    m_imageViews.push_back(imageView);
    m_layouts.push_back(layout);
}

void
VulkanPostProcess::updateImageLayouts()
{
    if (m_framebuffer->m_colorFormat != VK_FORMAT_UNDEFINED)
    {
        m_framebuffer->m_colorImage->setImageLayout(m_framebuffer->m_colorLayout);
    }

    if (m_framebuffer->m_depthFormat != VK_FORMAT_UNDEFINED)
    {
        m_framebuffer->m_depthImage->setImageLayout(m_framebuffer->m_depthLayout);
    }

    if (m_framebuffer->m_normalFormat != VK_FORMAT_UNDEFINED)
    {
        m_framebuffer->m_normalImage->setImageLayout(m_framebuffer->m_normalLayout);
    }

    if (m_framebuffer->m_specularFormat != VK_FORMAT_UNDEFINED)
    {
        m_framebuffer->m_specularImage->setImageLayout(m_framebuffer->m_specularLayout);
    }
}

void
VulkanPostProcess::setAttachmentsToReadLayout(VkCommandBuffer* commandBuffer,
                                              uint32_t         queueFamily,
                                              const uint32_t   numViews)
{
    if (m_framebuffer->m_colorFormat != VK_FORMAT_UNDEFINED)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer,
            queueFamily,
            m_framebuffer->m_colorImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            numViews);
    }

    if (m_framebuffer->m_depthFormat != VK_FORMAT_UNDEFINED)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer,
            queueFamily,
            m_framebuffer->m_depthImage,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            numViews);
    }

    if (m_framebuffer->m_normalFormat != VK_FORMAT_UNDEFINED)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer,
            queueFamily,
            m_framebuffer->m_normalImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            numViews);
    }

    if (m_framebuffer->m_specularFormat != VK_FORMAT_UNDEFINED)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer,
            queueFamily,
            m_framebuffer->m_specularImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            numViews);
    }
}

void
VulkanPostProcess::clear(VkDevice* device)
{
    vkDestroyShaderModule(*device, m_pipelineComponents.vertexShader, nullptr);
    vkDestroyShaderModule(*device, m_pipelineComponents.fragmentShader, nullptr);
    vkDestroyPipelineLayout(*device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(*device, m_pipeline, nullptr);
    vkDestroyRenderPass(*device, m_renderPass, nullptr);

    for (auto layout : m_descriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(*device, layout, nullptr);
    }

    vkDestroyDescriptorPool(*device, m_descriptorPool, nullptr);
}
}