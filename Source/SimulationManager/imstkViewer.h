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

#include <memory>

#include "imstkScene.h"
#include "imstkRenderer.h"
#include "imstkScreenCaptureUtility.h"
#include "imstkInteractorStyle.h"
#include "imstkGUICanvas.h"

namespace imstk
{
class SimulationManager;

///
/// \class Viewer
///
/// \brief Rendering window manager and contains user API to configure the rendering with various backends
///
class Viewer
{
public:

    Viewer() {};

    Viewer(SimulationManager * manager){};

    ///
    /// \brief Get scene currently being rendered
    ///
    std::shared_ptr<Scene> getActiveScene() const;

    ///
    /// \brief Set scene to be rendered
    ///
    virtual void setActiveScene(std::shared_ptr<Scene> scene) = 0;

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
    virtual const Renderer::Mode getRenderingMode(){ return Renderer::Mode::EMPTY; };

    ///
    /// \brief Returns true if the Viewer is rendering
    ///
    const bool& isRendering() const;

    ///
    /// \brief Retrieve the renderer associated with the current scene
    ///
    std::shared_ptr<Renderer> getActiveRenderer() const;

    ///
    /// \brief access screen shot utility
    ///
    std::shared_ptr<ScreenCaptureUtility> getScreenCaptureUtility() const;

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false) = 0;

    ///
    /// \brief Get canvas
    ///
    std::shared_ptr<GUIOverlay::Canvas> getCanvas();

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

protected:
    std::shared_ptr<Scene> m_activeScene;

    std::unordered_map<std::shared_ptr<Scene>, std::shared_ptr<Renderer>> m_rendererMap;

    std::shared_ptr<ScreenCaptureUtility> m_screenCapturer; ///> Screen shot utility

    std::shared_ptr<InteractorStyle> m_interactorStyle;

    bool m_running = false;
    std::shared_ptr<GUIOverlay::Canvas> m_canvas = std::make_shared<GUIOverlay::Canvas>();;
};
}
