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

#include "imstkVulkanRenderer.h"

namespace imstk {
VulkanRenderer::VulkanRenderer(std::shared_ptr<Scene> scene)
{
    m_scene = scene;
}

void
VulkanRenderer::initialize()
{
    // If debug mode, enable validation layer (slower performance)
#ifndef NDEBUG
    m_layers.push_back(VulkanValidation::getValidationLayer());
    m_extensions.push_back(VulkanValidation::getValidationExtension());
#endif

    // Instance of a Vulkan application
    VkInstanceCreateInfo m_creationInfo;
    m_creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    m_creationInfo.pNext = nullptr;
    m_creationInfo.flags = 0;
    m_creationInfo.pApplicationInfo = nullptr;
    m_creationInfo.enabledLayerCount = (uint32_t)m_layers.size();
    m_creationInfo.ppEnabledLayerNames = &m_layers[0];
    m_creationInfo.enabledExtensionCount = (uint32_t)m_extensions.size();
    m_creationInfo.ppEnabledExtensionNames = &m_extensions[0];

    std::cout << "\n" << "Vulkan Renderer Information:" << std::endl;

    for (int i = 0; i < m_extensions.size(); i++)
    {
        std::cout << "Enabled extension: " << m_creationInfo.ppEnabledExtensionNames[i] << std::endl;
    }

    m_instance = new VkInstance();

    vkCreateInstance(&m_creationInfo, nullptr, m_instance);

#ifndef NDEBUG
    VkDebugReportCallbackCreateInfoEXT debugReportInfo;
    debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugReportInfo.pNext = nullptr;
    debugReportInfo.flags =
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT;
    debugReportInfo.pfnCallback = &(VulkanValidation::debugReportCallback);
    debugReportInfo.pUserData = nullptr;

    VkDebugReportCallbackEXT debugReportCallback;
    PFN_vkCreateDebugReportCallbackEXT createCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*m_instance, "vkCreateDebugReportCallbackEXT");
    createCallback(*m_instance, &debugReportInfo, nullptr, &debugReportCallback);
#endif

    auto camera = m_scene->getCamera();
    m_fov = (float)glm::radians(camera->getViewAngle());

    // Setup logical devices
    this->setupGPUs();
    this->printGPUs();

    // Setup command pool(s) - right now we just have one
    this->setupCommandPools();
    this->buildCommandBuffer();
    this->setupRenderPasses();
    this->setupSynchronization();
    this->setupMemoryManager();
    this->createGlobalUniformBuffers();

    std::vector<VkPipeline> graphicsPipelines;
    std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelinesInfo;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_renderPhysicalDevice, &deviceProperties);
    m_deviceLimits = deviceProperties.limits;

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.pNext = nullptr;
    pipelineCacheCreateInfo.flags = 0;
    pipelineCacheCreateInfo.initialDataSize = 0;
    pipelineCacheCreateInfo.pInitialData = nullptr;

    vkCreatePipelineCache(m_renderDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);
}

void
VulkanRenderer::setupGPUs()
{
    // Prevent devices from being set up multiple times
    if (m_physicalDeviceCount != 0)
    {
        return;
    }

    // Setup physical devices
    vkEnumeratePhysicalDevices(*m_instance, &m_physicalDeviceCount, nullptr);
    m_physicalDevices = new VkPhysicalDevice[(int)m_physicalDeviceCount]();
    vkEnumeratePhysicalDevices(*m_instance, &m_physicalDeviceCount, m_physicalDevices);
    m_renderPhysicalDevice = m_physicalDevices[0];

    // Get render queue family
    vkGetPhysicalDeviceQueueFamilyProperties(m_renderPhysicalDevice, &m_queueFamilyPropertiesCount, nullptr);
    m_queueFamilyProperties = new VkQueueFamilyProperties[(int)m_queueFamilyPropertiesCount]();
    vkGetPhysicalDeviceQueueFamilyProperties(m_renderPhysicalDevice, &m_queueFamilyPropertiesCount, m_queueFamilyProperties);

    m_renderQueueFamily = 0;

    for (int i = 0; i < (int)(m_queueFamilyPropertiesCount); i++)
    {
        if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_renderQueueFamily = i;
            break;
        }
    }

    std::vector<float> priorities(m_queueFamilyProperties[m_renderQueueFamily].queueCount);
    std::fill(priorities.begin(), priorities.end(), 0.0f);
    priorities[m_renderQueueFamily] = 1.0;

    //Setup logical devices
    m_deviceCount = m_physicalDeviceCount;
    m_devices = new VkDevice[(int)(m_deviceCount)]();

    VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.flags = 0;
    queueInfo.queueFamilyIndex = m_renderQueueFamily;
    queueInfo.queueCount = m_queueFamilyProperties[m_renderQueueFamily].queueCount;
    queueInfo.pQueuePriorities = &priorities[0];

    // The display system isn't part of the Vulkan core
    char * deviceExtensions[1];
    deviceExtensions[0] = "VK_KHR_swapchain";

    // Enabling optional Vulkan features
    VkPhysicalDeviceFeatures features = {VK_FALSE};
    features.fillModeNonSolid = VK_TRUE;
    features.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
