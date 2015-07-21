// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMVIEWER_H
#define SMVIEWER_H

// 3rd Party iIncludes
#include <SFML/Window.hpp>

// SimMedTK includes
#include "Core/ViewerBase.h"
#include "Rendering/Shader.h"
#include "Rendering/GLUtils.h"
#include "Rendering/VBO.h"
#include "Simulators/PBDSceneObject.h"
#include "Core/DataStructures.h"
#include "Simulators/StylusObject.h"
#include "Rendering/FrameBuffer.h"
#include "Rendering/Camera.h"

//forward declaration
class OpenGLWindowStream;

/// \brief Handles all rendering routines.
class Viewer : public ViewerBase
{
public:
    std::unique_ptr<sf::Context> sfmlContext;
    std::unique_ptr<sf::Window> sfmlWindow;

    std::shared_ptr<OpenGLWindowStream> windowOutput;

    /// \brief default constructor
    Viewer();
    /// \brief for exit viewer
    virtual void exitViewer() override;
    /// \brief add object for rendering
    void addObject(std::shared_ptr<CoreClass> object);
    /// \brief add text for display
    virtual void addText(std::string p_tag) override;
    /// \brief update text
    virtual void updateText(std::string p_tag, std::string p_string) override;
    virtual void updateText(int p_handle, std::string p_string) override;
    /// \brief set scene as texture
    void setSceneAsTextureShader(std::shared_ptr<SceneTextureShader> p_shader);
    /// \brief set the window title
    void setWindowTitle(const std::string &str);
    /// \brief enable/disable VSync
    virtual void setVSync(bool sync) override;
    virtual void registerScene(std::shared_ptr<Scene> p_scene, RenderTargetType p_target, const std::string &p_fboName);
    virtual void addFBO(const std::string &p_fboName,
                Texture *p_colorTex, Texture *p_depthTex,
                unsigned int p_width, unsigned int p_height);

    std::string windowTitle;
    Color defaultDiffuseColor;
    Color defaultAmbientColor;
    Color defaultSpecularColor;

protected:
    virtual void initRenderingCapabilities() override;
    virtual void initResources() override;
    virtual void initRenderingContext() override;
    virtual void destroyRenderingContext() override;
    /// \brief Cleanup function called on exit to ensure resources are cleaned up
    virtual void cleanUp() override;
    /// \brief Renders the internal sceneList
    void renderSceneList();

    virtual void processViewerOptions() override;
    virtual void processWindowEvents() override;
    virtual void renderToScreen(const RenderOperation &p_rop) override;
    virtual void renderToFBO(const RenderOperation &p_rop) override;
    virtual void initFboListItems();
    virtual void destroyFboListItems();
    virtual void setToDefaults() override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void renderTextureOnView() override;
    /// \brief  event handler
    virtual void handleEvent(std::shared_ptr<core::Event> p_event) override;
    /// \brief processes an SFML event
    void processSFMLEvents(const sf::Event& p_event);
};

#endif
