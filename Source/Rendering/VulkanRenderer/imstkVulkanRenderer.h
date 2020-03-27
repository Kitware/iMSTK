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

#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#ifdef iMSTK_ENABLE_VR
#include "openvr.h"
#endif

#include "vulkan/vulkan.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "imstkScene.h"
#include "imstkRenderer.h"
#include "imstkDecalPool.h"
#include "imstkRenderParticleEmitter.h"
#include "imstkTextureManager.h"
#include "imstkAnimationObject.h"

#include "imstkVulkanValidation.h"
#include "imstkVulkanVertexBuffer.h"
#include "imstkVulkanUniformBuffer.h"
#include "imstkVulkanRenderDelegate.h"
#include "imstkVulkanMaterialDelegate.h"
#include "imstkVulkanDecalRenderDelegate.h"
#include "imstkVulkanParticleRenderDelegate.h"
#include "imstkVulkanPostProcess.h"
#include "imstkVulkanPostProcessingChain.h"
#include "imstkVulkanMemoryManager.h"
#include "imstkVulkanFramebuffer.h"
#include "imstkVulkanUtilities.h"
#include "imstkVulkanRenderPassGenerator.h"

namespace imstk
{
struct VulkanRendererConstants
{
    unsigned int numLights;
};

class VulkanRenderer : public Renderer
{
public:
    explicit VulkanRenderer(std::shared_ptr<Scene> scene);
    ~VulkanRenderer();

    void setShadowMapResolution(uint32_t resolution);

    void setResolution(unsigned int width, unsigned int height);

    void setBloomOn();
    void setBloomOff();

    void enableLensDistortion(const float distortion);

protected:
    friend class VulkanViewer;
    friend class VulkanInteractorStyleVR;
    friend class VulkanMaterialDelegate;
    friend class VulkanPostProcess;
    friend class VulkanPostProcessingChain;

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
    void initializeFramebuffers(VkSwapchainKHR* swapchain);

    ///
    /// \brief Initializes framebuffer images
    ///
    void initializeFramebufferImages(VkSwapchainKHR* swapchain);

    ///
    /// \brief Deletes the framebuffer
    ///
    void deleteFramebuffers();

    ///
    /// \brief Initializes the framebuffer
    ///
    void resizeFramebuffers(VkSwapchainKHR* swapchain, int width, int height);

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
    void updateGlobalUniforms(uint32_t frameIndex);

    ///
    /// \brief Create shadow maps
    ///
    void createShadowMaps(uint32_t resolution);

    /// \brief Update background colors
    ///
    void updateBackground(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false) {};

protected:
    friend class VulkanViewer;
    friend class VulkanMaterialDelegate;
    friend class VulkanPostProcess;
    friend class VulkanPostProcessingChain;

    void createInstance();

    void initialize(const unsigned int width,
                    const unsigned int height,
                    const unsigned int windowWidth,
                    const unsigned int windowHeight);
    void loadAllVisualModels();

    ///
    /// \brief Load visual model
    /// \param visualModel visual model to use to create render delegate
    /// \param type type of scene object (determines renderer implemetation)
    /// \returns Render delegate
    ///
    std::shared_ptr<VulkanRenderDelegate> loadVisualModel(
        std::shared_ptr<VisualModel> visualModel,
        SceneObject::Type            type);

    ///
    /// \brief Sets some command buffer state
    ///
    void setCommandBufferState(VkCommandBuffer* commandBuffer, uint32_t width, uint32_t height);

    ///
    /// \brief Sets up GUI
    ///
    void setupGUI();

    ///
    /// \brief Initialize images to correct layout
    ///
    void initializeFramebufferAttachments(VkCommandBuffer* commandBuffer);

    unsigned int m_width        = 2000;
    unsigned int m_height       = 1600;
    unsigned int m_windowWidth  = 1000;
    unsigned int m_windowHeight = 800;
    float m_fov       = PI;
    float m_nearPlane = 0.01;
    float m_farPlane  = 1000;

    VulkanRendererConstants m_constants;

    std::vector<std::string> m_extensions;
    std::vector<const char*> m_layers;

    std::shared_ptr<Scene> m_scene = nullptr;

    VkInstance* m_instance = nullptr;
    VkDebugReportCallbackEXT m_debugReportCallback;

    uint32_t m_physicalDeviceCount      = 0;
    VkPhysicalDevice* m_physicalDevices = nullptr;
    VkPhysicalDevice  m_renderPhysicalDevice;

    uint32_t  m_deviceCount = 0;
    VkDevice* m_devices     = nullptr;
    VkPhysicalDeviceLimits m_deviceLimits;
    float    m_anisotropyAmount;
    VkDevice m_renderDevice;

    VkPipelineCache m_pipelineCache;

    uint32_t m_queueFamilyPropertiesCount = 0;
    VkQueueFamilyProperties* m_queueFamilyProperties = nullptr;
    VkQueue m_renderQueue;

    VkCommandPool m_renderCommandPool;
    VkCommandPool m_postProcessingCommandPool;
    std::vector<VkCommandBuffer> m_renderCommandBuffer;
    std::vector<VkCommandBuffer> m_postProcessingCommandBuffer;