#ifndef NDEBUG
    deviceInfo.enabledLayerCount = (uint32_t)m_layers.size();
    deviceInfo.ppEnabledLayerNames = &m_layers[0];
#else
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = nullptr;
#endif
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceInfo.pEnabledFeatures = &features;

    for (int i = 0; i < (int)(m_physicalDeviceCount); i++)
    {
        vkCreateDevice(m_physicalDevices[i], &deviceInfo, nullptr, &m_devices[i]);
    }

    // This decision needs some work, may pick weaker device
    m_renderDevice = m_devices[0];

    // Get the first render-capable queue
    vkGetDeviceQueue(m_renderDevice, m_renderQueueFamily, 0, &m_renderQueue);
}

void
VulkanRenderer::printGPUs()
{
    this->setupGPUs();

    VkPhysicalDeviceProperties properties;

    std::cout << "Devices:" << std::endl;

    for (int i = 0; i < (int)(m_physicalDeviceCount); i++)
    {
        vkGetPhysicalDeviceProperties(m_physicalDevices[i], &properties);
        properties.limits.maxMemoryAllocationCount;
        std::cout << (i + 1) << ". " << properties.deviceName << std::endl;
    }
}

void
VulkanRenderer::setupCommandPools()
{
    // Create command pools (only one for now)
    VkCommandPoolCreateInfo commandPoolInfo;
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = 0;

    vkCreateCommandPool(m_renderDevice, &commandPoolInfo, nullptr, &m_renderCommandPool);
}

void
VulkanRenderer::buildCommandBuffer()
{
    // Build command buffer
    VkCommandBufferAllocateInfo commandBufferInfo;
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.pNext = nullptr;
    commandBufferInfo.commandPool = m_renderCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = 1; // TODO: increase this for double/triple buffering

    vkAllocateCommandBuffers(m_renderDevice, &commandBufferInfo, &m_renderCommandBuffer);

    vkAllocateCommandBuffers(m_renderDevice, &commandBufferInfo, &m_postProcessingCommandBuffer);
}

void
VulkanRenderer::setupRenderPasses()
{
    // Number of geometry passes
    m_renderPasses.resize(2);

    VulkanRenderPassGenerator::generateOpaqueRenderPass(m_renderDevice, m_renderPasses[0], m_samples);
    VulkanRenderPassGenerator::generateDecalRenderPass(m_renderDevice, m_renderPasses[1], m_samples);
}

void
VulkanRenderer::resizeFramebuffers(VkSwapchainKHR * swapchain, int width, int height)
{
    m_width = width;
    m_height = height;

    this->deleteFramebuffers();

    this->initializeFramebuffers(swapchain);

    std::vector<VkPipeline> pipelines;
    std::vector<VkGraphicsPipelineCreateInfo> pipelineInfos;

    for (int i = 0; i < m_renderDelegates.size(); i++)
    {
        auto material = m_renderDelegates[i]->m_material;
        vkDestroyPipeline(m_renderDevice, material->m_pipeline, nullptr);
        material->initialize(this);
    }
}

