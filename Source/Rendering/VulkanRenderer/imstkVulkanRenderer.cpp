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

namespace imstk
{
VulkanRenderer::VulkanRenderer(std::shared_ptr<Scene> scene)
{
    m_scene = scene;
}

void
VulkanRenderer::createInstance()
{
    // If debug mode, enable validation layer (slower performance)
#ifndef NDEBUG
    m_layers.push_back(VulkanValidation::getValidationLayer());
    m_extensions.push_back(VulkanValidation::getValidationExtension());
#endif

#ifdef iMSTK_ENABLE_VR
    if (m_VRMode)
    {
        uint32_t numBytes = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
        std::vector<char> vrInstanceExtensions(numBytes);
        vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(&vrInstanceExtensions[0], numBytes);
        std::string tempVRExtensionString(vrInstanceExtensions.begin(), vrInstanceExtensions.end());
        std::istringstream vrInstanceExtensionsStream(tempVRExtensionString);
        std::string tempVRInstanceExtension;

        while (std::getline(vrInstanceExtensionsStream, tempVRInstanceExtension, ' '))
        {
            m_extensions.push_back(tempVRInstanceExtension.c_str());
        }

        m_numViews = 2; // for stereo rendering
    }
#endif

    std::vector<const char*> finalExtensions;
    for (auto extension : m_extensions)
    {
        auto temp = new char[extension.size() + 1];
        memcpy(temp, extension.c_str(), extension.size() + 1);
        finalExtensions.push_back(temp);
    }

    // Instance of a Vulkan application
    VkInstanceCreateInfo m_creationInfo;
    m_creationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    m_creationInfo.pNext = nullptr;
    m_creationInfo.flags = 0;
    m_creationInfo.pApplicationInfo = nullptr;
    m_creationInfo.enabledLayerCount = (uint32_t)m_layers.size();
    m_creationInfo.ppEnabledLayerNames = &m_layers[0];
    m_creationInfo.enabledExtensionCount = (uint32_t)m_extensions.size();
    m_creationInfo.ppEnabledExtensionNames = &finalExtensions[0];

    std::cout << "\n" << "Vulkan Renderer Information:" << std::endl;

    for (int i = 0; i < m_extensions.size(); i++)
    {
        std::cout << "Enabled extension: " << m_creationInfo.ppEnabledExtensionNames[i] << std::endl;
    }

    m_instance = new VkInstance();

    auto result = vkCreateInstance(&m_creationInfo, nullptr, m_instance);

#ifndef NDEBUG
    VkDebugReportCallbackCreateInfoEXT debugReportInfo;
    debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugReportInfo.pNext = nullptr;
    debugReportInfo.flags =
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT;
    debugReportInfo.pfnCallback = &(VulkanValidation::debugReportCallback);
    debugReportInfo.pUserData = nullptr;

    PFN_vkCreateDebugReportCallbackEXT createCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*m_instance, "vkCreateDebugReportCallbackEXT");
    createCallback(*m_instance, &debugReportInfo, nullptr, &m_debugReportCallback);
#endif

    // Setup logical devices
    this->setupGPUs();
    this->printGPUs();
}

