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
#include "imstkModule.h"

#include <unordered_map>

namespace imstk
{
class Camera;
class InteractorStyle;
class Scene;
class ScreenCaptureUtility;

struct ViewerConfig
{
    std::string m_windowName = "imstk";

    bool m_hideCurzor = false;
    bool m_hideBorder = true;
    bool m_fullScreen = false;

    int m_renderWinWidth  = 1000;
    int m_renderWinHeight = 800;
};

///
/// \class Viewer
///
/// \brief Base class for viewer that manages render window and the renderers
/// Creates backend-specific renderers on a per-scene basis.
/// Contains user API to configure the rendering with various backends
///
class Viewer : public Module
{
protected:
    Viewer(std::string name);
public:
    ~Viewer() override = default;

public:
    ///
    /// \brief Get scene currently being rendered
    ///
    std::shared_ptr<Scene> getActiveScene() const { return m_activeScene; }

    ///
    /// \brief Set scene to be rendered
    ///
    virtual void setActiveScene(std::shared_ptr<Scene> scene) = 0;

    ///
    /// \brief Setup the current renderer to render what's needed
    /// based on the mode chosen
    ///
    virtual void setRenderingMode(const Renderer::Mode mode) = 0;

    ///
    /// \brief Set the render window size
    ///
    virtual void setSize(int, int) { }
    void setSize(Vec2i size) { setSize(size[0], size[1]); }

    virtual const Vec2i getSize() const { return Vec2i::Zero(); }

    ///
    /// \brief Get the current renderer's mode
    ///
    virtual Renderer::Mode getRenderingMode() const { return Renderer::Mode::Empty; }

    ///
    /// \brief Retrieve the renderer associated with the current scene
    ///
    std::shared_ptr<Renderer> getActiveRenderer() const;

    ///
    /// \brief Set whether to sync frames to the refresh of the monitor.
    ///
    virtual void setUseVsync(const bool) { }

    ///
    /// \brief Set a string to be the title for the render window
    ///
    virtual void setWindowTitle(const std::string& title) = 0;

    ///
    /// \brief Set the info level, usually means display framerates and other
    /// viewer related information
    ///
    virtual void setInfoLevel(const int level);

    ///
    /// \brief Get the current info level
    ///
    int getInfoLevel() const { return m_infoLevel; }

    ///
    /// \brief Get the number of info levels for a viewer, varies on implementation
    ///
    virtual const int getInfoLevelCount() const { return 1; }

    ///
    /// \brief access screen shot utility
    ///
    std::shared_ptr<ScreenCaptureUtility> getScreenCaptureUtility() const { return m_screenCapturer; }

    ///
    /// \brief Set the coloring of the screen background
    /// If 'gradientBackground' is false or not supplied color1 will fill the entire background
    ///
    virtual void setBackgroundColors(const Color color1, const Color color2 = Color(0.0, 0.0, 0.0), const bool gradientBackground = false) = 0;

    virtual void processEvents() = 0;

protected:
    void updateModule() override;

    std::unordered_map<std::shared_ptr<Scene>, std::shared_ptr<Renderer>> m_rendererMap;

    std::shared_ptr<Scene>  m_activeScene;
    std::shared_ptr<Camera> m_debugCamera;
    std::shared_ptr<ScreenCaptureUtility> m_screenCapturer; ///< Screen shot utility

    std::shared_ptr<ViewerConfig> m_config;
    int m_infoLevel = 0; ///< Info level
};
} // namespace imstk