void
VulkanRenderer::initializeFramebuffers(VkSwapchainKHR * swapchain)
{
    m_mipLevels = std::log2(std::max(m_width, m_height)) + 1;

    // Get images from surface (color images)
    m_swapchain = swapchain;
    vkGetSwapchainImagesKHR(m_renderDevice, *m_swapchain, &m_swapchainImageCount, nullptr);
    m_swapchainImages.resize(m_swapchainImageCount);
    vkGetSwapchainImagesKHR(m_renderDevice, *m_swapchain, &m_swapchainImageCount, &m_swapchainImages[0]);

    // Depth image
    VkImageCreateInfo depthImageInfo;
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.pNext = nullptr;
    depthImageInfo.flags = 0;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
    depthImageInfo.extent = { m_width, m_height, 1 };
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = m_samples;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           | VK_IMAGE_USAGE_SAMPLED_BIT;
    depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthImageInfo.queueFamilyIndexCount = 0;
    depthImageInfo.pQueueFamilyIndices = nullptr;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_depthImage.resize(m_mipLevels);
    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        depthImageInfo.extent = { std::max(m_width >> i, 1u), std::max(m_height >> i, 1u), 1 };
        vkCreateImage(m_renderDevice, &depthImageInfo, nullptr, &m_depthImage[i]);
    }

    // Normal image
    auto normalImageInfo = depthImageInfo;
    normalImageInfo.extent = { m_width, m_height, 1 };
    normalImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT;
    normalImageInfo.format = VK_FORMAT_R8G8B8A8_SNORM;

    vkCreateImage(m_renderDevice, &normalImageInfo, nullptr, &m_normalImage);

    // HDR image
    auto HDRImageInfo = depthImageInfo;
    HDRImageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    HDRImageInfo.mipLevels = 1;
    HDRImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                         | VK_IMAGE_USAGE_SAMPLED_BIT;

    m_HDRImage[0].resize(m_mipLevels);
    m_HDRImage[1].resize(m_mipLevels);
    m_HDRImage[2].resize(m_mipLevels);
    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        HDRImageInfo.extent = { std::max(m_width >> i, 1u), std::max(m_height >> i, 1u), 1 };
        vkCreateImage(m_renderDevice, &HDRImageInfo, nullptr, &m_HDRImage[0][i]);
        vkCreateImage(m_renderDevice, &HDRImageInfo, nullptr, &m_HDRImage[1][i]);
        vkCreateImage(m_renderDevice, &HDRImageInfo, nullptr, &m_HDRImage[2][i]);
    }

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_HDRTonemaps.resize(m_swapchainImageCount);
    }

    // Create image views
    m_swapchainImageViews.resize(m_swapchainImageCount);
    m_depthImageView.resize(m_mipLevels);
    m_depthImageMemory.resize(m_mipLevels);

    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(m_renderDevice, m_depthImage[i], &memReqs);

        m_depthImageMemory[i] = m_memoryManager.allocateMemory(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkBindImageMemory(m_renderDevice, m_depthImage[i], *m_depthImageMemory[i], 0);

        VkComponentMapping componentMap;
        componentMap.r = VK_COMPONENT_SWIZZLE_R;
        componentMap.g = VK_COMPONENT_SWIZZLE_G;
        componentMap.b = VK_COMPONENT_SWIZZLE_B;
        componentMap.a = VK_COMPONENT_SWIZZLE_A;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = m_depthImage[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VK_FORMAT_D32_SFLOAT;
        imageViewInfo.components = componentMap;
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_depthImageView[i]);
    }

    {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(m_renderDevice, m_normalImage, &memReqs);

        m_normalImageMemory = m_memoryManager.allocateMemory(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkBindImageMemory(m_renderDevice, m_normalImage, *m_normalImageMemory, 0);

        VkComponentMapping componentMap;
        componentMap.r = VK_COMPONENT_SWIZZLE_R;
        componentMap.g = VK_COMPONENT_SWIZZLE_G;
        componentMap.b = VK_COMPONENT_SWIZZLE_B;
        componentMap.a = VK_COMPONENT_SWIZZLE_A;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = m_normalImage;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VK_FORMAT_R8G8B8A8_SNORM;
        imageViewInfo.components = componentMap;
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_normalImageView);
    }

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Trilinear interpolation
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias = 0.0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(m_renderDevice, &samplerInfo, nullptr, &m_HDRImageSampler);

    for (int i = 0; i < 3; i++)
    {
        m_HDRImageView[i].resize(m_mipLevels);
        m_HDRImageMemory[i].resize(m_mipLevels);

        for (uint32_t j = 0; j < m_mipLevels; j++)
        {
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(m_renderDevice, m_HDRImage[i][j], &memReqs);

            m_HDRImageMemory[i][j] = m_memoryManager.allocateMemory(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vkBindImageMemory(m_renderDevice, m_HDRImage[i][j], *m_HDRImageMemory[i][j], 0);

            VkComponentMapping componentMap;
            componentMap.r = VK_COMPONENT_SWIZZLE_R;
            componentMap.g = VK_COMPONENT_SWIZZLE_G;
            componentMap.b = VK_COMPONENT_SWIZZLE_B;
            componentMap.a = VK_COMPONENT_SWIZZLE_A;

            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkImageViewCreateInfo imageViewInfo;
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.pNext = nullptr;
            imageViewInfo.flags = 0;
            imageViewInfo.image = m_HDRImage[i][j];
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            imageViewInfo.components = componentMap;
            imageViewInfo.subresourceRange = subresourceRange;

            vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_HDRImageView[i][j]);
        }
    }

    m_swapchainImageSamplers.resize(m_swapchainImageCount);
    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        VkComponentMapping componentMap;
        componentMap.r = VK_COMPONENT_SWIZZLE_R;
        componentMap.g = VK_COMPONENT_SWIZZLE_G;
        componentMap.b = VK_COMPONENT_SWIZZLE_B;
        componentMap.a = VK_COMPONENT_SWIZZLE_A;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = m_swapchainImages[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        imageViewInfo.components = componentMap;
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_swapchainImageViews[i]);

        VkSamplerCreateInfo samplerInfo;
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Trilinear interpolation
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0;
        samplerInfo.anisotropyEnable = VK_FALSE; // TODO:: add option to enable
        samplerInfo.maxAnisotropy = 1.0;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        vkCreateSampler(m_renderDevice, &samplerInfo, nullptr, &m_swapchainImageSamplers[i]);
    }

    this->initializePostProcesses();

    m_drawingFramebuffers.clear();
    m_drawingFramebuffers.push_back(
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, false, m_samples));
    m_drawingFramebuffers[0]->setColor(&m_HDRImageView[0][0], VK_FORMAT_R16G16B16A16_SFLOAT);
    m_drawingFramebuffers[0]->setSpecular(&m_HDRImageView[1][0], VK_FORMAT_R16G16B16A16_SFLOAT);
    m_drawingFramebuffers[0]->setDepth(&m_depthImageView[0], VK_FORMAT_D32_SFLOAT);
    m_drawingFramebuffers[0]->setNormal(&m_normalImageView, VK_FORMAT_R8G8B8A8_SNORM);
    m_drawingFramebuffers[0]->initializeFramebuffer(&m_renderPasses[0]);

    m_drawingFramebuffers.push_back(
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, false, m_samples));
    m_drawingFramebuffers[1]->setColor(&m_HDRImageView[0][0], VK_FORMAT_R16G16B16A16_SFLOAT);
    m_drawingFramebuffers[1]->setSpecular(&m_HDRImageView[1][0], VK_FORMAT_R16G16B16A16_SFLOAT);
    m_drawingFramebuffers[1]->setDepth(&m_depthImageView[0], VK_FORMAT_D32_SFLOAT);
    m_drawingFramebuffers[1]->initializeFramebuffer(&m_renderPasses[1]);
}

