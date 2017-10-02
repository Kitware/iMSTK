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

#include "imstkVulkanMaterialDelegate.h"

#include "imstkVulkanRenderer.h"

namespace imstk
{
VulkanMaterialDelegate::VulkanMaterialDelegate(
    std::shared_ptr<VulkanUniformBuffer> vertexUniformBuffer,
    std::shared_ptr<VulkanUniformBuffer> fragmentUniformBuffer,
    std::shared_ptr<RenderMaterial> material,
    VulkanMemoryManager& memoryManager)
{
    m_vertexUniformBuffer = vertexUniformBuffer;
    m_fragmentUniformBuffer = fragmentUniformBuffer;

    m_memoryManager = &memoryManager;

    if (material)
    {
        m_material = material;
    }
    else
    {
        m_material = std::make_shared<RenderMaterial>();
    }
}

void
VulkanMaterialDelegate::initialize(VulkanRenderer * renderer)
{
    this->createDescriptorSetLayouts(renderer);
    this->createPipeline(renderer);
    this->initializeTextures(renderer);
    this->createDescriptors(renderer);
}

void
VulkanMaterialDelegate::createPipeline(VulkanRenderer * renderer)
{
    m_memoryManager->m_device = renderer->m_renderDevice;
    m_memoryManager->m_queueFamilyIndex = renderer->m_renderQueueFamily;

    std::string vertexShaderPath = "./Shaders/VulkanShaders/Mesh/mesh_vert.spv";
    std::ifstream vertexShaderDataStream(vertexShaderPath, std::ios_base::binary);
    char vertexShaderData[16000];
    vertexShaderDataStream.read(vertexShaderData, 15999);
    vertexShaderData[(int)vertexShaderDataStream.gcount()] = '\0';

    VkShaderModuleCreateInfo vertexShaderInfo;
    vertexShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderInfo.pNext = nullptr;
    vertexShaderInfo.flags = 0;
    vertexShaderInfo.codeSize = vertexShaderDataStream.gcount();
    vertexShaderInfo.pCode = (uint32_t *)vertexShaderData;
    if (vkCreateShaderModule(renderer->m_renderDevice, &vertexShaderInfo, nullptr, &m_pipelineComponents.vertexShader) != VK_SUCCESS)
    {
        LOG(FATAL) << "Unable to build vertex shader : " << vertexShaderPath;
    }

    if (m_material->getTessellated())
    {
        std::string tessellationControlShaderPath = "./Shaders/VulkanShaders/Mesh/mesh_tesc.spv";
        std::ifstream tessellationControlShaderDataStream(tessellationControlShaderPath, std::ios_base::binary);
        char tessellationControlShaderData[16000];
        tessellationControlShaderDataStream.read(tessellationControlShaderData, 15999);
        tessellationControlShaderData[(int)tessellationControlShaderDataStream.gcount()] = '\0';

        VkShaderModuleCreateInfo tessellationControlShaderInfo;
        tessellationControlShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        tessellationControlShaderInfo.pNext = nullptr;
        tessellationControlShaderInfo.flags = 0;
        tessellationControlShaderInfo.codeSize = tessellationControlShaderDataStream.gcount();
        tessellationControlShaderInfo.pCode = (uint32_t *)tessellationControlShaderData;
        if (vkCreateShaderModule(renderer->m_renderDevice, &tessellationControlShaderInfo, nullptr, &m_pipelineComponents.tessellationControlShader) != VK_SUCCESS)
        {
            LOG(FATAL) << "Unable to build fragment shader: " << tessellationControlShaderPath;
        }

        std::string tessellatedEvaluationShaderPath = "./Shaders/VulkanShaders/Mesh/mesh_tese.spv";
        std::ifstream tessellationEvaluationShaderDataStream(tessellatedEvaluationShaderPath, std::ios_base::binary);
        char tessellationEvaluationShaderData[16000];
        tessellationEvaluationShaderDataStream.read(tessellationEvaluationShaderData, 15999);
        tessellationEvaluationShaderData[(int)tessellationEvaluationShaderDataStream.gcount()] = '\0';

        VkShaderModuleCreateInfo tessellationEvaluationShaderInfo;
        tessellationEvaluationShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        tessellationEvaluationShaderInfo.pNext = nullptr;
        tessellationEvaluationShaderInfo.flags = 0;
        tessellationEvaluationShaderInfo.codeSize = tessellationEvaluationShaderDataStream.gcount();
        tessellationEvaluationShaderInfo.pCode = (uint32_t *)tessellationEvaluationShaderData;
        if (vkCreateShaderModule(renderer->m_renderDevice, &tessellationEvaluationShaderInfo, nullptr, &m_pipelineComponents.tessellationEvaluationShader) != VK_SUCCESS)
        {
            LOG(FATAL) << "Unable to build fragment shader: " << tessellatedEvaluationShaderPath;
        }
    }

    std::string fragmentShaderPath = "./Shaders/VulkanShaders/Mesh/mesh_frag.spv";
    std::ifstream fragmentShaderDataStream(fragmentShaderPath, std::ios_base::binary);
    char fragmentShaderData[16000];
    fragmentShaderDataStream.read(fragmentShaderData, 15999);
    fragmentShaderData[(int)fragmentShaderDataStream.gcount()] = '\0';

    VkShaderModuleCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderInfo.pNext = nullptr;
    fragmentShaderInfo.flags = 0;
    fragmentShaderInfo.codeSize = fragmentShaderDataStream.gcount();
    fragmentShaderInfo.pCode = (uint32_t *)fragmentShaderData;
    if (vkCreateShaderModule(renderer->m_renderDevice, &fragmentShaderInfo, nullptr, &m_pipelineComponents.fragmentShader) != VK_SUCCESS)
    {
        LOG(FATAL) << "Unable to build fragment shader: " << fragmentShaderPath;
    }

    // Copy renderer constants to material constants
    renderer->m_constants.numLights = (uint32_t)renderer->m_scene->getLights().size();

    m_constants.numLights = renderer->m_constants.numLights;
    m_constants.tessellation = m_material->getTessellated();
    m_constants.diffuseTexture = (m_material->getTexture(Texture::Type::DIFFUSE)->getPath() != "");
    m_constants.normalTexture = (m_material->getTexture(Texture::Type::NORMAL)->getPath() != "");
    m_constants.specularTexture = (m_material->getTexture(Texture::Type::SPECULAR)->getPath() != "");
    m_constants.roughnessTexture = (m_material->getTexture(Texture::Type::ROUGHNESS)->getPath() != "");
    m_constants.metalnessTexture = (m_material->getTexture(Texture::Type::METALNESS)->getPath() != "");
    m_constants.subsurfaceScatteringTexture = (m_material->getTexture(Texture::Type::SUBSURFACE_SCATTERING)->getPath() != "");
    m_constants.irradianceCubemapTexture = (m_material->getTexture(Texture::Type::IRRADIANCE_CUBEMAP)->getPath() != "");

    this->addSpecializationConstant(sizeof(m_constants.numLights),
        offsetof(VulkanMaterialConstants, numLights));
    this->addSpecializationConstant(sizeof(m_constants.tessellation),
        offsetof(VulkanMaterialConstants, tessellation));
    this->addSpecializationConstant(sizeof(m_constants.diffuseTexture),
        offsetof(VulkanMaterialConstants, diffuseTexture));
    this->addSpecializationConstant(sizeof(m_constants.normalTexture),
        offsetof(VulkanMaterialConstants, normalTexture));
    this->addSpecializationConstant(sizeof(m_constants.specularTexture),
        offsetof(VulkanMaterialConstants, specularTexture));
    this->addSpecializationConstant(sizeof(m_constants.roughnessTexture),
        offsetof(VulkanMaterialConstants, roughnessTexture));
    this->addSpecializationConstant(sizeof(m_constants.metalnessTexture),
        offsetof(VulkanMaterialConstants, metalnessTexture));
    this->addSpecializationConstant(sizeof(m_constants.subsurfaceScatteringTexture),
        offsetof(VulkanMaterialConstants, subsurfaceScatteringTexture));
    this->addSpecializationConstant(sizeof(m_constants.irradianceCubemapTexture),
        offsetof(VulkanMaterialConstants, irradianceCubemapTexture));

    m_pipelineComponents.fragmentSpecializationInfo.mapEntryCount = m_numConstants;
    m_pipelineComponents.fragmentSpecializationInfo.pMapEntries = &m_pipelineComponents.fragmentMapEntries[0];
    m_pipelineComponents.fragmentSpecializationInfo.dataSize = sizeof(m_constants);
    m_pipelineComponents.fragmentSpecializationInfo.pData = (void *)(&m_constants);

    m_pipelineComponents.shaderInfo.clear();

    // Vertex Shader
    {
        VkPipelineShaderStageCreateInfo shaderInfo;
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.pNext = nullptr;
        shaderInfo.flags = 0;
        shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderInfo.module = m_pipelineComponents.vertexShader;
        shaderInfo.pName = "main";
        shaderInfo.pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;
        m_pipelineComponents.shaderInfo.push_back(shaderInfo);
    }

    // Tessellation Shaders
    if (m_material->getTessellated())
    {
        {
            VkPipelineShaderStageCreateInfo shaderInfo;
            shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderInfo.pNext = nullptr;
            shaderInfo.flags = 0;
            shaderInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            shaderInfo.module = m_pipelineComponents.tessellationControlShader;
            shaderInfo.pName = "main";
            shaderInfo.pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;
            m_pipelineComponents.shaderInfo.push_back(shaderInfo);
        }

        {
            VkPipelineShaderStageCreateInfo shaderInfo;
            shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderInfo.pNext = nullptr;
            shaderInfo.flags = 0;
            shaderInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            shaderInfo.module = m_pipelineComponents.tessellationEvaluationShader;
            shaderInfo.pName = "main";
            shaderInfo.pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;
            m_pipelineComponents.shaderInfo.push_back(shaderInfo);
        }
    }

    // Fragment Shader
    {
        VkPipelineShaderStageCreateInfo shaderInfo;
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.pNext = nullptr;
        shaderInfo.flags = 0;
        shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderInfo.module = m_pipelineComponents.fragmentShader;
        shaderInfo.pName = "main";
        shaderInfo.pSpecializationInfo = &m_pipelineComponents.fragmentSpecializationInfo;
        m_pipelineComponents.shaderInfo.push_back(shaderInfo);
    }

    // Vertex Attributes
    m_pipelineComponents.vertexBindingDescription.resize(1);

    m_pipelineComponents.vertexBindingDescription[0].binding = 0;
    m_pipelineComponents.vertexBindingDescription[0].stride = sizeof(VulkanBasicVertex);
    m_pipelineComponents.vertexBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Vertex Attributes
    m_pipelineComponents.vertexAttributeDescription.resize(4);

    m_pipelineComponents.vertexAttributeDescription[0].location = 0;
    m_pipelineComponents.vertexAttributeDescription[0].binding = 0;
    m_pipelineComponents.vertexAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[0].offset = offsetof(VulkanBasicVertex, position);

    m_pipelineComponents.vertexAttributeDescription[1].location = 1;
    m_pipelineComponents.vertexAttributeDescription[1].binding = 0;
    m_pipelineComponents.vertexAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[1].offset = offsetof(VulkanBasicVertex, normal);

    m_pipelineComponents.vertexAttributeDescription[2].location = 2;
    m_pipelineComponents.vertexAttributeDescription[2].binding = 0;
    m_pipelineComponents.vertexAttributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[2].offset = offsetof(VulkanBasicVertex, tangent);

    m_pipelineComponents.vertexAttributeDescription[3].location = 3;
    m_pipelineComponents.vertexAttributeDescription[3].binding = 0;
    m_pipelineComponents.vertexAttributeDescription[3].format = VK_FORMAT_R32G32_SFLOAT;
    m_pipelineComponents.vertexAttributeDescription[3].offset = offsetof(VulkanBasicVertex, uv);

    // Pipeline stages
    m_pipelineComponents.vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_pipelineComponents.vertexInfo.pNext = nullptr;
    m_pipelineComponents.vertexInfo.flags = 0;
    m_pipelineComponents.vertexInfo.vertexBindingDescriptionCount = (uint32_t)m_pipelineComponents.vertexBindingDescription.size();
    m_pipelineComponents.vertexInfo.pVertexBindingDescriptions = &m_pipelineComponents.vertexBindingDescription[0];
    m_pipelineComponents.vertexInfo.vertexAttributeDescriptionCount = (uint32_t)m_pipelineComponents.vertexAttributeDescription.size();
    m_pipelineComponents.vertexInfo.pVertexAttributeDescriptions = &m_pipelineComponents.vertexAttributeDescription[0];

    m_pipelineComponents.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_pipelineComponents.inputAssemblyInfo.pNext = nullptr;
    m_pipelineComponents.inputAssemblyInfo.flags = 0;

    if (m_material->getTessellated())
    {
        m_pipelineComponents.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }
    else
    {
        m_pipelineComponents.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    m_pipelineComponents.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    m_pipelineComponents.tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    m_pipelineComponents.tessellationInfo.pNext = nullptr;
    m_pipelineComponents.tessellationInfo.flags = 0;
    if (m_material->getTessellated())
    {
        m_pipelineComponents.tessellationInfo.patchControlPoints =
            3;//renderer->m_deviceLimits.maxTessellationPatchSize;
    }
    else
    {
        m_pipelineComponents.tessellationInfo.patchControlPoints = 1;
    }

    m_pipelineComponents.viewports.resize(1);

    m_pipelineComponents.viewports[0].x = 0;
    m_pipelineComponents.viewports[0].y = 0;
    m_pipelineComponents.viewports[0].height = renderer->m_height;
    m_pipelineComponents.viewports[0].width = renderer->m_width;
    m_pipelineComponents.viewports[0].minDepth = 0.0;
    m_pipelineComponents.viewports[0].maxDepth = 1.0;

    m_pipelineComponents.scissors.resize(1);

    m_pipelineComponents.scissors[0].offset = { 0, 0 };
    m_pipelineComponents.scissors[0].extent = { renderer->m_width, renderer->m_height };

    m_pipelineComponents.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_pipelineComponents.viewportInfo.pNext = nullptr;
    m_pipelineComponents.viewportInfo.flags = 0;
    m_pipelineComponents.viewportInfo.viewportCount = (uint32_t)m_pipelineComponents.viewports.size();
    m_pipelineComponents.viewportInfo.pViewports = &m_pipelineComponents.viewports[0];
    m_pipelineComponents.viewportInfo.scissorCount = (uint32_t)m_pipelineComponents.scissors.size();
    m_pipelineComponents.viewportInfo.pScissors = &m_pipelineComponents.scissors[0];

    m_pipelineComponents.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_pipelineComponents.rasterizationInfo.pNext = nullptr;
    m_pipelineComponents.rasterizationInfo.flags = 0;
    m_pipelineComponents.rasterizationInfo.depthClampEnable = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; // Might be enabled later

    if (m_material->getDisplayMode() == RenderMaterial::DisplayMode::WIREFRAME)
    {
        m_pipelineComponents.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
    }
    else
    {
        m_pipelineComponents.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    }

    m_pipelineComponents.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // TODO: Allow backface culling
    m_pipelineComponents.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_pipelineComponents.rasterizationInfo.depthBiasEnable = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.depthBiasConstantFactor = 0.0;
    m_pipelineComponents.rasterizationInfo.depthBiasClamp = VK_FALSE;
    m_pipelineComponents.rasterizationInfo.depthBiasSlopeFactor = 0.0;
    m_pipelineComponents.rasterizationInfo.lineWidth = 1.0;

    m_pipelineComponents.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_pipelineComponents.multisampleInfo.pNext = nullptr;
    m_pipelineComponents.multisampleInfo.flags = 0;
    m_pipelineComponents.multisampleInfo.rasterizationSamples = renderer->m_samples; // TODO: Enable multisampling
    m_pipelineComponents.multisampleInfo.sampleShadingEnable = VK_FALSE;
    m_pipelineComponents.multisampleInfo.minSampleShading = 0;
    m_pipelineComponents.multisampleInfo.pSampleMask = nullptr;
    m_pipelineComponents.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    m_pipelineComponents.multisampleInfo.alphaToOneEnable = VK_FALSE;

    VkStencilOpState states[2];
    states[0].failOp = VK_STENCIL_OP_ZERO;
    states[0].passOp = VK_STENCIL_OP_KEEP;
    states[0].depthFailOp = VK_STENCIL_OP_ZERO;
    states[0].compareOp = VK_COMPARE_OP_LESS;
    states[0].compareMask = 0;
    states[0].writeMask = 0;
    states[0].reference = 0;

    states[1].failOp = VK_STENCIL_OP_ZERO;
    states[1].passOp = VK_STENCIL_OP_KEEP;
    states[1].depthFailOp = VK_STENCIL_OP_ZERO;
    states[1].compareOp = VK_COMPARE_OP_LESS;
    states[1].compareMask = 0;
    states[1].writeMask = 0;
    states[1].reference = 0;

    m_pipelineComponents.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_pipelineComponents.depthStencilInfo.pNext = nullptr;
    m_pipelineComponents.depthStencilInfo.flags = 0;
    m_pipelineComponents.depthStencilInfo.depthTestEnable = VK_TRUE;
    m_pipelineComponents.depthStencilInfo.depthWriteEnable = VK_TRUE;
    m_pipelineComponents.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    m_pipelineComponents.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.stencilTestEnable = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.front = states[0];
    m_pipelineComponents.depthStencilInfo.back = states[1];
    m_pipelineComponents.depthStencilInfo.minDepthBounds = VK_FALSE;
    m_pipelineComponents.depthStencilInfo.maxDepthBounds = VK_FALSE;

    m_pipelineComponents.colorBlendAttachments.resize(3);
    for (int i = 0; i < m_pipelineComponents.colorBlendAttachments.size(); i++)
    {
        m_pipelineComponents.colorBlendAttachments[i].blendEnable = VK_FALSE;
        m_pipelineComponents.colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        m_pipelineComponents.colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_pipelineComponents.colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
        m_pipelineComponents.colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        m_pipelineComponents.colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_pipelineComponents.colorBlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
        m_pipelineComponents.colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                                       VK_COLOR_COMPONENT_G_BIT |
                                                                       VK_COLOR_COMPONENT_B_BIT |
                                                                       VK_COLOR_COMPONENT_A_BIT;
    }

    m_pipelineComponents.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_pipelineComponents.colorBlendInfo.pNext = nullptr;
    m_pipelineComponents.colorBlendInfo.flags = 0;
    m_pipelineComponents.colorBlendInfo.logicOpEnable = VK_FALSE;
    m_pipelineComponents.colorBlendInfo.logicOp = VK_LOGIC_OP_SET;
    m_pipelineComponents.colorBlendInfo.attachmentCount = (uint32_t)m_pipelineComponents.colorBlendAttachments.size();
    m_pipelineComponents.colorBlendInfo.pAttachments = &m_pipelineComponents.colorBlendAttachments[0];
    m_pipelineComponents.colorBlendInfo.blendConstants[0] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[1] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[2] = 1.0;
    m_pipelineComponents.colorBlendInfo.blendConstants[3] = 1.0;

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.setLayoutCount = (uint32_t)m_descriptorSetLayouts.size();
    layoutInfo.pSetLayouts = &m_descriptorSetLayouts[0];
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout(renderer->m_renderDevice, &layoutInfo, nullptr, &m_pipelineLayout);

    m_graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_graphicsPipelineInfo.pNext = nullptr;
    m_graphicsPipelineInfo.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    m_graphicsPipelineInfo.stageCount = (uint32_t)m_pipelineComponents.shaderInfo.size();
    m_graphicsPipelineInfo.pStages = &m_pipelineComponents.shaderInfo[0];
    m_graphicsPipelineInfo.pVertexInputState = &m_pipelineComponents.vertexInfo;
    m_graphicsPipelineInfo.pInputAssemblyState = &m_pipelineComponents.inputAssemblyInfo;
    m_graphicsPipelineInfo.pTessellationState = &m_pipelineComponents.tessellationInfo;
    m_graphicsPipelineInfo.pViewportState = &m_pipelineComponents.viewportInfo;
    m_graphicsPipelineInfo.pRasterizationState = &m_pipelineComponents.rasterizationInfo;
    m_graphicsPipelineInfo.pMultisampleState = &m_pipelineComponents.multisampleInfo;
    m_graphicsPipelineInfo.pDepthStencilState = &m_pipelineComponents.depthStencilInfo;
    m_graphicsPipelineInfo.pColorBlendState = &m_pipelineComponents.colorBlendInfo;
    m_graphicsPipelineInfo.pDynamicState = nullptr;
    m_graphicsPipelineInfo.layout = m_pipelineLayout;
    m_graphicsPipelineInfo.renderPass = renderer->m_renderPasses[0];
    m_graphicsPipelineInfo.subpass = 0;
    m_graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    m_graphicsPipelineInfo.basePipelineIndex = 0;
}

void
VulkanMaterialDelegate::initializeTextures(VulkanRenderer * renderer)
{
    auto defaultTexture = std::make_shared<Texture>("");
    auto defaultCubemap = std::make_shared<Texture>("", Texture::Type::IRRADIANCE_CUBEMAP);

    if (m_material->getTexture(Texture::Type::DIFFUSE))
    {
        m_diffuseTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, m_material->getTexture(Texture::Type::DIFFUSE));
    }
    else
    {
        m_diffuseTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, defaultTexture);
    }