void
VulkanRenderer::initialize(const unsigned int width,
                           const unsigned int height,
                           const unsigned int windowWidth,
                           const unsigned int windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    m_height = height;
    m_width = width;

    auto camera = m_scene->getCamera();
    m_fov = (float)glm::radians(camera->getFieldOfView());

    // Setup command pool(s) - right now we just have one
    this->setupCommandPools();
    this->buildCommandBuffer();
    this->setupRenderPasses();
    this->setupSynchronization();
    this->setupMemoryManager();
    this->createGlobalUniformBuffers();
    this->createShadowMaps(m_shadowMapResolution);

    std::vector<VkPipeline> graphicsPipelines;
    std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelinesInfo;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_renderPhysicalDevice, &deviceProperties);
    m_deviceLimits = deviceProperties.limits;

    m_anisotropyAmount = m_deviceLimits.maxSamplerAnisotropy;

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.pNext = nullptr;
    pipelineCacheCreateInfo.flags = 0;
    pipelineCacheCreateInfo.initialDataSize = 0;
    pipelineCacheCreateInfo.pInitialData = nullptr;

    vkCreatePipelineCache(m_renderDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);

    this->setupGUI();
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
    queueInfo.queueCount = 1;// m_queueFamilyProperties[m_renderQueueFamily].queueCount;
    queueInfo.pQueuePriorities = &priorities[0];

    // The display system isn't part of the Vulkan core
    std::vector<std::string> deviceExtensions;
    deviceExtensions.push_back(std::string(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
    m_extensions.push_back(std::string(VK_KHR_MULTIVIEW_EXTENSION_NAME));
    m_extensions.push_back(std::string(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME));

#ifdef iMSTK_ENABLE_VR
    if (m_VRMode)
    {
        uint32_t numBytes = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(m_physicalDevices[0], nullptr, 0);
        std::vector<char> vrDeviceExtensions(numBytes);
        vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(m_physicalDevices[0], &vrDeviceExtensions[0], numBytes);
        std::string tempVRExtensionString(vrDeviceExtensions.begin(), vrDeviceExtensions.end());
        std::istringstream vrDeviceExtensionsStream(tempVRExtensionString);
        std::string tempVRDeviceExtension;

        while (std::getline(vrDeviceExtensionsStream, tempVRDeviceExtension, ' '))
        {
            deviceExtensions.push_back(tempVRDeviceExtension.c_str());
        }
    }
#endif

    std::vector<const char *> finalDeviceExtensions;

    for (auto extension : deviceExtensions)
    {
        auto temp = new char[extension.size() + 1];
        memcpy(temp, extension.c_str(), extension.size() + 1);
        finalDeviceExtensions.push_back(temp);
    }

    // Enabling optional Vulkan features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(m_physicalDevices[0], &deviceFeatures);

    VkPhysicalDeviceFeatures features = {VK_FALSE};
    features.fillModeNonSolid = VK_TRUE;
    features.tessellationShader = VK_TRUE;
    features.samplerAnisotropy = VK_TRUE;
    features.wideLines = deviceFeatures.wideLines;

    if (features.wideLines == VK_TRUE)
    {
        m_supportsWideLines = true;
    }

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
    deviceInfo.ppEnabledExtensionNames = &finalDeviceExtensions[0];
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

    // Create command pools (only one for now)
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = 0;

    vkCreateCommandPool(m_renderDevice, &commandPoolInfo, nullptr, &m_postProcessingCommandPool);
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
    commandBufferInfo.commandBufferCount = m_buffering;

    m_renderCommandBuffer.resize(m_buffering);
    vkAllocateCommandBuffers(m_renderDevice, &commandBufferInfo, &m_renderCommandBuffer[0]);

    commandBufferInfo.commandPool = m_postProcessingCommandPool;
    m_postProcessingCommandBuffer.resize(m_buffering);
    vkAllocateCommandBuffers(m_renderDevice, &commandBufferInfo, &m_postProcessingCommandBuffer[0]);
}

void
VulkanRenderer::setupRenderPasses()
{
    // Number of geometry passes
    VulkanRenderPassGenerator::generateOpaqueRenderPass(m_renderDevice, m_opaqueRenderPass, m_samples, m_numViews);
    VulkanRenderPassGenerator::generateDecalRenderPass(m_renderDevice, m_decalRenderPass, m_samples, m_numViews);
    VulkanRenderPassGenerator::generateParticleRenderPass(m_renderDevice, m_particleRenderPass, m_samples, m_numViews);
    VulkanRenderPassGenerator::generateDepthRenderPass(m_renderDevice, m_depthRenderPass, m_samples, m_numViews);
    VulkanRenderPassGenerator::generateGUIRenderPass(m_renderDevice, m_GUIRenderPass, m_samples, m_numViews);
}

void
VulkanRenderer::resizeFramebuffers(VkSwapchainKHR * swapchain, int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;

    this->deleteFramebuffers();

    this->initializeFramebuffers(swapchain);

    //std::vector<VkPipeline> pipelines;
    //std::vector<VkGraphicsPipelineCreateInfo> pipelineInfos;

    //for (int i = 0; i < m_renderDelegates.size(); i++)
    //{
    //auto material = m_renderDelegates[i]->m_material;
    //vkDestroyPipeline(m_renderDevice, material->m_pipeline, nullptr);
    //material->initialize(this);
    //}
}

void
VulkanRenderer::initializeFramebufferImages(VkSwapchainKHR * swapchain)
{
    m_mipLevels = std::log2(std::max(m_width, m_height)) + 1;

    m_swapchain = swapchain;
    vkGetSwapchainImagesKHR(m_renderDevice, *m_swapchain, &m_swapchainImageCount, nullptr);

    // Depth image
    VkImageCreateInfo depthImageInfo;
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.pNext = nullptr;
    depthImageInfo.flags = 0;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = VulkanFormats::DEPTH_FORMAT;
    depthImageInfo.extent = { m_width, m_height, 1 };
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = m_numViews;
    depthImageInfo.samples = m_samples;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           | VK_IMAGE_USAGE_SAMPLED_BIT;
    depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthImageInfo.queueFamilyIndexCount = 0;
    depthImageInfo.pQueueFamilyIndices = nullptr;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_depthImage.resize(m_mipLevels);
    m_depthImage[0] = m_memoryManager.requestImage(m_renderDevice,
                depthImageInfo,
                VulkanMemoryType::FRAMEBUFFER);

    for (uint32_t i = 1; i < m_mipLevels; i++)
    {
        depthImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depthImageInfo.format = VulkanFormats::DEPTH_MIP_FORMAT;
        depthImageInfo.extent = { std::max(m_width >> i, 1u), std::max(m_height >> i, 1u), 1 };
        m_depthImage[i] = m_memoryManager.requestImage(m_renderDevice,
                        depthImageInfo,
                        VulkanMemoryType::FRAMEBUFFER);
    }

    // Normal image
    auto normalImageInfo = depthImageInfo;
    normalImageInfo.extent = { m_width, m_height, 1 };
    normalImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT;
    normalImageInfo.format = VulkanFormats::NORMAL_SSS_FORMAT;

    m_normalImage = m_memoryManager.requestImage(m_renderDevice,
                normalImageInfo,
                VulkanMemoryType::FRAMEBUFFER);

    // HDR image
    auto HDRImageInfo = depthImageInfo;
    HDRImageInfo.format = VulkanFormats::HDR_FORMAT;
    HDRImageInfo.mipLevels = 1;
    HDRImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                         | VK_IMAGE_USAGE_SAMPLED_BIT;

    m_HDRImage[0].resize(m_mipLevels);
    m_HDRImage[1].resize(m_mipLevels);
    m_HDRImage[2].resize(m_mipLevels);
    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        HDRImageInfo.extent = { std::max(m_width >> i, 1u), std::max(m_height >> i, 1u), 1 };
        m_HDRImage[0][i] = m_memoryManager.requestImage(m_renderDevice, HDRImageInfo, VulkanMemoryType::FRAMEBUFFER);
        m_HDRImage[1][i] = m_memoryManager.requestImage(m_renderDevice, HDRImageInfo, VulkanMemoryType::FRAMEBUFFER);
        m_HDRImage[2][i] = m_memoryManager.requestImage(m_renderDevice, HDRImageInfo, VulkanMemoryType::FRAMEBUFFER);
    }

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_downSample.resize(m_swapchainImageCount);
    }

    // AO image
    auto AOImageInfo = depthImageInfo;
    AOImageInfo.format = VulkanFormats::AO_FORMAT;
    AOImageInfo.mipLevels = 1;
    AOImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                        | VK_IMAGE_USAGE_SAMPLED_BIT;
    AOImageInfo.extent = { m_width / 2, m_height / 2, 1 };

    m_halfAOImage[0] = m_memoryManager.requestImage(m_renderDevice, AOImageInfo, VulkanMemoryType::FRAMEBUFFER);
    m_halfAOImage[1] = m_memoryManager.requestImage(m_renderDevice, AOImageInfo, VulkanMemoryType::FRAMEBUFFER);

    // LDR image
    auto LDRImageInfo = depthImageInfo;
    LDRImageInfo.format = VulkanFormats::FINAL_FORMAT;
    LDRImageInfo.mipLevels = 1;
    LDRImageInfo.arrayLayers = 1;
    LDRImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                         | VK_IMAGE_USAGE_SAMPLED_BIT
                         | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                         | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    LDRImageInfo.extent = { m_width, m_height, 1 };

    m_LDRImage[0] = m_memoryManager.requestImage(m_renderDevice, LDRImageInfo, VulkanMemoryType::FRAMEBUFFER);
    m_LDRImage[1] = m_memoryManager.requestImage(m_renderDevice, LDRImageInfo, VulkanMemoryType::FRAMEBUFFER);

    // Create image views
    m_depthImageView.resize(m_mipLevels);

    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = i == 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = m_numViews;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = *m_depthImage[i]->getImage();
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        imageViewInfo.format = i == 0 ? VulkanFormats::DEPTH_FORMAT : VulkanFormats::DEPTH_MIP_FORMAT;

        imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_depthImageView[i]);
    }

    {
        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = m_numViews;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = *m_normalImage->getImage();
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewInfo.format = VulkanFormats::NORMAL_SSS_FORMAT;
        imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_normalImageView);
    }

    {
        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = m_numViews;

        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.flags = 0;
        imageViewInfo.image = *m_halfAOImage[0]->getImage();
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewInfo.format = VulkanFormats::AO_FORMAT;
        imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_halfAOImageView[0]);

        imageViewInfo.image = *m_halfAOImage[1]->getImage();
        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_halfAOImageView[1]);
    }

    {
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
        imageViewInfo.image = *m_LDRImage[0]->getImage();
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VulkanFormats::FINAL_FORMAT;
        imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_LDRImageView[0]);

        imageViewInfo.image = *m_LDRImage[1]->getImage();
        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_LDRImageView[1]);
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

        for (uint32_t j = 0; j < m_mipLevels; j++)
        {
            VkImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = m_numViews;

            VkImageViewCreateInfo imageViewInfo;
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.pNext = nullptr;
            imageViewInfo.flags = 0;
            imageViewInfo.image = *m_HDRImage[i][j]->getImage();
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            imageViewInfo.format = VulkanFormats::HDR_FORMAT;
            imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
            imageViewInfo.subresourceRange = subresourceRange;

            vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_HDRImageView[i][j]);
        }
    }
}

void
VulkanRenderer::initializeFramebuffers(VkSwapchainKHR * swapchain)
{
    // Get images from surface (color images)
    m_swapchain = swapchain;
    vkGetSwapchainImagesKHR(m_renderDevice, *m_swapchain, &m_swapchainImageCount, nullptr);
    m_swapchainNativeImages.resize(m_swapchainImageCount);
    m_swapchainImages.resize(m_swapchainImageCount);
    vkGetSwapchainImagesKHR(m_renderDevice, *m_swapchain, &m_swapchainImageCount, &m_swapchainNativeImages[0]);
    m_swapchainImageViews.resize(m_swapchainImageCount);

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_swapchainImages[i] = new VulkanInternalImage(&m_swapchainNativeImages[i]);

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
        imageViewInfo.image = *m_swapchainImages[i]->getImage();
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VulkanFormats::FINAL_FORMAT;
        imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
        imageViewInfo.subresourceRange = subresourceRange;

        vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_swapchainImageViews[i]);
    }

    {
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

        vkCreateSampler(m_renderDevice, &samplerInfo, nullptr, &m_swapchainImageSampler);
    }

    this->initializePostProcesses();

    m_opaqueFramebuffer =
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, m_samples);
    m_opaqueFramebuffer->setColor(m_HDRImage[0][0], &m_HDRImageView[0][0], VulkanFormats::HDR_FORMAT);
    m_opaqueFramebuffer->setSpecular(m_HDRImage[1][0], &m_HDRImageView[1][0], VulkanFormats::HDR_FORMAT);
    m_opaqueFramebuffer->setDepth(m_depthImage[0], &m_depthImageView[0], VulkanFormats::DEPTH_FORMAT);
    m_opaqueFramebuffer->setNormal(m_normalImage, &m_normalImageView, VulkanFormats::FINAL_FORMAT);
    m_opaqueFramebuffer->initializeFramebuffer(&m_opaqueRenderPass);

    m_decalFramebuffer =
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, m_samples);
    m_decalFramebuffer->setColor(m_HDRImage[0][0], &m_HDRImageView[0][0], VulkanFormats::HDR_FORMAT);
    m_decalFramebuffer->setSpecular(m_HDRImage[1][0], &m_HDRImageView[1][0], VulkanFormats::HDR_FORMAT);
    m_decalFramebuffer->setDepth(m_depthImage[0], &m_depthImageView[0], VulkanFormats::DEPTH_FORMAT);
    m_decalFramebuffer->initializeFramebuffer(&m_decalRenderPass);

    m_particleFramebuffer =
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, m_samples);
    m_particleFramebuffer->setColor(m_HDRImage[0][0], &m_HDRImageView[0][0], VulkanFormats::HDR_FORMAT);
    m_particleFramebuffer->setSpecular(m_HDRImage[1][0], &m_HDRImageView[1][0], VulkanFormats::HDR_FORMAT);
    m_particleFramebuffer->setDepth(m_depthImage[0], &m_depthImageView[0], VulkanFormats::DEPTH_FORMAT);
    m_particleFramebuffer->initializeFramebuffer(&m_particleRenderPass);

    m_depthFramebuffer =
        std::make_shared<VulkanFramebuffer>(m_memoryManager, m_width, m_height, m_samples);
    m_depthFramebuffer->setDepth(m_depthImage[0], &m_depthImageView[0], VulkanFormats::DEPTH_FORMAT);
    m_depthFramebuffer->initializeFramebuffer(&m_depthRenderPass);
}

