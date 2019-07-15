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

#include "imstkVulkanViewer.h"

namespace imstk
{
VulkanViewer::VulkanViewer(SimulationManager * manager, bool enableVR)
{
    m_simManager = manager;

#ifdef iMSTK_ENABLE_VR
    if (vr::VR_IsHmdPresent() && enableVR)
    {
        m_VRMode = enableVR;
    }
#endif

    if (m_VRMode)
    {
#ifdef iMSTK_ENABLE_VR
        auto style = std::make_shared<VulkanInteractorStyleVR>();
        style->m_simManager = m_simManager;
        m_interactorStyle = style;
#endif
    }
    else
    {
        auto style = std::make_shared<VulkanInteractorStyleFreeCamera>();
        style->m_simManager = m_simManager;
        m_interactorStyle = style;
    }

    // Create GUI
    ImGui::CreateContext();
}

void
VulkanViewer::setActiveScene(const std::shared_ptr<Scene>& scene)
{
    m_renderer = std::make_shared<VulkanRenderer>(scene);
    m_renderer->m_backgroundColor = m_backgroundColor;
}

void
VulkanViewer::setBackgroundColors(const Vec3d color1, const Vec3d color2, const bool gradientBackground)
{
    m_backgroundColor = color1;

    if (m_renderer)
    {
        m_renderer->m_backgroundColor = m_backgroundColor;
    }
}

void
VulkanViewer::enableVSync()
{
    m_VSync = true;
}

void
VulkanViewer::disableVSync()
{
    m_VSync = false;
}

void
VulkanViewer::enableFullscreen()
{
    m_fullscreen = true;
}

void
VulkanViewer::disableFullscreen()
{
    m_fullscreen = false;
}

void
VulkanViewer::setResolution(unsigned int width, unsigned int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    m_width = width;
    m_height = height;
}

void
VulkanViewer::enableLensDistortion(const float distortion)
{
    m_renderer->enableLensDistortion(distortion);
}

void
VulkanViewer::startRenderingLoop()
{
    m_running = true;

#ifdef iMSTK_ENABLE_VR
    if (m_VRMode)
    {
        m_renderer->m_VRMode = true;

        vr::EVRInitError error;
        m_renderer->m_VRSystem = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Scene);

        if (error != vr::EVRInitError::VRInitError_None)
        {
            LOG(FATAL) << "VR initialization error: " << error;
        }
        auto interactor = std::dynamic_pointer_cast<VulkanInteractorStyleVR>(m_interactorStyle);
        interactor->initialize(m_renderer);

        m_renderer->m_VRSystem->GetRecommendedRenderTargetSize(&m_width, &m_height);
        m_windowWidth = m_width;
        m_windowHeight = m_height;
    }
#endif

    this->setupWindow();
    m_renderer->createInstance();

    this->createWindow();
    m_renderer->initialize(m_width, m_height, m_windowWidth, m_windowHeight);

    this->setupSwapchain();
    m_renderer->initializeFramebufferImages(&m_swapchain);
    m_renderer->initializeFramebuffers(&m_swapchain);

    m_renderer->loadAllVisualModels();

    GUIOverlay::Utilities::initializeGUISystem();

    while (!glfwWindowShouldClose(m_window))
    {
        std::dynamic_pointer_cast<VulkanInteractorStyle>(m_interactorStyle)->OnTimer();
        glfwPollEvents();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_canvas->render();

        ImGui::Render();

        m_renderer->renderFrame();
    }

    glfwTerminate();
    m_running = false;
}