    if (m_material->getTexture(Texture::Type::NORMAL))
    {
        m_normalTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, m_material->getTexture(Texture::Type::NORMAL));
    }
    else
    {
        m_normalTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, defaultTexture);
    }

    if (m_material->getTexture(Texture::Type::ROUGHNESS))
    {
        m_roughnessTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, m_material->getTexture(Texture::Type::ROUGHNESS));
    }
    else
    {
        m_roughnessTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, defaultTexture);
    }

    if (m_material->getTexture(Texture::Type::METALNESS))
    {
        m_metalnessTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, m_material->getTexture(Texture::Type::METALNESS));
    }
    else
    {
        m_metalnessTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, defaultTexture);
    }

    if (m_material->getTexture(Texture::Type::SUBSURFACE_SCATTERING))
    {
        m_subsurfaceScatteringTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, m_material->getTexture(Texture::Type::SUBSURFACE_SCATTERING));
    }
    else
    {
        m_subsurfaceScatteringTexture =
            std::make_shared<VulkanTextureDelegate>(*m_memoryManager, defaultCubemap);
    }
}

void
VulkanMaterialDelegate::addSpecializationConstant(uint32_t size, uint32_t offset)
{
    m_pipelineComponents.fragmentMapEntries.push_back(VkSpecializationMapEntry());

    m_pipelineComponents.fragmentMapEntries[m_numConstants].constantID = m_numConstants;
    m_pipelineComponents.fragmentMapEntries[m_numConstants].offset = offset;
    m_pipelineComponents.fragmentMapEntries[m_numConstants].size = size;

    m_numConstants++;
}

