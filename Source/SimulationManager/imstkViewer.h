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

#include "imstkRenderer.h"

#include <unordered_map>

namespace imstk
{
class InteractorStyle;
class OpenVRDeviceClient;
class Scene;
class ScreenCaptureUtility;
class SimulationManager;

#ifdef iMSTK_USE_Vulkan
namespace GUIOverlay
{
class Canvas;
}
#endif

namespace
{
using EventHandlerFunction = std::function<bool (InteractorStyle* iStyle)>;
}

struct ViewerConfig
{
    std::string m_windowName = "imstk";

    bool m_hideCurzor = false;
    bool m_hideBorder = true;
    bool m_fullScreen = false;

    int m_renderWinWidth  = 1000;
    int m_renderWinHeight = 800;

    bool m_enableVR = false;
};

///
/// \class Viewer
///
/// \brief Based class for viewer that manages render window and the renderers
/// Creates backend-specific renderers on a per-scene basis.
/// Contains user API to configure the rendering with various backends
/// Manages the keyboard and mouse events
///
class Viewer
{
public:
    Viewer();
    Viewer(ViewerConfig config);
    virtual ~Viewer() = default;

    ///
    /// \brief Get scene currently being rendered
    ///
    const std::shared_ptr<Scene>& getActiveScene() const;

    ///
    /// \brief Set scene to be rendered
    ///
    virtual void setActiveScene(const std::shared_ptr<Scene>& scene) = 0;

    ///
    /// \brief Start rendering
    ///
    virtual void startRenderingLoop() = 0;

    ///
    /// \brief Terminate rendering
    ///
    virtual void endRenderingLoop() = 0;

    ///
    /// \brief Setup the current renderer to render what's needed
    /// based on the mode chosen
    ///
    virtual void setRenderingMode(const Renderer::Mode mode) = 0;

    ///
    /// \brief Get the current renderer's mode
    ///
    virtual Renderer::Mode getRenderingMode() { return Renderer::Mode::Empty; }

    ///
    /// \brief Returns true if the Viewer is rendering
    ///
    const bool& isRendering() const;

    ///
    /// \brief Retrieve the renderer associated with the current scene
    ///
    const std::shared_ptr<Renderer>& getActiveRenderer() const;

    ///
    /// \brief Set a string to be the title for the render window
    ///
    virtual void setWindowTitle(const std::string& title) = 0;

    ///
    /// \brief access screen shot utility
    ///
    const std::shared_ptr<ScreenCaptureUtility>& getScreenCaptureUtility() const;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false) = 0;

#ifdef iMSTK_USE_Vulkan
    ///
    /// \brief Get canvas
    ///
    const std::shared_ptr<GUIOverlay::Canvas>& getCanvas();
#endif

    ///
    /// \brief Set custom event handlers on interactor style
    ///
    void setOnCharFunction(char c, EventHandlerFunction func);
    void setOnMouseMoveFunction(EventHandlerFunction func);
    void setOnLeftButtonDownFunction(EventHandlerFunction func);
    void setOnLeftButtonUpFunction(EventHandlerFunction func);
    void setOnMiddleButtonDownFunction(EventHandlerFunction func);
    void setOnMiddleButtonUpFunction(EventHandlerFunction func);
    void setOnRightButtonDownFunction(EventHandlerFunction func);
    void setOnRightButtonUpFunction(EventHandlerFunction func);
    void setOnMouseWheelForwardFunction(EventHandlerFunction func);
    void setOnMouseWheelBackwardFunction(EventHandlerFunction func);

    ///
    /// \brief Set custom behavior to be run on every frame.
    /// The return of the function will not have any  effect.
    ///
    void setOnTimerFunction(EventHandlerFunction func);

    ///
    /// \brief Acquire the first VR device of specified type
    ///
    std::shared_ptr<OpenVRDeviceClient> getVRDeviceClient(int deviceType);

    ///
    /// \brief Acquire the full list of VR devices tied to this viewer
    ///
    const std::list<std::shared_ptr<OpenVRDeviceClient>>& getVRDeviceClient() const { return m_vrDeviceClients; }

protected:
    std::unordered_map<std::shared_ptr<Scene>, std::shared_ptr<Renderer>> m_rendererMap;

    std::shared_ptr<Scene> m_activeScene = nullptr;
    std::shared_ptr<InteractorStyle>      m_interactorStyle = nullptr;
    std::shared_ptr<ScreenCaptureUtility> m_screenCapturer  = nullptr; ///> Screen shot utility

    bool m_running = false;
#ifdef iMSTK_USE_Vulkan
    std::shared_ptr<GUIOverlay::Canvas> m_canvas = nullptr;
#endif

    std::shared_ptr<ViewerConfig> m_config = nullptr;

    ///> The VR controllers are tied to the view
    std::list<std::shared_ptr<OpenVRDeviceClient>> m_vrDeviceClients;
};
}
