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

#ifndef imstkVulkanRenderer_h
#define imstkVulkanRenderer_h

#include <iostream>
#include <memory>
#include <vector>
#include <fstream>

#include "vulkan/vulkan.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imstkScene.h"
#include "imstkRenderer.h"
#include "imstkDecalPool.h"

#include "imstkVulkanValidation.h"
#include "imstkVulkanVertexBuffer.h"
#include "imstkVulkanUniformBuffer.h"
#include "imstkVulkanRenderDelegate.h"
#include "imstkVulkanMaterialDelegate.h"
#include "imstkVulkanDecalRenderDelegate.h"
#include "imstkVulkanPostProcess.h"
#include "imstkVulkanPostProcessingChain.h"
#include "imstkVulkanMemoryManager.h"
#include "imstkVulkanFramebuffer.h"
#include "imstkVulkanRenderPassGenerator.h"

namespace imstk
{
struct VulkanRendererConstants
{
    unsigned int numLights;
};

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(std::shared_ptr<Scene> scene);
    ~VulkanRenderer();

    ///
    /// \brief Populates the device fields for the rendering class (both physical and logical devices)
    ///
    void setupGPUs();

    ///
    /// \brief Prints the physical device name
    ///
    void printGPUs();

    ///
    /// \brief Sets up command pools
    ///
    void setupCommandPools();

    ///
    /// \brief Sets up command pools
    ///
    void buildCommandBuffer();

    ///
    /// \brief Sets up render pass
    ///
    void setupRenderPasses();

    ///
    /// \brief Sets up swapchain
    ///
    void setupSwapchain();

    ///
    /// \brief Initializes the framebuffer
    ///
    void initializeFramebuffers(VkSwapchainKHR * swapchain);

    ///
    /// \brief Deletes the framebuffer
    ///
    void deleteFramebuffers();

    ///
    /// \brief Initializes the framebuffer
    ///
    void resizeFramebuffers(VkSwapchainKHR * swapchain, int width, int height);

    ///
    /// \brief Renders the frame
    ///
    void renderFrame();

    ///
    /// \brief Setups semaphores/fences
    ///
    void setupSynchronization();

    ///
    /// \brief Get device memory properties
    ///
    void setupMemoryManager();

    ///
    /// \brief Create global uniforms
    ///
    void createGlobalUniformBuffers();

    ///
    /// \brief Initialize the post processing effects
    ///
    void initializePostProcesses();

    ///
    /// \brief Update global uniforms
    ///
    void updateGlobalUniforms();

protected:
    friend class VulkanViewer;
    friend class VulkanMaterialDelegate;
    friend class VulkanPostProcess;
    friend class VulkanPostProcessingChain;

    void initialize();
    void loadAllGeometry();
    std::shared_ptr<VulkanRenderDelegate> loadGeometry(std::shared_ptr<Geometry> geometry);

    unsigned int m_width = 1000;
    unsigned int m_height = 800;
    float m_fov = PI;
    float m_nearPlane = 0.01;
    float m_farPlane = 1000;

    VulkanRendererConstants m_constants;

    std::vector<char *> m_extensions;
    std::vector<char *> m_layers;

    std::shared_ptr<Scene> m_scene = nullptr;

    VkInstance * m_instance = nullptr;

    uint32_t m_physicalDeviceCount = 0;
    VkPhysicalDevice * m_physicalDevices = nullptr;
    VkPhysicalDevice m_renderPhysicalDevice;

    uint32_t m_deviceCount = 0;
    VkDevice * m_devices = nullptr;
    VkPhysicalDeviceLimits m_deviceLimits;
    VkDevice m_renderDevice;

    VkPipelineCache m_pipelineCache;

    uint32_t m_queueFamilyPropertiesCount = 0;
    VkQueueFamilyProperties * m_queueFamilyProperties = nullptr;
    VkQueue m_renderQueue;

    VkCommandPool m_renderCommandPool;
    VkCommandBuffer m_renderCommandBuffer;
    VkCommandBuffer m_postProcessingCommandBuffer;

    uint32_t m_dynamicOffsets = {0};

    VulkanMemoryManager m_memoryManager;

    std::shared_ptr<VulkanUniformBuffer> m_globalVertexUniformBuffer;
    std::shared_ptr<VulkanUniformBuffer> m_globalFragmentUniformBuffer;
    VulkanGlobalVertexUniforms m_globalVertexUniforms;
    VulkanGlobalFragmentUniforms m_globalFragmentUniforms;

    VkDescriptorPool m_globalDescriptorPool;
    std::vector<VkDescriptorSet> m_globalDescriptorSets;
    std::vector<VkDescriptorSetLayout> m_globalDescriptorSetLayouts;
    std::vector<VkWriteDescriptorSet> m_globalWriteDescriptorSets;

    std::vector<VkRenderPass> m_renderPasses;

    // Framebuffers
    VkSwapchainKHR * m_swapchain = nullptr;
    uint32_t m_swapchainImageCount = 0;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkSampler> m_swapchainImageSamplers;

    // Depth buffer
    std::vector<VkImage> m_depthImage;
    std::vector<VkImageView> m_depthImageView;
    std::vector<VkDeviceMemory*> m_depthImageMemory;

    // Normal buffer
    VkImage m_normalImage;
    VkImageView m_normalImageView;
    VkDeviceMemory * m_normalImageMemory;

    // Color buffers
    std::vector<VkImage> m_HDRImage[3];
    VkSampler m_HDRImageSampler;
    std::vector<VkImageView> m_HDRImageView[3];
    std::vector<VkDeviceMemory*> m_HDRImageMemory[3];
    uint32_t m_mipLevels = 1;

    std::vector<std::shared_ptr<VulkanFramebuffer>> m_drawingFramebuffers;
    std::vector<std::shared_ptr<VulkanPostProcess>> m_HDRTonemaps;
    std::shared_ptr<VulkanPostProcessingChain> m_postProcessingChain;

    VkSemaphore m_readyToRender;
    VkSemaphore m_drawingComplete;
    VkSemaphore m_presentImages;

    unsigned int m_buffering = 3;

    int m_frameNumber = 0;
    VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;

    VkFence m_commandBufferSubmit;

    glm::mat4 m_projectionMatrix;

    std::vector<std::shared_ptr<VulkanRenderDelegate>> m_renderDelegates;

    uint32_t m_renderQueueFamily = 0;
};
}

#endif