void
VulkanMaterialDelegate::createDescriptors(VulkanRenderer * renderer)
{
    this->createDescriptorPool(renderer);
    this->createDescriptorSets(renderer);
}

void
VulkanMaterialDelegate::createDescriptorSetLayouts(VulkanRenderer * renderer)
{
    m_descriptorSets.resize(2);
    m_descriptorSetLayouts.resize(2);

    // Descriptor Sets
    VkDescriptorSetLayoutBinding vertexDescriptorSetLayoutBindings[2];
    vertexDescriptorSetLayoutBindings[0].binding = 0;
    vertexDescriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vertexDescriptorSetLayoutBindings[0].descriptorCount = 1;
    vertexDescriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                                      | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
                                                      | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    vertexDescriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

    vertexDescriptorSetLayoutBindings[1].binding = 1;
    vertexDescriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vertexDescriptorSetLayoutBindings[1].descriptorCount = 1;
    vertexDescriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                                      | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
                                                      | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    vertexDescriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> fragmentDescriptorSetLayoutBindings;

    // Global uniform buffer
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 0;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Local uniform buffer
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 1;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Diffuse texture
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 2;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Normal texture
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 3;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Roughness texture
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 4;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Metalness texture
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 5;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    // Subsurface scattering texture
    {
        VkDescriptorSetLayoutBinding fragmentLayoutBinding;
        fragmentLayoutBinding.binding = 6;
        fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        fragmentLayoutBinding.descriptorCount = 1;
        fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentLayoutBinding.pImmutableSamplers = nullptr;
        fragmentDescriptorSetLayoutBindings.push_back(fragmentLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo[2];
    descriptorSetLayoutInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo[0].pNext = nullptr;
    descriptorSetLayoutInfo[0].flags = 0;
    descriptorSetLayoutInfo[0].bindingCount = 2;
    descriptorSetLayoutInfo[0].pBindings = vertexDescriptorSetLayoutBindings;

    descriptorSetLayoutInfo[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo[1].pNext = nullptr;
    descriptorSetLayoutInfo[1].flags = 0;
    descriptorSetLayoutInfo[1].bindingCount = (uint32_t)fragmentDescriptorSetLayoutBindings.size();
    descriptorSetLayoutInfo[1].pBindings = &fragmentDescriptorSetLayoutBindings[0];

    for (int i = 0; i < m_descriptorSetLayouts.size(); i++)
    {
        vkCreateDescriptorSetLayout(renderer->m_renderDevice, &descriptorSetLayoutInfo[i], nullptr, &m_descriptorSetLayouts[i]);
    }
}

void
VulkanMaterialDelegate::createDescriptorPool(VulkanRenderer * renderer)
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

    // Vertex shader uniform buffers
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 2;
        descriptorPoolSizes.push_back(poolSize);
    }

    // Fragment shader uniform buffers
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 2;
        descriptorPoolSizes.push_back(poolSize);
    }

    // Fragment shader textures
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 5; // Number of textures
        descriptorPoolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.flags = 0;
    descriptorPoolInfo.maxSets = (uint32_t)m_descriptorSets.size();
    descriptorPoolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
    descriptorPoolInfo.pPoolSizes = &descriptorPoolSizes[0];

    vkCreateDescriptorPool(renderer->m_renderDevice, &descriptorPoolInfo, nullptr, &m_descriptorPool);
}