void
VulkanRenderer::deleteFramebuffers()
{
    // The framebuffers/command buffers may still be in use
    vkDeviceWaitIdle(m_renderDevice);

    // Depth buffer
    for (uint32_t i = 0; i < m_mipLevels; i++)
    {
        vkDestroyImageView(m_renderDevice, m_depthImageView[i], nullptr);
    }

    // LDR buffers
    for (int i = 0; i < 2; i++)
    {
        vkDestroyImageView(m_renderDevice, m_LDRImageView[i], nullptr);
    }

    // HDR buffers
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < m_HDRImageView[i].size(); j++)
        {
            vkDestroyImageView(m_renderDevice, m_HDRImageView[i][j], nullptr);
        }
    }

    // Normal buffer
    vkDestroyImageView(m_renderDevice, m_normalImageView, nullptr);

    // AO buffer
    vkDestroyImageView(m_renderDevice, m_halfAOImageView[0], nullptr);
    vkDestroyImageView(m_renderDevice, m_halfAOImageView[1], nullptr);

    for (auto imageView : m_swapchainImageViews)
    {
        vkDestroyImageView(m_renderDevice, imageView, nullptr);
    }

    // Delete all post processing resources
    for (auto postProcess : m_postProcessingChain->m_postProcesses)
    {
        postProcess->m_framebuffer->clear(&m_renderDevice);
    }

    // Delete all HDR resources
    for (auto postProcess : m_HDRTonemaps)
    {
        postProcess->m_framebuffer->clear(&m_renderDevice);
    }

    // Delete all downsample resources
    for (auto pass : m_downSample)
    {
        pass->m_framebuffer->clear(&m_renderDevice);
    }

    // Delete all AO resources
    for (auto pass : m_ssao)
    {
        pass->m_framebuffer->clear(&m_renderDevice);
    }

    // Delete all drawing resources
    vkDestroyFramebuffer(m_renderDevice, m_opaqueFramebuffer->m_framebuffer, nullptr);
    vkDestroyFramebuffer(m_renderDevice, m_depthFramebuffer->m_framebuffer, nullptr);
    vkDestroyFramebuffer(m_renderDevice, m_decalFramebuffer->m_framebuffer, nullptr);
    vkDestroyFramebuffer(m_renderDevice, m_particleFramebuffer->m_framebuffer, nullptr);
}