void
VulkanViewer::endRenderingLoop()
{
    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

void
VulkanViewer::setRenderingMode(const Renderer::Mode mode)
{
    m_renderer->setMode(mode, false);
}

Renderer::Mode
VulkanViewer::getRenderingMode()
{
    return m_renderer->getMode();
}

#ifdef iMSTK_ENABLE_VR
vr::IVRSystem *
VulkanViewer::getVRSystem()
{
    return m_renderer->m_VRSystem;
}

#endif

void
VulkanViewer::setupWindow()
{
    if (!glfwInit())
    {
        LOG(FATAL) << "GLFW failed to initialize";
        return;
    }

    if (!glfwVulkanSupported())
    {
        LOG(FATAL) << "Vulkan is not supported";
        return;
    }

    uint32_t tempCount;
    const char ** tempExtensions = glfwGetRequiredInstanceExtensions(&tempCount);
    for (int i = 0; i < (int)tempCount; i++)
    {
        m_renderer->m_extensions.push_back((char*)tempExtensions[i]);
    }

    // Keep resolution if not fullscreen
    if (!m_fullscreen)
    {
        return;
    }

    // find appropriate resolution
    int numMonitors;
    auto monitors = glfwGetMonitors(&numMonitors);

    int numVideoModes;
    auto videoModes = glfwGetVideoModes(monitors[0], &numVideoModes);

    unsigned int tempWidth;
    unsigned int tempHeight;
    unsigned int lastDifference = UINT_MAX;

    for (int i = 0; i < numVideoModes; i++)
    {
        auto dw = m_width - videoModes[i].width;
        auto dh = m_height - videoModes[i].height;

        if (dh * dw < lastDifference)
        {
            lastDifference = dh * dw;
            tempWidth = videoModes[i].width;
            tempHeight = videoModes[i].height;
        }
    }

    m_width = tempWidth;
    m_height = tempHeight;
}

void
VulkanViewer::createWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    int numMonitors;
    auto monitors = glfwGetMonitors(&numMonitors);

    if (!m_fullscreen || numMonitors == 0)
    {
        m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "iMSTK", nullptr, nullptr);
    }
    else
    {
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
        m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "iMSTK", monitors[0], nullptr);
    }

    // Wire window into GUI
    ImGui_ImplGlfw_InitForVulkan(m_window, false);

    VkResult status = glfwCreateWindowSurface(*m_renderer->m_instance, m_window, nullptr, &m_surface);

    std::dynamic_pointer_cast<VulkanInteractorStyle>(m_interactorStyle)->setWindow(m_window, this);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalCapabilities);
    m_windowWidth = m_physicalCapabilities.currentExtent.width;
    m_windowHeight = m_physicalCapabilities.currentExtent.height;
}

void
VulkanViewer::resizeWindow(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    vkDeviceWaitIdle(m_renderer->m_renderDevice);
    vkDestroySwapchainKHR(m_renderer->m_renderDevice, m_swapchain, nullptr);
    this->setupSwapchain();

    m_renderer->resizeFramebuffers(&m_swapchain, width, height);
}

void
VulkanViewer::setupSwapchain()
{
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalFormatsCount, nullptr);
    m_physicalFormats = new VkSurfaceFormatKHR[(int)m_physicalFormatsCount]();
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalFormatsCount, m_physicalFormats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderer->m_physicalDevices[0], m_surface, &m_presentModesCount, nullptr);
    m_presentModes = new VkPresentModeKHR[(int)m_presentModesCount]();
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderer->m_physicalDevices[0], m_surface, &m_presentModesCount, m_presentModes);

    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_renderer->m_physicalDevices[0], 0, m_surface, &supported);

    // Build swapchain
    VkExtent2D extent;

    extent.height = m_windowHeight;
    extent.width = m_windowWidth;

    bool linearColorSpaceSupported = false;

    // Right now linear colorspace is a requirement
    for (int i = 0; i < (int)m_physicalFormatsCount; i++)
    {
        if (m_physicalFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            linearColorSpaceSupported = true;
        }
    }

    if (!linearColorSpaceSupported)
    {
        LOG(FATAL) << "Linear color space not supported";
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (!m_VSync)
    {
        for (uint32_t i = 0; i < m_presentModesCount; i++)
        {
            if (m_presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = m_renderer->m_buffering; // buffering
    swapchainInfo.imageFormat = VulkanFormats::FINAL_FORMAT;
    swapchainInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(m_renderer->m_renderDevice, &swapchainInfo, nullptr, &m_swapchain);
}
}