void
VulkanMaterialDelegate::createDescriptorSets(VulkanRenderer * renderer)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocationInfo[1];
    descriptorSetAllocationInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocationInfo[0].pNext = nullptr;
    descriptorSetAllocationInfo[0].descriptorPool = m_descriptorPool;
    descriptorSetAllocationInfo[0].descriptorSetCount = (uint32_t)m_descriptorSetLayouts.size();
    descriptorSetAllocationInfo[0].pSetLayouts = &m_descriptorSetLayouts[0];

    VkDeviceSize size = { VK_WHOLE_SIZE };

    // Global Buffers
    std::vector<VkDescriptorBufferInfo> vertexBufferInfo(2);
    vertexBufferInfo[0].offset = 0;
    vertexBufferInfo[0].range = size;
    vertexBufferInfo[0].buffer = *(renderer->m_globalVertexUniformBuffer->getUniformBuffer());

    vertexBufferInfo[1].offset = 0;
    vertexBufferInfo[1].range = size;
    vertexBufferInfo[1].buffer = *(m_vertexUniformBuffer->getUniformBuffer());

    // Global buffers
    std::vector<VkDescriptorBufferInfo> fragmentBufferInfo(2);
    fragmentBufferInfo[0].offset = 0;
    fragmentBufferInfo[0].range = size;
    fragmentBufferInfo[0].buffer = *(renderer->m_globalFragmentUniformBuffer->getUniformBuffer());

    fragmentBufferInfo[1].offset = 0;
    fragmentBufferInfo[1].range = size;
    fragmentBufferInfo[1].buffer = *(m_fragmentUniformBuffer->getUniformBuffer());

    std::vector<VkDescriptorImageInfo> fragmentTextureInfo;

    // Textures
    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler = m_diffuseTexture->m_sampler;
        textureInfo.imageView = m_diffuseTexture->m_imageView;
        textureInfo.imageLayout = m_diffuseTexture->m_layout;
        fragmentTextureInfo.push_back(textureInfo);
    }

    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler = m_normalTexture->m_sampler;
        textureInfo.imageView = m_normalTexture->m_imageView;
        textureInfo.imageLayout = m_normalTexture->m_layout;
        fragmentTextureInfo.push_back(textureInfo);
    }

    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler = m_roughnessTexture->m_sampler;
        textureInfo.imageView = m_roughnessTexture->m_imageView;
        textureInfo.imageLayout = m_roughnessTexture->m_layout;
        fragmentTextureInfo.push_back(textureInfo);
    }

    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler = m_metalnessTexture->m_sampler;
        textureInfo.imageView = m_metalnessTexture->m_imageView;
        textureInfo.imageLayout = m_metalnessTexture->m_layout;
        fragmentTextureInfo.push_back(textureInfo);
    }

    {
        VkDescriptorImageInfo textureInfo;
        textureInfo.sampler = m_subsurfaceScatteringTexture->m_sampler;
        textureInfo.imageView = m_subsurfaceScatteringTexture->m_imageView;
        textureInfo.imageLayout = m_subsurfaceScatteringTexture->m_layout;
        fragmentTextureInfo.push_back(textureInfo);
    }

    vkAllocateDescriptorSets(renderer->m_renderDevice, descriptorSetAllocationInfo, &m_descriptorSets[0]);

    m_writeDescriptorSets.resize(2);
    VkWriteDescriptorSet set;
    m_writeDescriptorSets.push_back(set);

    for (int i = 0; i < m_writeDescriptorSets.size(); i++)
    {
        m_writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_writeDescriptorSets[i].pNext = nullptr;
        m_writeDescriptorSets[i].dstBinding = 0;
        m_writeDescriptorSets[i].dstArrayElement = 0;
        m_writeDescriptorSets[i].descriptorCount = 0;
        m_writeDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_writeDescriptorSets[i].pBufferInfo = &vertexBufferInfo[0];
        m_writeDescriptorSets[i].pImageInfo = nullptr;
        m_writeDescriptorSets[i].pTexelBufferView = nullptr;
    }

    // Vertex descriptor set
    m_writeDescriptorSets[0].descriptorCount = 2;
    m_writeDescriptorSets[0].dstSet = m_descriptorSets[0];
    m_writeDescriptorSets[0].pBufferInfo = &vertexBufferInfo[0];

    // Fragment descriptor set
    m_writeDescriptorSets[1].descriptorCount = 2;
    m_writeDescriptorSets[1].dstSet = m_descriptorSets[1];
    m_writeDescriptorSets[1].pBufferInfo = &fragmentBufferInfo[0];

    // Fragment texture descriptor set
    m_writeDescriptorSets[2].descriptorCount = (uint32_t)fragmentTextureInfo.size();
    m_writeDescriptorSets[2].dstBinding = 2;
    m_writeDescriptorSets[2].dstSet = m_descriptorSets[1];
    m_writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    m_writeDescriptorSets[2].pImageInfo = &fragmentTextureInfo[0];

    vkUpdateDescriptorSets(renderer->m_renderDevice, (uint32_t)m_writeDescriptorSets.size(), &m_writeDescriptorSets[0], 0, nullptr);
}
}