    uint32_t m_dynamicOffsets = { 0 };

    VulkanMemoryManager m_memoryManager;

    std::shared_ptr<VulkanUniformBuffer> m_globalVertexUniformBuffer;
    std::shared_ptr<VulkanUniformBuffer> m_globalFragmentUniformBuffer;
    VulkanGlobalVertexUniforms   m_globalVertexUniforms;
    VulkanGlobalFragmentUniforms m_globalFragmentUniforms;

    VkDescriptorPool m_globalDescriptorPool;
    std::vector<VkDescriptorSet>       m_globalDescriptorSets;
    std::vector<VkDescriptorSetLayout> m_globalDescriptorSetLayouts;
    std::vector<VkWriteDescriptorSet>  m_globalWriteDescriptorSets;

    VkDescriptorPool m_GUIDescriptorPool;

    VkRenderPass m_depthRenderPass;
    VkRenderPass m_opaqueRenderPass;
    VkRenderPass m_decalRenderPass;
    VkRenderPass m_particleRenderPass;
    VkRenderPass m_GUIRenderPass;

    // Swapchain
    VkSwapchainKHR* m_swapchain    = nullptr;
    uint32_t m_swapchainImageCount = 0;
    std::vector<VulkanInternalImage*> m_swapchainImages;
    std::vector<VkImage>     m_swapchainNativeImages;
    std::vector<VkImageView> m_swapchainImageViews;
    VkSampler m_swapchainImageSampler;

    // Final image buffers (used before image gets copied to swapchain images)
    VulkanInternalImage* m_LDRImage[2];
    VkImageView m_LDRImageView[2];

    // Depth buffer
    std::vector<VulkanInternalImage*> m_depthImage;
    std::vector<VkImageView> m_depthImageView;

    // Normal buffer
    VulkanInternalImage* m_normalImage;
    VkImageView m_normalImageView;

    // AO buffers
    VulkanInternalImage* m_halfAOImage[2];
    VkImageView m_halfAOImageView[2];

    // Color buffers
    std::vector<VulkanInternalImage*> m_HDRImage[3];
    VkSampler m_HDRImageSampler;
    std::vector<VkImageView> m_HDRImageView[3];
    uint32_t m_mipLevels = 1;

    // For noise-post processing
    std::shared_ptr<Texture> m_noiseTexture = nullptr;
    std::shared_ptr<VulkanTextureDelegate> m_noiseTextureDelegate = nullptr;

    std::shared_ptr<VulkanFramebuffer> m_opaqueFramebuffer;
    std::shared_ptr<VulkanFramebuffer> m_decalFramebuffer;
    std::shared_ptr<VulkanFramebuffer> m_particleFramebuffer;
    std::shared_ptr<VulkanFramebuffer> m_depthFramebuffer;

    std::vector<std::shared_ptr<VulkanPostProcess>> m_HDRTonemaps; // One for each eye
    std::vector<std::shared_ptr<VulkanPostProcess>> m_ssao;
    std::vector<std::shared_ptr<VulkanPostProcess>> m_downSample;

    std::shared_ptr<VulkanPostProcessingChain> m_postProcessingChain;

    VkSemaphore m_readyToRender;
    VkSemaphore m_drawingComplete;
    VkSemaphore m_presentImages;

    uint32_t m_buffering = 2;

    int m_frameNumber = 0;
    VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkFence> m_commandBufferSubmit;

    // Device limits and features
    bool m_supportsWideLines = false;

    glm::mat4 m_projectionMatrix;

    VulkanInternalImage* m_shadowMaps;          ///< a single texture array (hence why it's one image)
    VkImageView m_shadowMapsView;               ///< for binding to the shaders (so shaders can access all layers)
    std::vector<VkImageView> m_shadowMapsViews; ///< for framebuffers
    std::vector<std::shared_ptr<DirectionalLight>>  m_shadowLights;
    std::vector<std::shared_ptr<VulkanFramebuffer>> m_shadowFramebuffers;
    std::vector<VkRenderPass> m_shadowPasses;
    uint32_t m_shadowMapResolution = 2048;
    std::vector<glm::mat4> m_lightMatrices;

    bool  m_enableLensDistortion = false;
    float m_lensDistortionFactor = 0.0f;

    std::vector<std::shared_ptr<VulkanRenderDelegate>> m_renderDelegates;

    uint32_t m_renderQueueFamily = 0;
    Vec3d    m_backgroundColor   = Vec3d(0.5, 0.5, 0.5);

    std::map<std::shared_ptr<Texture>, std::shared_ptr<VulkanTextureDelegate>> m_textureMap;

    uint32_t m_numViews = 1; ///< for multiview functionality

    glm::mat4 m_viewMatrices[2];
    glm::mat4 m_projectionMatrices[2];
    glm::vec4 m_cameraPositions[2];
    bool      m_VRMode = false;

#ifdef iMSTK_ENABLE_VR
    vr::IVRSystem* m_VRSystem = nullptr;
#endif
};
}