void
VulkanRenderer::deleteFramebuffers()
{
    // The framebuffers/command buffers may still be in use
    vkDeviceWaitIdle(m_renderDevice);

    // Depth buffer
    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        vkDestroyImage(m_renderDevice, m_depthImage[i], nullptr);
        vkDestroyImageView(m_renderDevice, m_depthImageView[i], nullptr);
    }


    // HDR buffers
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < m_HDRImageView[i].size(); j++)
        {
            vkDestroyImage(m_renderDevice, m_HDRImage[i][j], nullptr);
            vkDestroyImageView(m_renderDevice, m_HDRImageView[i][j], nullptr);
        }
    }

    // Normal buffer
    vkDestroyImage(m_renderDevice, m_normalImage, nullptr);
    vkDestroyImageView(m_renderDevice, m_normalImageView, nullptr);

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        vkDestroyImageView(m_renderDevice, m_swapchainImageViews[i], nullptr);
    }

    // Delete all post processing resources
    for (int i = 0; i < m_postProcessingChain->m_postProcesses.size(); i++)
    {
        vkDestroyFramebuffer(m_renderDevice, m_postProcessingChain->m_postProcesses[i]->m_framebuffer->m_framebuffer, nullptr);
    }

    // Delete all HDR resources
    for (int i = 0; i < m_HDRTonemaps.size(); i++)
    {
        vkDestroyFramebuffer(m_renderDevice, m_HDRTonemaps[i]->m_framebuffer->m_framebuffer, nullptr);
    }

    // Delete all drawing resources
    for (int i = 0; i < m_drawingFramebuffers.size() - 1; i++)
    {
        vkDestroyFramebuffer(m_renderDevice, m_drawingFramebuffers[i]->m_framebuffer, nullptr);
    }
}

