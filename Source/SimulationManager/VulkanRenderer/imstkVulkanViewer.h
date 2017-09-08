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

#ifndef imstkVulkanViewer_h
#define imstkVulkanViewer_h

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "g3log/g3log.hpp"

#include "imstkVulkanRenderer.h"
#include "imstkViewer.h"
#include "imstkTimer.h"
#include "imstkVulkanInteractorStyle.h"

namespace imstk
{
class VulkanInteractorStyle;

class VulkanViewer : public Viewer {
public:
    VulkanViewer(SimulationManager * manager = nullptr);

    virtual void setActiveScene(std::shared_ptr<Scene> scene);

    virtual void startRenderingLoop();

    virtual void endRenderingLoop();

    ///
    /// \brief Setups up the swapchain
    ///
    /// A swapchain is basically a queue of backbuffers
    ///
    void setupSwapchain();

protected:
    friend class VulkanInteractorStyle;
    friend class VulkanInteractorStyleTrackballCamera;

    void setupWindow();
    void createWindow();
    void resizeWindow(int width, int height);

    unsigned int m_width = 1000;
    unsigned int m_height = 800;

    std::shared_ptr<VulkanRenderer> m_renderer;
    VkSurfaceKHR m_surface;
    GLFWwindow * m_window;
    SimulationManager * m_simManager;
    VkSwapchainKHR m_swapchain;

    VkSurfaceCapabilitiesKHR m_physicalCapabilities;

    uint32_t m_physicalFormatsCount;
    VkSurfaceFormatKHR * m_physicalFormats;
    std::shared_ptr<VulkanInteractorStyle> m_interactorStyle;
};
}
#endif