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
VulkanViewer::VulkanViewer(SimulationManager * manager)
{
    m_simManager = manager;
}

void
VulkanViewer::setActiveScene(std::shared_ptr<Scene>scene)
{
    m_renderer = std::make_shared<VulkanRenderer>(scene);
}

void
VulkanViewer::startRenderingLoop()
{
    m_running = true;
    this->setupWindow();
    m_renderer->initialize();
    this->createWindow();

    this->setupSwapchain();
    m_renderer->initializeFramebuffers(&m_swapchain);

    m_renderer->loadAllGeometry();

    while (!glfwWindowShouldClose(m_window))
    {
        m_renderer->renderFrame();
        glfwPollEvents();
        m_interactorStyle->OnTimer();
    }

    m_running = false;
}

void
VulkanViewer::endRenderingLoop()
{
}

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
}

void
VulkanViewer::createWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_width, m_height, "iMSTK", nullptr, nullptr);
    VkResult status = glfwCreateWindowSurface(*m_renderer->m_instance, m_window, nullptr, &m_surface);
    std::cout << status << std::endl;

    m_interactorStyle = std::make_shared<VulkanInteractorStyle>();

    m_interactorStyle->setWindow(m_window, this);
    m_interactorStyle->m_simManager = m_simManager;
}

void
VulkanViewer::resizeWindow(int width, int height)
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
    // Build swapchain
    VkExtent2D extent;
    extent.height = m_height;
    extent.width = m_width;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalCapabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalFormatsCount, nullptr);
    m_physicalFormats = new VkSurfaceFormatKHR[(int)m_physicalFormatsCount]();
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->m_physicalDevices[0], m_surface, &m_physicalFormatsCount, m_physicalFormats);

    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_renderer->m_physicalDevices[0], 0, m_surface, &supported);

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

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = 3; // triple buffering
    swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(m_renderer->m_renderDevice, &swapchainInfo, nullptr, &m_swapchain);
}
}