void
VulkanRenderer::renderFrame()
{
    m_frameNumber++;

    this->loadAllGeometry();

    // Update global uniforms
    this->updateGlobalUniforms();

    // Update local uniforms
    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getGeometry()->getType() == Geometry::Type::DecalPool)
        {
            auto decalPool = std::dynamic_pointer_cast<VulkanDecalRenderDelegate>(m_renderDelegates[renderDelegateIndex]);
            decalPool->update(m_scene->getCamera());
        }
        m_renderDelegates[renderDelegateIndex]->update();
    }

    // The swapchain contains multiple buffers, so get one that is available (i.e., not currently being written to)
    uint32_t nextImageIndex;
    vkAcquireNextImageKHR(m_renderDevice, *m_swapchain, UINT64_MAX, m_readyToRender, VK_NULL_HANDLE, &nextImageIndex);

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(m_renderCommandBuffer, &commandBufferBeginInfo);

    VkRect2D renderArea;
    renderArea.offset = { 0, 0 };
    renderArea.extent = { m_width, m_height };

    std::array<VkClearValue, 4> clearValues;
    clearValues[0].color = { { (float)m_backgroundColor[0], (float)m_backgroundColor[1], (float)m_backgroundColor[2], 1 } }; // Color
    clearValues[1].depthStencil = { { 1.0 }, { 0 } }; // Depth
    clearValues[2].color = { { 0, 0, 0, 0 } }; // Normal
    clearValues[3].color = { { 0, 0, 0, 0 } }; // Specular

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = m_renderPasses[0];
    renderPassBeginInfo.framebuffer = m_drawingFramebuffers[0]->m_framebuffer;
    renderPassBeginInfo.renderArea = renderArea;
    renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassBeginInfo.pClearValues = &clearValues[0];

    // Do buffer transfers
    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();
        buffers->uploadBuffers(m_renderCommandBuffer);
    }

    vkCmdBeginRenderPass(m_renderCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkDeviceSize deviceSize = { 0 };

    // Pass 1: Render opaque geometry
    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getGeometry()->getType() == Geometry::Type::DecalPool)
        {
            continue;
        }

        auto material = m_renderDelegates[renderDelegateIndex]->m_material;
        vkCmdBindPipeline(m_renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);

        vkCmdBindDescriptorSets(m_renderCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        vkCmdBindVertexBuffers(m_renderCommandBuffer, 0, 1, &buffers->m_vertexBuffer, &deviceSize);
        vkCmdBindIndexBuffer(m_renderCommandBuffer, buffers->m_indexBuffer, deviceSize, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(m_renderCommandBuffer, buffers->m_numIndices, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(m_renderCommandBuffer);

    // Pass 2: Render decals
    VkRenderPassBeginInfo decalRenderPassBeginInfo;
    decalRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    decalRenderPassBeginInfo.pNext = nullptr;
    decalRenderPassBeginInfo.renderPass = m_renderPasses[1];
    decalRenderPassBeginInfo.framebuffer = m_drawingFramebuffers[1]->m_framebuffer;
    decalRenderPassBeginInfo.renderArea = renderArea;
    decalRenderPassBeginInfo.clearValueCount = 0;
    decalRenderPassBeginInfo.pClearValues = &clearValues[0];
    vkCmdBeginRenderPass(m_renderCommandBuffer, &decalRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getGeometry()->getType() != Geometry::Type::DecalPool)
        {
            continue;
        }

        auto geometry = std::dynamic_pointer_cast<DecalPool>(m_renderDelegates[renderDelegateIndex]->getGeometry());
        auto material = m_renderDelegates[renderDelegateIndex]->m_material;
        vkCmdBindPipeline(m_renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);

        vkCmdBindDescriptorSets(m_renderCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        vkCmdBindVertexBuffers(m_renderCommandBuffer, 0, 1, &buffers->m_vertexBuffer, &deviceSize);
        vkCmdBindIndexBuffer(m_renderCommandBuffer, buffers->m_indexBuffer, deviceSize, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(m_renderCommandBuffer, buffers->m_numIndices, geometry->getNumDecals(), 0, 0, 0);
    }

    vkCmdEndRenderPass(m_renderCommandBuffer);
    vkEndCommandBuffer(m_renderCommandBuffer);

    vkBeginCommandBuffer(m_postProcessingCommandBuffer, &commandBufferBeginInfo);

    // Pass 3 to N - 1: Post processing
    for (unsigned int postProcessIndex = 0; postProcessIndex < m_postProcessingChain->m_postProcesses.size(); postProcessIndex++)
    {
        clearValues[0].color = { { 1.0, 0.0, 0.0, 1 } }; // Color

        auto postProcess = m_postProcessingChain->m_postProcesses[postProcessIndex];

        auto postProcessRenderPassBeginInfo = renderPassBeginInfo;
        auto framebuffer = postProcess->m_framebuffer;
        postProcessRenderPassBeginInfo.renderPass =
            postProcess->m_renderPass;
        postProcessRenderPassBeginInfo.framebuffer = framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)framebuffer->m_attachments.size();
        postProcessRenderPassBeginInfo.renderArea.extent = { framebuffer->m_width, framebuffer->m_height };

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer, &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdPushConstants(m_postProcessingCommandBuffer, postProcess->m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128, (void*)postProcess->m_pushConstantData);

        vkCmdBindPipeline(m_postProcessingCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipeline);

        vkCmdBindDescriptorSets(m_postProcessingCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipelineLayout, 0,
            (uint32_t)postProcess->m_descriptorSets.size(),
            &postProcess->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = postProcess->m_vertexBuffer;
        vkCmdBindVertexBuffers(m_postProcessingCommandBuffer, 0, 1, &buffers->m_vertexBuffer, &deviceSize);
        vkCmdBindIndexBuffer(m_postProcessingCommandBuffer, buffers->m_indexBuffer, deviceSize, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(m_postProcessingCommandBuffer, buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_postProcessingCommandBuffer);
    }

    // Pass N: HDR tonemap (this is special because of the swapchain)
    {
        auto postProcessRenderPassBeginInfo = renderPassBeginInfo;
        postProcessRenderPassBeginInfo.renderPass = m_HDRTonemaps[nextImageIndex]->m_renderPass;
        postProcessRenderPassBeginInfo.framebuffer = m_HDRTonemaps[nextImageIndex]->m_framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)m_HDRTonemaps[nextImageIndex]->m_framebuffer->m_attachments.size();

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer, &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_postProcessingCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_HDRTonemaps[nextImageIndex]->m_pipeline);

        vkCmdBindDescriptorSets(m_postProcessingCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_HDRTonemaps[nextImageIndex]->m_pipelineLayout, 0, (uint32_t)m_HDRTonemaps[nextImageIndex]->m_descriptorSets.size(),
            &m_HDRTonemaps[nextImageIndex]->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_HDRTonemaps[nextImageIndex]->m_vertexBuffer;
        vkCmdBindVertexBuffers(m_postProcessingCommandBuffer, 0, 1, &buffers->m_vertexBuffer, &deviceSize);
        vkCmdBindIndexBuffer(m_postProcessingCommandBuffer, buffers->m_indexBuffer, deviceSize, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(m_postProcessingCommandBuffer, buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_postProcessingCommandBuffer);
    }

    vkEndCommandBuffer(m_postProcessingCommandBuffer);

    VkCommandBuffer commandBuffers[2];
    commandBuffers[0] = m_renderCommandBuffer;
    commandBuffers[1] = m_postProcessingCommandBuffer;

    VkPipelineStageFlags stageWaitFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[2];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount = 1;
    submitInfo[0].pWaitSemaphores = &m_readyToRender;
    submitInfo[0].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = &commandBuffers[0];
    submitInfo[0].signalSemaphoreCount = 1;
    submitInfo[0].pSignalSemaphores = &m_drawingComplete;

    submitInfo[1] = submitInfo[0];
    submitInfo[1].pWaitSemaphores = &m_drawingComplete;
    submitInfo[1].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[1].pCommandBuffers = &commandBuffers[1];
    submitInfo[1].pSignalSemaphores = &m_presentImages;

    // Submit command buffers
    vkQueueSubmit(m_renderQueue, 2, submitInfo, m_commandBufferSubmit);

    VkSwapchainKHR swapchains[] = {*m_swapchain};

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_presentImages;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &nextImageIndex; // change this in the future for multi-screen rendering
    presentInfo.pResults = nullptr;

    // Display backbuffer
    vkQueuePresentKHR(m_renderQueue, &presentInfo);

    // Wait until command buffer is done so that we can write to it again
    vkWaitForFences(m_renderDevice, 1, &m_commandBufferSubmit, VK_TRUE, UINT64_MAX);
    vkResetFences(m_renderDevice, 1, &m_commandBufferSubmit);
}

void
VulkanRenderer::setupSynchronization()
{
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    vkCreateSemaphore(m_renderDevice, &semaphoreInfo, nullptr, &m_readyToRender);

    vkCreateSemaphore(m_renderDevice, &semaphoreInfo, nullptr, &m_presentImages);

    vkCreateSemaphore(m_renderDevice, &semaphoreInfo, nullptr, &m_drawingComplete);

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0;

    vkCreateFence(m_renderDevice, &fenceInfo, nullptr, &m_commandBufferSubmit);
}

void
VulkanRenderer::loadAllGeometry()
{
    // Add new objects
    for (auto sceneObject : m_scene->getSceneObjects())
    {
        auto geometry = sceneObject->getVisualGeometry();

        if (geometry && !geometry->m_renderDelegateCreated)
        {
            auto renderDelegate = this->loadGeometry(geometry);
        }
    }
}

std::shared_ptr<VulkanRenderDelegate>
VulkanRenderer::loadGeometry(std::shared_ptr<Geometry> geometry)
{
    auto renderDelegate = VulkanRenderDelegate::make_delegate(geometry, m_memoryManager);
    if (renderDelegate != nullptr)
    {
        m_renderDelegates.push_back(renderDelegate);
        renderDelegate->getBuffer()->initializeBuffers(m_memoryManager);
        renderDelegate->m_material->initialize(this);
    }
    return renderDelegate;
}

void
VulkanRenderer::setupMemoryManager()
{
    m_memoryManager.m_device = m_renderDevice;
    m_memoryManager.m_physicalDevice = m_renderPhysicalDevice;
    m_memoryManager.m_queueFamilyIndex = m_renderQueueFamily;
    m_memoryManager.m_transferCommandBuffer = &m_renderCommandBuffer;
    m_memoryManager.m_transferQueue = &m_renderQueue;
}

void
VulkanRenderer::createGlobalUniformBuffers()
{
    m_globalVertexUniformBuffer = std::make_shared<VulkanUniformBuffer>(m_memoryManager, (uint32_t)sizeof(VulkanGlobalVertexUniforms));
    m_globalFragmentUniformBuffer = std::make_shared<VulkanUniformBuffer>(m_memoryManager, (uint32_t)sizeof(VulkanGlobalFragmentUniforms));
}

void
VulkanRenderer::initializePostProcesses()
{
    std::vector<VkPipeline> graphicsPipelines;
    std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelinesInfo;
    m_postProcessingChain = std::make_shared<VulkanPostProcessingChain>(this);

    // HDR pipeline creation
    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_HDRTonemaps[i] = std::make_shared<VulkanPostProcess>(this, 0, true);
        m_HDRTonemaps[i]->addInputImage(&m_swapchainImageSamplers[i], &m_HDRImageView[m_postProcessingChain->m_lastOutput][0]);
        m_HDRTonemaps[i]->m_framebuffer->setColor(&m_swapchainImageViews[i], VK_FORMAT_B8G8R8A8_SRGB);
        m_HDRTonemaps[i]->initialize(this, "./Shaders/VulkanShaders/PostProcessing/HDR_tonemap_frag.spv");

        graphicsPipelines.push_back(m_HDRTonemaps[i]->m_pipeline);
        graphicsPipelinesInfo.push_back(m_HDRTonemaps[i]->m_graphicsPipelineInfo);
    }

    // Post processing pipeline creation
    for (int i = 0; i < m_postProcessingChain->m_postProcesses.size(); i++)
    {
        graphicsPipelines.push_back(m_postProcessingChain->m_postProcesses[i]->m_pipeline);
        graphicsPipelinesInfo.push_back(m_postProcessingChain->m_postProcesses[i]->m_graphicsPipelineInfo);
    }

    vkCreateGraphicsPipelines(m_renderDevice,
        m_pipelineCache,
        (uint32_t)graphicsPipelines.size(),
        &graphicsPipelinesInfo[0],
        nullptr,
        &graphicsPipelines[0]);

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_HDRTonemaps[i]->m_pipeline = graphicsPipelines[i];
    }

    for (int i = m_swapchainImageCount; i < m_postProcessingChain->m_postProcesses.size() + m_swapchainImageCount; i++)
    {
        m_postProcessingChain->m_postProcesses[i - m_swapchainImageCount]->m_pipeline = graphicsPipelines[i];
    }
}

void
VulkanRenderer::updateGlobalUniforms()
{
    // Vertex uniforms
    {
        // Projection matrix
        auto camera = m_scene->getCamera();
        m_fov = (float)glm::radians(camera->getViewAngle());
        m_globalVertexUniforms.projectionMatrix = glm::perspective(m_fov, (float)(m_width) / (float)(m_height), m_nearPlane, m_farPlane);
        glm::mat4 correctionMatrix; // for Vulkan rendering
        correctionMatrix[1][1] = -1;
        correctionMatrix[2][2] = 0.5;
        correctionMatrix[3][2] = 0.5;
        m_globalVertexUniforms.projectionMatrix *= correctionMatrix;

        // View matrix
        auto eye = glm::tvec3<float>(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z());
        auto center = glm::tvec3<float>(camera->getFocalPoint().x(), camera->getFocalPoint().y(), camera->getFocalPoint().z());
        auto up = glm::tvec3<float>(camera->getViewUp().x(), camera->getViewUp().y(), camera->getViewUp().z());
        m_globalVertexUniforms.cameraPosition = glm::vec4(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z(), 0.0);
        m_globalVertexUniforms.viewMatrix = glm::lookAt(eye, center, up);
    }

    // Lights uniforms
    {
        // Update lights - need to optimize
        auto lights = m_scene->getLights();
        for (int i = 0; i < lights.size(); i++)
        {
            // Only supports directional lights right now
            auto focalPoint = lights[i]->getFocalPoint();
            auto position = Vec3d(0,0,0);
            int type = 1;

            if (lights[i]->getType() == LightType::POINT_LIGHT || lights[i]->getType() == LightType::SPOT_LIGHT)
            {
                position = std::static_pointer_cast<PointLight>(lights[i])->getPosition();
                type = 2;
            }

            m_globalFragmentUniforms.lights[i].position = glm::vec3(position.x(), position.y(), position.z());

            m_globalFragmentUniforms.lights[i].direction.x = focalPoint.x() - position.x();
            m_globalFragmentUniforms.lights[i].direction.y = focalPoint.y() - position.y();
            m_globalFragmentUniforms.lights[i].direction.z = focalPoint.z() - position.z();

            m_globalFragmentUniforms.lights[i].direction = glm::normalize(m_globalFragmentUniforms.lights[i].direction);

            Color lightColor = lights[i]->getColor();
            m_globalFragmentUniforms.lights[i].color = glm::vec4(lightColor.r, lightColor.g, lightColor.b, 1.0);

            if (lights[i]->getType() == LightType::SPOT_LIGHT)
            {
                m_globalFragmentUniforms.lights[i].direction.a =
                    glm::radians(std::static_pointer_cast<SpotLight>(lights[i])->getSpotAngle());
                type = 3;
            }

            m_globalFragmentUniforms.lights[i].color.a = lights[i]->getIntensity();

            m_globalFragmentUniforms.lights[i].type = type;
        }

        memcpy(&m_globalVertexUniforms.lights, &m_globalFragmentUniforms.lights, sizeof(m_globalFragmentUniforms.lights));

        m_globalFragmentUniforms.inverseViewMatrix = glm::inverse(m_globalVertexUniforms.viewMatrix);
        m_globalFragmentUniforms.inverseProjectionMatrix = glm::inverse(m_globalVertexUniforms.projectionMatrix);
        m_globalFragmentUniforms.resolution = glm::vec4(m_width, m_height, 0, 0);
    }

    m_globalVertexUniformBuffer->updateUniforms(sizeof(VulkanGlobalVertexUniforms), &m_globalVertexUniforms);
    m_globalFragmentUniformBuffer->updateUniforms(sizeof(VulkanGlobalFragmentUniforms), &m_globalFragmentUniforms);
}

VulkanRenderer::~VulkanRenderer()
{
    // Delete devices
    for (int i = 0; i < (int)(m_deviceCount); i++)
    {
        // Important: must wait for device to be finished
        vkDeviceWaitIdle(m_devices[i]);
        vkDestroySemaphore(m_renderDevice, m_readyToRender, nullptr);
        vkDestroySemaphore(m_renderDevice, m_drawingComplete, nullptr);

        // Clear all memory
        m_memoryManager.clear();

        for (int x = 0; x < m_renderDelegates.size(); x++)
        {
            // TODO: Clear all render delegates
        }

        vkDestroyDevice(m_devices[i], nullptr);
    }

    vkDestroyInstance(*m_instance, nullptr);
}
}