void
VulkanRenderer::renderFrame()
{
    m_frameNumber++;

    // The swapchain contains multiple buffers, so get one that is available (i.e., not currently being written to)
    uint32_t nextImageIndex;
    vkAcquireNextImageKHR(m_renderDevice, *m_swapchain, UINT64_MAX, m_readyToRender, VK_NULL_HANDLE, &nextImageIndex);

    this->loadAllVisualModels();

    for (auto sceneObject : m_scene->getSceneObjects())
    {
        if (sceneObject->getType() == SceneObject::Type::Animation)
        {
            auto animatedObject = std::static_pointer_cast<AnimationObject>(sceneObject);
            animatedObject->getAnimationModel()->update();
        }
    }

    // Update global uniforms
    this->updateGlobalUniforms(nextImageIndex);

    // Update local uniforms
    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::DecalPool)
        {
            auto decalPool = std::dynamic_pointer_cast<VulkanDecalRenderDelegate>(m_renderDelegates[renderDelegateIndex]);
            decalPool->update(nextImageIndex, m_scene->getCamera());
        }
        else if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::RenderParticles)
        {
            auto particleEmitter = std::dynamic_pointer_cast<VulkanParticleRenderDelegate>(m_renderDelegates[renderDelegateIndex]);
            particleEmitter->update(nextImageIndex, m_scene->getCamera());
        }
        m_renderDelegates[renderDelegateIndex]->update(nextImageIndex);
    }

    // Wait until command buffer is done so that we can write to it again
    vkWaitForFences(m_renderDevice, 1, &m_commandBufferSubmit[nextImageIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(m_renderDevice, 1, &m_commandBufferSubmit[nextImageIndex]);

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(m_renderCommandBuffer[nextImageIndex], &commandBufferBeginInfo);

    if (m_frameNumber == 1)
    {
        this->initializeFramebufferAttachments(&m_renderCommandBuffer[nextImageIndex]);
    }

    VkRect2D renderArea;
    renderArea.offset = { 0, 0 };
    renderArea.extent = { m_width, m_height };

    std::array<VkClearValue, 4> clearValues;
    clearValues[0].color = { { (float)m_backgroundColor[0], (float)m_backgroundColor[1], (float)m_backgroundColor[2], 1 } }; // Color
    clearValues[1].depthStencil = { { 1.0 }, { 0 } }; // Depth
    clearValues[2].color = { { 0, 0, 0, 0 } }; // Normal
    clearValues[3].color = { { 0, 0, 0, 0 } }; // Specular

    // Do buffer transfers
    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();
        buffers->uploadBuffers(m_renderCommandBuffer[nextImageIndex]);
    }
    VkMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;

    vkCmdPipelineBarrier(m_renderCommandBuffer[nextImageIndex],
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0,
        1,
        &barrier,
        0,
        nullptr,
        0,
        nullptr);

    VkDeviceSize deviceSize = { 0 };

    // Render pass: opaque shadows
    for (size_t i = 0; i < m_shadowPasses.size(); i++)
    {
        VkRect2D shadowRenderArea = { {0, 0}, {m_shadowMapResolution, m_shadowMapResolution} };

        VkRenderPassBeginInfo shadowPassBeginInfo;
        shadowPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        shadowPassBeginInfo.pNext = nullptr;
        shadowPassBeginInfo.renderPass = m_shadowPasses[i];
        shadowPassBeginInfo.framebuffer = m_shadowFramebuffers[i]->m_framebuffer;
        shadowPassBeginInfo.renderArea = shadowRenderArea;
        shadowPassBeginInfo.clearValueCount = 1;
        shadowPassBeginInfo.pClearValues = &clearValues[1]; // depth buffer

        vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &shadowPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
        {
            auto material = m_renderDelegates[renderDelegateIndex]->m_shadowMaterial;

            if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::DecalPool
                || m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::RenderParticles
                || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->getRenderMaterial()->getCastsShadows()
                || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->isVisible())
            {
                continue;
            }

            vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);

            vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
                &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

            auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

            vkCmdPushConstants(m_renderCommandBuffer[nextImageIndex], material->m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, (void*)&m_lightMatrices[i]);
            buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], nextImageIndex);
            vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);
        m_shadowMaps->setImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        VulkanAttachmentBarriers::changeImageLayout(&m_renderCommandBuffer[nextImageIndex],
            m_renderQueueFamily,
            m_shadowMaps,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            1);
    }

    // Render pass: depth pre-pass
    VkRenderPassBeginInfo depthRenderPassBeginInfo;
    depthRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    depthRenderPassBeginInfo.pNext = nullptr;
    depthRenderPassBeginInfo.renderPass = m_depthRenderPass;
    depthRenderPassBeginInfo.framebuffer = m_depthFramebuffer->m_framebuffer;
    depthRenderPassBeginInfo.renderArea = renderArea;
    depthRenderPassBeginInfo.clearValueCount = 1;
    depthRenderPassBeginInfo.pClearValues = &clearValues[1];

    vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &depthRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::DecalPool
            || m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::RenderParticles
            || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->isVisible())
        {
            continue;
        }

        auto material = m_renderDelegates[renderDelegateIndex]->m_depthMaterial;
        vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);
        this->setCommandBufferState(&m_renderCommandBuffer[nextImageIndex], m_width, m_height);

        vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], nextImageIndex);
        vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);
    m_depthImage[0]->setImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    VulkanAttachmentBarriers::changeImageLayout(&m_renderCommandBuffer[nextImageIndex],
        m_renderQueueFamily,
        m_depthImage[0],
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        m_numViews);

    // Render passes: AO processing
    VkRenderPassBeginInfo aoRenderPassBeginInfo;
    aoRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    aoRenderPassBeginInfo.pNext = nullptr;
    aoRenderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    aoRenderPassBeginInfo.pClearValues = &clearValues[0];

    for (unsigned int postProcessIndex = 0; postProcessIndex < m_ssao.size(); postProcessIndex++)
    {
        auto postProcess = m_ssao[postProcessIndex];

        auto framebuffer = postProcess->m_framebuffer;
        aoRenderPassBeginInfo.renderPass =
            postProcess->m_renderPass;
        aoRenderPassBeginInfo.framebuffer = framebuffer->m_framebuffer;
        aoRenderPassBeginInfo.clearValueCount = (uint32_t)framebuffer->m_attachments.size();
        aoRenderPassBeginInfo.renderArea.offset = { 0, 0 };
        aoRenderPassBeginInfo.renderArea.extent = { framebuffer->m_width, framebuffer->m_height };

        vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &aoRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdPushConstants(m_renderCommandBuffer[nextImageIndex], postProcess->m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128, (void*)postProcess->m_pushConstantData);

        vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipeline);
        this->setCommandBufferState(&m_renderCommandBuffer[nextImageIndex], framebuffer->m_width, framebuffer->m_height);

        vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipelineLayout, 0,
            (uint32_t)postProcess->m_descriptorSets.size(),
            &postProcess->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = postProcess->m_vertexBuffer;
        buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], 0);
        vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);

        postProcess->updateImageLayouts();
        postProcess->setAttachmentsToReadLayout(&m_renderCommandBuffer[nextImageIndex], m_renderQueueFamily, m_numViews);
    }

    // Render pass: render opaque geometry
    VkRenderPassBeginInfo opaqueRenderPassBeginInfo;
    opaqueRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    opaqueRenderPassBeginInfo.pNext = nullptr;
    opaqueRenderPassBeginInfo.renderPass = m_opaqueRenderPass;
    opaqueRenderPassBeginInfo.framebuffer = m_opaqueFramebuffer->m_framebuffer;
    opaqueRenderPassBeginInfo.renderArea = renderArea;
    opaqueRenderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    opaqueRenderPassBeginInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &opaqueRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::DecalPool
            || m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() == Geometry::Type::RenderParticles
            || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->isVisible())
        {
            continue;
        }

        auto material = m_renderDelegates[renderDelegateIndex]->m_material;
        vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);
        this->setCommandBufferState(&m_renderCommandBuffer[nextImageIndex], m_width, m_height);

        vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], nextImageIndex);
        vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);

    m_normalImage->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VulkanAttachmentBarriers::changeImageLayout(&m_renderCommandBuffer[nextImageIndex],
        m_renderQueueFamily,
        m_normalImage,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        m_numViews);

    // Render pass: render decals
    VkRenderPassBeginInfo decalRenderPassBeginInfo;
    decalRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    decalRenderPassBeginInfo.pNext = nullptr;
    decalRenderPassBeginInfo.renderPass = m_decalRenderPass;
    decalRenderPassBeginInfo.framebuffer = m_decalFramebuffer->m_framebuffer;
    decalRenderPassBeginInfo.renderArea = renderArea;
    decalRenderPassBeginInfo.clearValueCount = 0;
    decalRenderPassBeginInfo.pClearValues = &clearValues[0];
    vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &decalRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() != Geometry::Type::DecalPool
            || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->isVisible())
        {
            continue;
        }

        auto geometry = std::dynamic_pointer_cast<DecalPool>(m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry());
        auto material = m_renderDelegates[renderDelegateIndex]->m_material;
        vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);
        this->setCommandBufferState(&m_renderCommandBuffer[nextImageIndex], m_width, m_height);

        vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], nextImageIndex);
        vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, geometry->getNumDecals(), 0, 0, 0);
    }

    vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);

    // Render pass: render particles
    VkRenderPassBeginInfo particleRenderPassBeginInfo;
    particleRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    particleRenderPassBeginInfo.pNext = nullptr;
    particleRenderPassBeginInfo.renderPass = m_particleRenderPass;
    particleRenderPassBeginInfo.framebuffer = m_particleFramebuffer->m_framebuffer;
    particleRenderPassBeginInfo.renderArea = renderArea;
    particleRenderPassBeginInfo.clearValueCount = 0;
    particleRenderPassBeginInfo.pClearValues = &clearValues[0];
    vkCmdBeginRenderPass(m_renderCommandBuffer[nextImageIndex], &particleRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (unsigned int renderDelegateIndex = 0; renderDelegateIndex < m_renderDelegates.size(); renderDelegateIndex++)
    {
        if (m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry()->getType() != Geometry::Type::RenderParticles
            || !m_renderDelegates[renderDelegateIndex]->getVisualModel()->isVisible())
        {
            continue;
        }

        auto geometry = std::dynamic_pointer_cast<RenderParticles>(m_renderDelegates[renderDelegateIndex]->getVisualModel()->getGeometry());
        auto material = m_renderDelegates[renderDelegateIndex]->m_material;
        vkCmdBindPipeline(m_renderCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_pipeline);
        this->setCommandBufferState(&m_renderCommandBuffer[nextImageIndex], m_width, m_height);

        vkCmdBindDescriptorSets(m_renderCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->m_pipelineLayout, 0, (uint32_t)material->m_descriptorSets.size(),
            &material->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_renderDelegates[renderDelegateIndex]->getBuffer().get();

        buffers->bindBuffers(&m_renderCommandBuffer[nextImageIndex], nextImageIndex);
        vkCmdDrawIndexed(m_renderCommandBuffer[nextImageIndex], buffers->m_numIndices, geometry->getNumParticles(), 0, 0, 0);
    }

    vkCmdEndRenderPass(m_renderCommandBuffer[nextImageIndex]);
    m_HDRImage[0][0]->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_HDRImage[1][0]->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VulkanAttachmentBarriers::changeImageLayout(&m_renderCommandBuffer[nextImageIndex],
        m_renderQueueFamily,
        m_HDRImage[0][0],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        m_numViews);

    VulkanAttachmentBarriers::changeImageLayout(&m_renderCommandBuffer[nextImageIndex],
        m_renderQueueFamily,
        m_HDRImage[1][0],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        m_numViews);

    vkEndCommandBuffer(m_renderCommandBuffer[nextImageIndex]);

    vkBeginCommandBuffer(m_postProcessingCommandBuffer[nextImageIndex], &commandBufferBeginInfo);

    // Render passes: post processing
    for (unsigned int postProcessIndex = 0; postProcessIndex < m_postProcessingChain->m_postProcesses.size(); postProcessIndex++)
    {
        clearValues[0].color = { { 1.0, 0.0, 0.0, 1 } }; // Color

        auto postProcess = m_postProcessingChain->m_postProcesses[postProcessIndex];

        auto postProcessRenderPassBeginInfo = opaqueRenderPassBeginInfo;
        auto framebuffer = postProcess->m_framebuffer;
        postProcessRenderPassBeginInfo.renderPass =
            postProcess->m_renderPass;
        postProcessRenderPassBeginInfo.framebuffer = framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)framebuffer->m_attachments.size();
        postProcessRenderPassBeginInfo.renderArea.extent = { framebuffer->m_width, framebuffer->m_height };

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer[nextImageIndex], &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdPushConstants(m_postProcessingCommandBuffer[nextImageIndex], postProcess->m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128, (void*)postProcess->m_pushConstantData);

        vkCmdBindPipeline(m_postProcessingCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipeline);
        this->setCommandBufferState(&m_postProcessingCommandBuffer[nextImageIndex], framebuffer->m_width, framebuffer->m_height);

        vkCmdBindDescriptorSets(m_postProcessingCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            postProcess->m_pipelineLayout, 0,
            (uint32_t)postProcess->m_descriptorSets.size(),
            &postProcess->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = postProcess->m_vertexBuffer;
        buffers->bindBuffers(&m_postProcessingCommandBuffer[nextImageIndex], 0);
        vkCmdDrawIndexed(m_postProcessingCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_postProcessingCommandBuffer[nextImageIndex]);

        postProcess->updateImageLayouts();
        postProcess->setAttachmentsToReadLayout(&m_postProcessingCommandBuffer[nextImageIndex],
            m_renderQueueFamily,
            m_numViews);
    }

    // Render pass: HDR tonemap
    for (unsigned int i = 0; i < m_HDRTonemaps.size(); i++)
    {
        auto postProcessRenderPassBeginInfo = opaqueRenderPassBeginInfo;
        postProcessRenderPassBeginInfo.renderArea.extent = {m_HDRTonemaps[i]->m_framebuffer->m_width, m_HDRTonemaps[i]->m_framebuffer->m_height};
        postProcessRenderPassBeginInfo.renderPass = m_HDRTonemaps[i]->m_renderPass;
        postProcessRenderPassBeginInfo.framebuffer = m_HDRTonemaps[i]->m_framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)m_HDRTonemaps[i]->m_framebuffer->m_attachments.size();

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer[nextImageIndex], &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_postProcessingCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_HDRTonemaps[i]->m_pipeline);
        this->setCommandBufferState(&m_postProcessingCommandBuffer[nextImageIndex],
            m_HDRTonemaps[i]->m_framebuffer->m_width,
            m_HDRTonemaps[i]->m_framebuffer->m_height);

        vkCmdBindDescriptorSets(m_postProcessingCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_HDRTonemaps[i]->m_pipelineLayout, 0, (uint32_t)m_HDRTonemaps[i]->m_descriptorSets.size(),
            &m_HDRTonemaps[i]->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_HDRTonemaps[i]->m_vertexBuffer;
        buffers->bindBuffers(&m_postProcessingCommandBuffer[nextImageIndex], 0);
        vkCmdPushConstants(m_postProcessingCommandBuffer[nextImageIndex], m_HDRTonemaps[i]->m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128, (void*)m_HDRTonemaps[i]->m_pushConstantData);

        vkCmdDrawIndexed(m_postProcessingCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_postProcessingCommandBuffer[nextImageIndex]);

        m_HDRTonemaps[i]->updateImageLayouts();
        m_HDRTonemaps[i]->setAttachmentsToReadLayout(&m_postProcessingCommandBuffer[nextImageIndex],
            m_renderQueueFamily,
            1);
    }

    // Render pass: downsample (this is special because of the swapchain)
    {
        auto postProcessRenderPassBeginInfo = opaqueRenderPassBeginInfo;
        postProcessRenderPassBeginInfo.renderArea.extent = {m_windowWidth, m_windowHeight};
        postProcessRenderPassBeginInfo.renderPass = m_downSample[nextImageIndex]->m_renderPass;
        postProcessRenderPassBeginInfo.framebuffer = m_downSample[nextImageIndex]->m_framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)m_downSample[nextImageIndex]->m_framebuffer->m_attachments.size();

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer[nextImageIndex], &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_postProcessingCommandBuffer[nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_downSample[nextImageIndex]->m_pipeline);
        this->setCommandBufferState(&m_postProcessingCommandBuffer[nextImageIndex],
            m_downSample[nextImageIndex]->m_framebuffer->m_width,
            m_downSample[nextImageIndex]->m_framebuffer->m_height);

        vkCmdBindDescriptorSets(m_postProcessingCommandBuffer[nextImageIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_downSample[nextImageIndex]->m_pipelineLayout, 0, (uint32_t)m_downSample[nextImageIndex]->m_descriptorSets.size(),
            &m_downSample[nextImageIndex]->m_descriptorSets[0], 0, &m_dynamicOffsets);

        auto buffers = m_downSample[nextImageIndex]->m_vertexBuffer;
        buffers->bindBuffers(&m_postProcessingCommandBuffer[nextImageIndex], 0);
        vkCmdPushConstants(m_postProcessingCommandBuffer[nextImageIndex], m_downSample[nextImageIndex]->m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128, (void*)m_downSample[nextImageIndex]->m_pushConstantData);

        vkCmdDrawIndexed(m_postProcessingCommandBuffer[nextImageIndex], buffers->m_numIndices, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_postProcessingCommandBuffer[nextImageIndex]);
        m_downSample[nextImageIndex]->updateImageLayouts();
    }

    // Render pass: GUI
    {
        auto postProcessRenderPassBeginInfo = opaqueRenderPassBeginInfo;
        postProcessRenderPassBeginInfo.renderArea.extent = {m_windowWidth, m_windowHeight};
        postProcessRenderPassBeginInfo.renderPass = m_GUIRenderPass;
        postProcessRenderPassBeginInfo.framebuffer = m_downSample[nextImageIndex]->m_framebuffer->m_framebuffer;
        postProcessRenderPassBeginInfo.clearValueCount = (uint32_t)m_downSample[nextImageIndex]->m_framebuffer->m_attachments.size();

        vkCmdBeginRenderPass(m_postProcessingCommandBuffer[nextImageIndex], &postProcessRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_postProcessingCommandBuffer[nextImageIndex]);
        vkCmdEndRenderPass(m_postProcessingCommandBuffer[nextImageIndex]);
    }

    m_swapchainImages[nextImageIndex]->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VulkanAttachmentBarriers::changeImageLayout(&m_postProcessingCommandBuffer[nextImageIndex],
        m_renderQueueFamily,
        m_swapchainImages[nextImageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        1);

    for (unsigned int i = 0; i < m_numViews; i++)
    {
        VulkanAttachmentBarriers::changeImageLayout(&m_postProcessingCommandBuffer[nextImageIndex],
            m_renderQueueFamily,
            m_LDRImage[i],
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            1);
    }

    vkEndCommandBuffer(m_postProcessingCommandBuffer[nextImageIndex]);

    VkCommandBuffer commandBuffers[2];
    commandBuffers[0] = m_renderCommandBuffer[nextImageIndex];
    commandBuffers[1] = m_postProcessingCommandBuffer[nextImageIndex];

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
    vkQueueSubmit(m_renderQueue, 2, submitInfo, m_commandBufferSubmit[nextImageIndex]);

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

#ifdef iMSTK_ENABLE_VR
    if (m_VRMode)
    {
        vr::VRVulkanTextureData_t VRTextureDataLeft;
        VRTextureDataLeft.m_nImage = (uint64_t)*m_LDRImage[0]->getImage();
        VRTextureDataLeft.m_pDevice = m_renderDevice;
        VRTextureDataLeft.m_pPhysicalDevice = m_renderPhysicalDevice;
        VRTextureDataLeft.m_pInstance = *m_instance;
        VRTextureDataLeft.m_pQueue = m_renderQueue;
        VRTextureDataLeft.m_nQueueFamilyIndex = m_renderQueueFamily;
        VRTextureDataLeft.m_nWidth = m_width;
        VRTextureDataLeft.m_nHeight = m_height;
        VRTextureDataLeft.m_nFormat = (uint32_t)VulkanFormats::FINAL_FORMAT;
        VRTextureDataLeft.m_nSampleCount = VK_SAMPLE_COUNT_1_BIT;

        vr::Texture_t VRTextureLeft;
        VRTextureLeft.eColorSpace = vr::EColorSpace::ColorSpace_Auto;
        VRTextureLeft.eType = vr::ETextureType::TextureType_Vulkan;
        VRTextureLeft.handle = (void*)(&VRTextureDataLeft);

        auto errorLeft = vr::VRCompositor()->Submit(vr::Eye_Left, &VRTextureLeft);

        auto VRTextureDataRight = VRTextureDataLeft;
        VRTextureDataRight.m_nImage = (uint64_t)*m_LDRImage[1]->getImage();
        auto VRTextureRight = VRTextureLeft;
        VRTextureRight.handle = (void*)(&VRTextureDataRight);
        auto errorRight = vr::VRCompositor()->Submit(vr::Eye_Right, &VRTextureRight);
    }
#endif

    // Display backbuffer
    vkQueuePresentKHR(m_renderQueue, &presentInfo);
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
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_commandBufferSubmit.resize(m_buffering);
    for (uint32_t i = 0; i < m_buffering; i++)
    {
        vkCreateFence(m_renderDevice, &fenceInfo, nullptr, &m_commandBufferSubmit[i]);
    }
}

void
VulkanRenderer::loadAllVisualModels()
{
    // Add new objects
    for (auto sceneObject : m_scene->getSceneObjects())
    {
        auto type = sceneObject->getType();

        for (auto visualModel : sceneObject->getVisualModels())
        {
            if (visualModel && !visualModel->isRenderDelegateCreated())
            {
                auto renderDelegate = this->loadVisualModel(visualModel, type);
            }
        }
    }
}

std::shared_ptr<VulkanRenderDelegate>
VulkanRenderer::loadVisualModel(std::shared_ptr<VisualModel> visualModel, SceneObject::Type type)
{
    auto renderDelegate = VulkanRenderDelegate::make_delegate(visualModel, type, m_memoryManager);
    if (renderDelegate != nullptr)
    {
        m_renderDelegates.push_back(renderDelegate);
        renderDelegate->getBuffer()->initializeBuffers(m_memoryManager);
        renderDelegate->m_material->initialize(this);

        if (!renderDelegate->getVisualModel()->getRenderMaterial()->isDecal()
            && !renderDelegate->getVisualModel()->getRenderMaterial()->isParticle())
        {
            renderDelegate->m_shadowMaterial->initialize(this);
            renderDelegate->m_depthMaterial->initialize(this);
        }
    }
    return renderDelegate;
}

void
VulkanRenderer::setupMemoryManager()
{
    m_memoryManager.setup(&m_renderPhysicalDevice);
    m_memoryManager.m_device = m_renderDevice;
    m_memoryManager.m_queueFamilyIndex = m_renderQueueFamily;
    m_memoryManager.m_transferCommandBuffer = &m_renderCommandBuffer[0];
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
    m_HDRTonemaps.resize(m_numViews);
    for (uint32_t i = 0; i < m_numViews; i++)
    {
        m_HDRTonemaps[i] = std::make_shared<VulkanPostProcess>(this, 1, m_width, m_height);
        m_HDRTonemaps[i]->addInputImage(&m_HDRImageSampler, &m_HDRImageView[m_postProcessingChain->m_lastOutput][0]);
        m_HDRTonemaps[i]->m_framebuffer->setColor(m_LDRImage[i], &m_LDRImageView[i], VulkanFormats::FINAL_FORMAT);
        m_HDRTonemaps[i]->initialize(this, VulkanShaderPath::PostProcessing + "HDR_tonemap_frag.spv");
        m_HDRTonemaps[i]->m_pushConstantData[0] = (float)i;

        graphicsPipelines.push_back(m_HDRTonemaps[i]->m_pipeline);
        graphicsPipelinesInfo.push_back(m_HDRTonemaps[i]->m_graphicsPipelineInfo);
    }

    // LDR down sample pipeline creation
    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_downSample[i] = std::make_shared<VulkanPostProcess>(this, 1, m_windowWidth, m_windowHeight);
        m_downSample[i]->addInputImage(&m_swapchainImageSampler, &m_LDRImageView[0]);
        m_downSample[i]->addInputImage(&m_swapchainImageSampler, &m_LDRImageView[1]);
        m_downSample[i]->m_framebuffer->setColor(m_swapchainImages[i],
            &m_swapchainImageViews[i],
            VulkanFormats::FINAL_FORMAT);
        m_downSample[i]->initialize(this, VulkanShaderPath::PostProcessing + "vr_composite_frag.spv");
        m_downSample[i]->m_pushConstantData[0] = (float)m_numViews;

        graphicsPipelines.push_back(m_downSample[i]->m_pipeline);
        graphicsPipelinesInfo.push_back(m_downSample[i]->m_graphicsPipelineInfo);
    }

    // AO pipeline creation
    m_ssao.resize(4);

    // Textures
    if (!m_noiseTexture)
    {
        m_noiseTexture = std::make_shared<Texture>("noise", Texture::Type::NONE);
        m_noiseTextureDelegate = std::make_shared<VulkanTextureDelegate>(m_memoryManager, m_noiseTexture, 0.0f);
    }

    m_ssao[0] = std::make_shared<VulkanPostProcess>(this, m_numViews, 1);
    m_ssao[0]->addInputImage(&m_HDRImageSampler, &m_depthImageView[0], VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    m_ssao[0]->m_framebuffer->setColor(m_depthImage[1], &m_depthImageView[1], VulkanFormats::DEPTH_MIP_FORMAT);
    m_ssao[0]->initialize(this, VulkanShaderPath::PostProcessing + "depth_downscale_frag.spv");

    m_ssao[1] = std::make_shared<VulkanPostProcess>(this, m_numViews, 1);
    m_ssao[1]->addInputImage(&m_HDRImageSampler, &m_depthImageView[1]);
    m_ssao[1]->addInputImage(&m_noiseTextureDelegate->m_sampler, &m_noiseTextureDelegate->m_imageView);
    m_ssao[1]->m_framebuffer->setColor(m_halfAOImage[0], &m_halfAOImageView[0], VulkanFormats::AO_FORMAT);
    m_ssao[1]->m_pushConstantData[0] = m_fov;
    m_ssao[1]->m_pushConstantData[1] = 0.1;
    m_ssao[1]->m_pushConstantData[2] = m_nearPlane;
    m_ssao[1]->m_pushConstantData[3] = m_farPlane;
    m_ssao[1]->m_pushConstantData[4] = 6;
    m_ssao[1]->m_pushConstantData[5] = m_width / 2;
    m_ssao[1]->m_pushConstantData[6] = m_height / 2;
    m_ssao[1]->initialize(this, VulkanShaderPath::PostProcessing + "ao_frag.spv");

    m_ssao[2] = std::make_shared<VulkanPostProcess>(this, m_numViews, 1);
    m_ssao[2]->addInputImage(&m_HDRImageSampler, &m_halfAOImageView[0]);
    m_ssao[2]->addInputImage(&m_HDRImageSampler, &m_depthImageView[1]);
    m_ssao[2]->m_framebuffer->setColor(m_halfAOImage[1], &m_halfAOImageView[1], VulkanFormats::AO_FORMAT);
    m_ssao[2]->m_pushConstantData[0] = std::max(m_width >> 1, 1u);
    m_ssao[2]->m_pushConstantData[1] = std::max(m_height >> 1, 1u);
    m_ssao[2]->m_pushConstantData[2] = m_nearPlane;
    m_ssao[2]->m_pushConstantData[3] = m_farPlane;
    m_ssao[2]->m_pushConstantData[4] = 2;
    VulkanPostProcessingChain::calculateBlurValuesLinear(2,
        &m_ssao[2]->m_pushConstantData[5],
        &m_ssao[2]->m_pushConstantData[10]);
    m_ssao[2]->initialize(this, VulkanShaderPath::PostProcessing + "bilateral_blur_horizontal_frag.spv");

    m_ssao[3] = std::make_shared<VulkanPostProcess>(this, m_numViews, 1);
    m_ssao[3]->addInputImage(&m_HDRImageSampler, &m_halfAOImageView[1]);
    m_ssao[3]->addInputImage(&m_HDRImageSampler, &m_depthImageView[1]);
    m_ssao[3]->m_framebuffer->setColor(m_halfAOImage[0], &m_halfAOImageView[0], VulkanFormats::AO_FORMAT);
    m_ssao[3]->m_pushConstantData[0] = std::max(m_width >> 1, 1u);
    m_ssao[3]->m_pushConstantData[1] = std::max(m_height >> 1, 1u);
    m_ssao[3]->m_pushConstantData[2] = m_nearPlane;
    m_ssao[3]->m_pushConstantData[3] = m_farPlane;
    m_ssao[3]->m_pushConstantData[4] = 2;
    VulkanPostProcessingChain::calculateBlurValuesLinear(2,
        &m_ssao[3]->m_pushConstantData[5],
        &m_ssao[3]->m_pushConstantData[10]);
    m_ssao[3]->initialize(this, VulkanShaderPath::PostProcessing + "bilateral_blur_vertical_frag.spv");

    for (int i = 0; i < 4; i++)
    {
        graphicsPipelines.push_back(m_ssao[i]->m_pipeline);
        graphicsPipelinesInfo.push_back(m_ssao[i]->m_graphicsPipelineInfo);
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

    int index = 0;

    for (unsigned int i = 0; i < m_HDRTonemaps.size(); i++)
    {
        m_HDRTonemaps[i]->m_pipeline = graphicsPipelines[index];
        index++;
    }

    for (uint32_t i = 0; i < m_swapchainImageCount; i++)
    {
        m_downSample[i]->m_pipeline = graphicsPipelines[index];
        index++;
    }

    for (uint32_t i = 0; i < 4; i++)
    {
        m_ssao[i]->m_pipeline = graphicsPipelines[index];
        index++;
    }

    for (int i = 0; i < m_postProcessingChain->m_postProcesses.size(); i++)
    {
        m_postProcessingChain->m_postProcesses[i]->m_pipeline = graphicsPipelines[index];
        index++;
    }
}

void
VulkanRenderer::updateGlobalUniforms(uint32_t frameIndex)
{
    // Vertex uniforms
    {
        if (m_VRMode)
        {
            m_globalVertexUniforms.viewMatrices[0] = m_viewMatrices[0];
            m_globalVertexUniforms.viewMatrices[1] = m_viewMatrices[1];

            m_globalVertexUniforms.projectionMatrices[0] = m_projectionMatrices[0];
            m_globalVertexUniforms.projectionMatrices[1] = m_projectionMatrices[1];

            glm::mat4 correctionMatrix; // for Vulkan rendering
            correctionMatrix[1][1] = -1;
            m_globalVertexUniforms.projectionMatrices[0] *= correctionMatrix;
            m_globalVertexUniforms.projectionMatrices[1] *= correctionMatrix;

            m_globalVertexUniforms.cameraPositions[0] = m_cameraPositions[0];
            m_globalVertexUniforms.cameraPositions[1] = m_cameraPositions[1];
        }
        else
        {
            // Projection matrix
            auto camera = m_scene->getCamera();
            m_fov = (float)glm::radians(camera->getFieldOfView());
            m_globalVertexUniforms.projectionMatrices[0] = glm::perspective(m_fov, (float)(m_width) / (float)(m_height), m_nearPlane, m_farPlane);
            glm::mat4 correctionMatrix; // for Vulkan rendering
            correctionMatrix[1][1] = -1;
            m_globalVertexUniforms.projectionMatrices[0] *= correctionMatrix;
            m_globalVertexUniforms.projectionMatrices[1] = m_globalVertexUniforms.projectionMatrices[0];

            // View matrix
            auto eye = glm::tvec3<float>(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z());
            auto center = glm::tvec3<float>(camera->getFocalPoint().x(), camera->getFocalPoint().y(), camera->getFocalPoint().z());
            auto up = glm::tvec3<float>(camera->getViewUp().x(), camera->getViewUp().y(), camera->getViewUp().z());
            m_globalVertexUniforms.cameraPositions[0] = glm::vec4(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z(), 0.0);
            m_globalVertexUniforms.cameraPositions[1] = m_globalVertexUniforms.cameraPositions[0];
            m_globalVertexUniforms.viewMatrices[0] = glm::lookAt(eye, center, up);
            m_globalVertexUniforms.viewMatrices[1] = m_globalVertexUniforms.viewMatrices[0];
        }
    }

    // Lights uniforms
    {
        // Update lights - need to optimize
        auto lights = m_scene->getLights();
        for (int i = 0; i < lights.size(); i++)
        {
            // Only supports directional lights right now
            auto focalPoint = lights[i]->getFocalPoint();
            auto position = Vec3f(0, 0, 0);
            int type = 1;
            int shadowMapIndex = -1;

            if (lights[i]->getType() == LightType::POINT_LIGHT || lights[i]->getType() == LightType::SPOT_LIGHT)
            {
                position = std::static_pointer_cast<PointLight>(lights[i])->getPosition();
                type = 2;
            }

            m_globalFragmentUniforms.lights[i].position = glm::vec4(position.x(), position.y(), position.z(), 1);

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

            if (lights[i]->getType() == LightType::DIRECTIONAL_LIGHT)
            {
                shadowMapIndex = std::static_pointer_cast<DirectionalLight>(lights[i])->m_shadowMapIndex;
            }

            m_globalFragmentUniforms.lights[i].color.a = lights[i]->getIntensity();

            m_globalFragmentUniforms.lights[i].state.x = type;
            m_globalFragmentUniforms.lights[i].state.y = shadowMapIndex;
        }

        memcpy(&m_globalVertexUniforms.lights, &m_globalFragmentUniforms.lights, sizeof(m_globalFragmentUniforms.lights));

        m_globalFragmentUniforms.inverseViewMatrices[0] = glm::inverse(m_globalVertexUniforms.viewMatrices[0]);
        m_globalFragmentUniforms.inverseProjectionMatrices[0] = glm::inverse(m_globalVertexUniforms.projectionMatrices[0]);

        if (m_VRMode)
        {
            m_globalFragmentUniforms.inverseViewMatrices[1] = glm::inverse(m_globalVertexUniforms.viewMatrices[1]);
            m_globalFragmentUniforms.inverseProjectionMatrices[1] = glm::inverse(m_globalVertexUniforms.projectionMatrices[1]);
        }

        m_globalFragmentUniforms.resolution = glm::vec4(m_width, m_height, m_shadowMapResolution, 0);

        for (size_t i = 0; i < m_shadowLights.size(); i++)
        {
            auto light = m_shadowLights[i];
            auto camera = m_scene->getCamera();

            auto shadowRange = light->m_shadowRange;
            auto shadowCenter = light->m_shadowCenter;

            m_lightMatrices[i] = glm::ortho(-shadowRange, shadowRange, -shadowRange, shadowRange, -shadowRange, shadowRange);
            glm::mat4 correctionMatrix; // for Vulkan rendering
            correctionMatrix[1][1] = -1;
            correctionMatrix[2][2] = 0.5;
            correctionMatrix[3][2] = 0.5;
            m_lightMatrices[i] *= correctionMatrix;
            auto eye = glm::tvec3<float>(shadowCenter.x(), shadowCenter.y(), shadowCenter.z());
            auto center = glm::tvec3<float>(light->getFocalPoint().x(), light->getFocalPoint().y(), light->getFocalPoint().z()) + eye;
            auto offset = glm::normalize(eye - center) * shadowRange;
            center += offset;
            eye += offset;
            auto up = glm::tvec3<float>(0, 1, 0);
            m_lightMatrices[i] *= glm::lookAt(eye, center, up);
            m_globalFragmentUniforms.lightMatrices[i] = m_lightMatrices[i];
        }
    }

    m_globalVertexUniformBuffer->updateUniforms(sizeof(VulkanGlobalVertexUniforms), &m_globalVertexUniforms, frameIndex);
    m_globalFragmentUniformBuffer->updateUniforms(sizeof(VulkanGlobalFragmentUniforms), &m_globalFragmentUniforms, frameIndex);
}

void
VulkanRenderer::createShadowMaps(uint32_t resolution)
{
    // shadow maps image
    uint32_t numShadows = 0;
    for (auto light : m_scene->getLights())
    {
        if (light->getType() == LightType::DIRECTIONAL_LIGHT)
        {
            numShadows++;
        }
    }

    VkImageCreateInfo shadowMapsInfo;
    shadowMapsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    shadowMapsInfo.pNext = nullptr;
    shadowMapsInfo.flags = 0;
    shadowMapsInfo.imageType = VK_IMAGE_TYPE_2D;
    shadowMapsInfo.format = VulkanFormats::SHADOW_FORMAT;
    shadowMapsInfo.extent = { resolution, resolution, 1 };
    shadowMapsInfo.mipLevels = 1;
    shadowMapsInfo.arrayLayers = std::max(numShadows, (uint32_t)1);
    shadowMapsInfo.samples = m_samples;
    shadowMapsInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    shadowMapsInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           | VK_IMAGE_USAGE_SAMPLED_BIT;
    shadowMapsInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    shadowMapsInfo.queueFamilyIndexCount = 0;
    shadowMapsInfo.pQueueFamilyIndices = nullptr;
    shadowMapsInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_shadowMaps = m_memoryManager.requestImage(m_renderDevice, shadowMapsInfo, VulkanMemoryType::TEXTURE);

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = std::max(numShadows, (uint32_t)1);

    VkImageViewCreateInfo imageViewInfo;
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext = nullptr;
    imageViewInfo.flags = 0;
    imageViewInfo.image = *m_shadowMaps->getImage();
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    imageViewInfo.format = VulkanFormats::SHADOW_FORMAT;
    imageViewInfo.components = VulkanDefaults::getDefaultComponentMapping();
    imageViewInfo.subresourceRange = subresourceRange;

    vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_shadowMapsView);

    m_shadowFramebuffers.clear();
    m_shadowMapsViews.resize((size_t)numShadows);
    m_shadowPasses.resize((size_t)numShadows);
    uint32_t currentLight = 0;
    auto shadowSamples = VK_SAMPLE_COUNT_1_BIT;

    for (auto light : m_scene->getLights())
    {
        if (light->getType() == LightType::DIRECTIONAL_LIGHT && currentLight < 16)
        {
            imageViewInfo.subresourceRange.baseArrayLayer = currentLight;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

            vkCreateImageView(m_renderDevice, &imageViewInfo, nullptr, &m_shadowMapsViews[currentLight]);

            VulkanRenderPassGenerator::generateShadowRenderPass(m_renderDevice, m_shadowPasses[currentLight], shadowSamples, 1);

            m_shadowFramebuffers.push_back(
                std::make_shared<VulkanFramebuffer>(m_memoryManager, resolution, resolution, shadowSamples));
            m_shadowFramebuffers[currentLight]->setDepth(&m_shadowMaps[currentLight], &m_shadowMapsViews[currentLight], VulkanFormats::SHADOW_FORMAT);
            m_shadowFramebuffers[currentLight]->initializeFramebuffer(&m_shadowPasses[currentLight]);

            auto directionalLight = std::dynamic_pointer_cast<DirectionalLight>(light);
            directionalLight->m_shadowMapIndex = currentLight;
            m_shadowLights.push_back(directionalLight);
            currentLight++;
        }
    }

    m_lightMatrices.resize(currentLight);
}

void
VulkanRenderer::setShadowMapResolution(uint32_t resolution)
{
    m_shadowMapResolution = resolution;
}

void
VulkanRenderer::setResolution(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
}

void
VulkanRenderer::setBloomOn()
{
    m_postProcessingChain->m_bloom = true;
}

void
VulkanRenderer::setBloomOff()
{
    m_postProcessingChain->m_bloom = false;
}

void
VulkanRenderer::enableLensDistortion(const float distortion)
{
    float d = distortion;
    if (distortion >= 1.0f)
    {
        d = 0.99f;
        LOG(WARNING) << "Distortion invalid (>= 1.0f), clamped to 0.99f";
    }

    if (distortion <= -1.0f)
    {
        d = -0.99f;
        LOG(WARNING) << "Distortion invalid (<= -1.0f), clamped to -0.99f";
    }

    m_enableLensDistortion = true;
    m_lensDistortionFactor = d;
}

void
VulkanRenderer::setCommandBufferState(VkCommandBuffer * commandBuffer, uint32_t width, uint32_t height)
{
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.height = height;
    viewport.width = width;
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

    VkRect2D scissor;

    scissor.offset = { 0, 0 };
    scissor.extent = { (uint32_t)viewport.width,
                       (uint32_t)viewport.height };

    vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
}

void
VulkanRenderer::setupGUI()
{
    std::array<VkDescriptorPoolSize, 11> descriptorPoolSizes;
    descriptorPoolSizes[0] = { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 };
    descriptorPoolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 };
    descriptorPoolSizes[2] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024 };
    descriptorPoolSizes[3] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 };
    descriptorPoolSizes[4] = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 };
    descriptorPoolSizes[5] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 };
    descriptorPoolSizes[6] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 };
    descriptorPoolSizes[7] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 };
    descriptorPoolSizes[8] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 };
    descriptorPoolSizes[9] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 };
    descriptorPoolSizes[10] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024 };

    VkDescriptorPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
    info.pPoolSizes = &descriptorPoolSizes[0];
    info.maxSets = 1024;
    vkCreateDescriptorPool(m_renderDevice, &info, nullptr, &m_GUIDescriptorPool);

    ImGui_ImplVulkan_InitInfo GUIInfo;
    GUIInfo.Allocator = nullptr;
    GUIInfo.CheckVkResultFn = nullptr;
    GUIInfo.DescriptorPool = m_GUIDescriptorPool;
    GUIInfo.Device = m_renderDevice;
    GUIInfo.Instance = *m_instance;
    GUIInfo.PhysicalDevice = m_physicalDevices[0];
    GUIInfo.PipelineCache = m_pipelineCache;
    GUIInfo.Queue = m_renderQueue;
    GUIInfo.QueueFamily = m_renderQueueFamily;
    ImGui_ImplVulkan_Init(&GUIInfo, m_GUIRenderPass);

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(*m_memoryManager.m_transferCommandBuffer, &commandBufferBeginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(*m_memoryManager.m_transferCommandBuffer);
    vkEndCommandBuffer(*m_memoryManager.m_transferCommandBuffer);

    VkCommandBuffer commandBuffers[] = { *m_memoryManager.m_transferCommandBuffer };

    VkPipelineStageFlags stageWaitFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[1];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount = 0;
    submitInfo[0].pWaitSemaphores = nullptr;
    submitInfo[0].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = commandBuffers;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = nullptr;

    vkQueueSubmit(m_renderQueue, 1, submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(m_renderQueue);
    ImGui_ImplVulkan_InvalidateFontUploadObjects();
}

void
VulkanRenderer::initializeFramebufferAttachments(VkCommandBuffer * commandBuffer)
{
    VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_depthImage[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, m_numViews);

    for (int i = 1; i < m_depthImage.size(); i++)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_depthImage[i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_numViews);
    }

    for (int i = 0; i < m_HDRImage[0].size(); i++)
    {
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_HDRImage[0][i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_numViews);
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_HDRImage[1][i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_numViews);
        VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_HDRImage[2][i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_numViews);
    }

    VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_normalImage,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_numViews);
    VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_LDRImage[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    VulkanAttachmentBarriers::changeImageLayout(commandBuffer, m_renderQueueFamily, m_LDRImage[1],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
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
        vkDestroySemaphore(m_renderDevice, m_presentImages, nullptr);

        for (auto fence : m_commandBufferSubmit)
        {
            vkDestroyFence(m_renderDevice, fence, nullptr);
        }

        // Clear all memory
        m_memoryManager.clear();

        // Delete framebuffers
        this->deleteFramebuffers();

        vkDestroyDescriptorPool(m_renderDevice, m_GUIDescriptorPool, nullptr);

        // Delete shadows
        for (auto imageView : m_shadowMapsViews)
        {
            vkDestroyImageView(m_renderDevice, imageView, nullptr);
        }
        vkDestroyImageView(m_renderDevice, m_shadowMapsView, nullptr);

        // Delete textures
        for (auto texture : m_textureMap)
        {
            texture.second->clear(&m_renderDevice);
        }
        m_noiseTextureDelegate->clear(&m_renderDevice);

        for (auto renderDelegate : m_renderDelegates)
        {
            renderDelegate->m_material->clear(&m_renderDevice);
            if (renderDelegate->m_depthMaterial)
            {
                renderDelegate->m_depthMaterial->clear(&m_renderDevice);
            }
            if (m_shadowPasses.size() > 0 &&
                renderDelegate->m_shadowMaterial != nullptr)
            {
                renderDelegate->m_shadowMaterial->clear(&m_renderDevice);
            }
        }

        for (auto postProcess : m_postProcessingChain->m_postProcesses)
        {
            postProcess->clear(&m_renderDevice);
        }

        for (auto pass : m_ssao)
        {
            pass->clear(&m_renderDevice);
        }

        for (auto pass : m_HDRTonemaps)
        {
            pass->clear(&m_renderDevice);
        }

        for (auto pass : m_downSample)
        {
            pass->clear(&m_renderDevice);
        }

        vkDestroyPipelineCache(m_renderDevice, m_pipelineCache, nullptr);

        vkDestroyRenderPass(m_renderDevice, m_opaqueRenderPass, nullptr);
        vkDestroyRenderPass(m_renderDevice, m_decalRenderPass, nullptr);
        vkDestroyRenderPass(m_renderDevice, m_particleRenderPass, nullptr);
        vkDestroyRenderPass(m_renderDevice, m_depthRenderPass, nullptr);
        vkDestroyRenderPass(m_renderDevice, m_GUIRenderPass, nullptr);

        for (auto pass : m_shadowPasses)
        {
            vkDestroyRenderPass(m_renderDevice, pass, nullptr);
        }

        vkDestroySampler(m_renderDevice, m_HDRImageSampler, nullptr);
        vkDestroySampler(m_renderDevice, m_swapchainImageSampler, nullptr);

        for (auto framebuffer : m_shadowFramebuffers)
        {
            framebuffer->clear(&m_renderDevice);
        }

        ImGui_ImplVulkan_Shutdown();

        vkDestroySwapchainKHR(m_renderDevice, *m_swapchain, nullptr);

        // Delete command buffers
        vkDestroyCommandPool(m_renderDevice, m_renderCommandPool, nullptr);
        vkDestroyCommandPool(m_renderDevice, m_postProcessingCommandPool, nullptr);

        vkDestroyDevice(m_devices[i], nullptr);
    }

#ifndef NDEBUG
    PFN_vkDestroyDebugReportCallbackEXT createCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*m_instance, "vkDestroyDebugReportCallbackEXT");
    createCallback(*m_instance, m_debugReportCallback, nullptr);
#endif
    vkDestroyInstance(*m_instance, nullptr);
}
}