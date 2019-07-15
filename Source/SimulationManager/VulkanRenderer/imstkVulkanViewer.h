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

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "g3log/g3log.hpp"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"

#include <climits>

#include "imstkVulkanRenderer.h"
#include "imstkViewer.h"
#include "imstkTimer.h"
#include "imstkVulkanInteractorStyleFreeCamera.h"
#include "imstkVulkanInteractorStyleVR.h"
#include "imstkVulkanUtilities.h"
#include "imstkGUIUtilities.h"

namespace imstk
{
class VulkanInteractorStyle;

class VulkanViewer : public Viewer
{
public:
    VulkanViewer(SimulationManager* manager = nullptr, bool enableVR = false);

    virtual void setActiveScene(const std::shared_ptr<Scene>& scene) override;

    virtual void startRenderingLoop() override;

    virtual void endRenderingLoop() override;

    ///
    /// \brief Setup the current renderer to render what's needed
    /// based on the mode chosen
    ///
    virtual void setRenderingMode(const Renderer::Mode mode) override;

    virtual Renderer::Mode getRenderingMode() override;

    ///
    /// \brief Setups up the swapchain
    ///
    /// A swapchain is basically a queue of backbuffers
    ///
    void setupSwapchain();

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(),
                                     const bool gradientBackground = false) override;

    ///
    /// \brief Enable/disable fullscreen
    ///
    void enableFullscreen();
    void disableFullscreen();

    ///
    /// \brief Enable/disable V-Sync
    ///
    void enableVSync();
    void disableVSync();

    ///
    /// \brief Sets the resolution
    ///
    void setResolution(unsigned int width, unsigned int height);

    ///
    /// \brief Enable lens distortion
    /// \param distortion Distortion factor to apply: range is (-1.0, 1.0)
    /// A negative distortion value will cause pincushion distortion. A
    /// positive value will cause barrel distortion.
    ///
    void enableLensDistortion(const float distortion);

#ifdef iMSTK_ENABLE_VR
    ///
    /// \brief Utility function to get VR system
    /// \returns OpenVR system
    ///
    vr::IVRSystem* getVRSystem();
#endif

protected:
    friend class VulkanInteractorStyle;
    friend class VulkanInteractorStyleFreeCamera;
    friend class VulkanInteractorStyleGeneral;
    friend class VulkanInteractorStyleVR;

    void setupWindow();
    void createWindow();
    void resizeWindow(unsigned int width, unsigned int height);

    unsigned int m_width        = 1000;
    unsigned int m_height       = 800;
    unsigned int m_windowWidth  = 1000;
    unsigned int m_windowHeight = 800;

    bool m_VSync = true;

    std::shared_ptr<VulkanRenderer> m_renderer;
    VkSurfaceKHR       m_surface;
    GLFWwindow*        m_window;
    SimulationManager* m_simManager;
    VkSwapchainKHR     m_swapchain;

    VkSurfaceCapabilitiesKHR m_physicalCapabilities;

    uint32_t          m_presentModesCount;
    VkPresentModeKHR* m_presentModes;

    uint32_t m_physicalFormatsCount;
    VkSurfaceFormatKHR* m_physicalFormats;
    bool m_fullscreen = false;

    bool m_VRMode = false;

    Vec3d m_backgroundColor = Vec3d(0.5, 0.5, 0.5);
};
}
