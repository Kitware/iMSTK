// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_VIEWERBASE_H
#define CORE_VIEWERBASE_H

// iMSTK includes
#include "Core/Config.h"
#include "Core/Scene.h"
#include "Core/Dispatcher.h"
#include "Core/Module.h"
#include "Core/DataStructures.h"

namespace imstk {

enum RenderingStageType
{
    IMSTK_RENDERSTAGE_SHADOWPASS,
    IMSTK_RENDERSTAGE_DPMAPPASS,
    IMSTK_RENDERSTAGE_CUSTOMPASS,
    IMSTK_RENDERSTAGE_FINALPASS
};

enum RenderTargetType
{
    IMSTK_RENDERTARGET_SCREEN,
    IMSTK_RENDERTARGET_FBO
};

/// \brief Describes what to render and where the rendering should take place
struct RenderOperation
{
    RenderOperation();
    RenderOperation(std::shared_ptr<Scene> s,  const RenderTargetType &t, const std::string &fbName) :
        scene(s),
        target(t),
        fboName(fbName)
    {}
    std::shared_ptr<Scene> scene; ///< The scene full of objects to render
    RenderTargetType target; ///< Specifies where the rendered result should be placed see RenderTargetType
    std::string fboName; ///< Only required if rendering to FBO, named reference to look up the FBO pointer
};

/// \brief Handles all rendering routines.
class ViewerBase : public Module
{
protected:
    std::vector<std::shared_ptr<CoreClass>> objectList;
    std::vector<RenderOperation> renderOperations;
    std::shared_ptr<ErrorLog> log;
    int unlimitedFPSVariableChanged;
    bool unlimitedFPSEnabled;
    int screenResolutionWidth;
    int screenResolutionHeight;

public:

    RenderingStageType renderStage;

    ///if the camera motion is enabled from other external devices
    bool enableCameraMotion;

    /// \brief Viewer settings
    unsigned int viewerRenderDetail;

    float globalAxisLength;

    /// \brief for exit viewer
    virtual void exitViewer() = 0;
    /// \brief add text for display
    virtual void addText(std::string p_tag) = 0;
    /// \brief update text
    virtual void updateText(std::string p_tag, std::string p_string) = 0;
    virtual void updateText(int p_handle, std::string p_string) = 0;
    /// \brief enable/disable VSync
    virtual void setVSync(bool sync) = 0;

    virtual int height(void);
    virtual int width(void);
    virtual float aspectRatio(void);
    /// \brief disable vSync
    virtual void setUnlimitedFPS(bool p_enableFPS);
    /// \brief default constructor
    ViewerBase();
    /// \brief initialization for viewer
    virtual bool init() override;
    /// \brief add object for rendering
    virtual void addObject(std::shared_ptr<CoreClass> object);
    /// \brief change window resolution
    virtual void setScreenResolution(int p_width, int p_height);
    /// \brief set the window title
    virtual void setWindowTitle(const std::string &str);
    /// \brief Registers a scene for rendering with the viewer
    virtual void registerScene(std::shared_ptr<Scene> scene, RenderTargetType target = IMSTK_RENDERTARGET_SCREEN, const std::string &fboName = "");

    virtual void setGlobalAxisLength(const float len);

    virtual const bool &isValid()
    {
        return this->valid;
    }

    void setViewerRenderDetail(const unsigned int newRenderDetail);

    std::string windowTitle;
    Color defaultDiffuseColor;
    Color defaultAmbientColor;
    Color defaultSpecularColor;
    bool valid;
    /// \brief initialize, run the event loop (processWindowEvents) and clean up.
    virtual void exec() override;

protected:
    /// \brief Renders the render operation to screen
    virtual void renderToScreen(const RenderOperation &p_rop) = 0;
    /// \brief Renders the render operation to an FBO
    virtual void renderToFBO(const RenderOperation &p_rop) = 0;
    /// \brief Initializes rendering system (e.g., OpenGL) capabilities and flags
    virtual void initRenderingCapabilities() = 0;
    /// \brief Initilizes the rendering system (e.g., OpenGL) context, and window containing it
    virtual void initRenderingContext() = 0;
    /// \brief Cleans up after initGLContext()
    virtual void destroyRenderingContext() = 0;
    /// \brief Initializes FBOs, textures, shaders and VAOs
    virtual void initResources() = 0;
    /// \brief Processes viewerRenderDetail options
    virtual void processViewerOptions() = 0;
    /// \brief Process window events and render as the major part of an event loop
    virtual void processWindowEvents() = 0;
    /// \brief Set the color and other viewer defaults
    virtual void setToDefaults() = 0;
    /// \brief render depth texture for debugging
    virtual void renderTextureOnView() = 0;

    /// \brief Initializes the internal objectList
    virtual void initObjects();
    /// \brief Initializes scenes in the sceneList
    virtual void initScenes();
    /// \brief Cleanup function called on exit to ensure resources are cleaned up
    virtual void cleanUp();
    /// \brief Renders the internal sceneList
    void renderSceneList();
    /// \brief Processes a render operation
    virtual void processRenderOperation(const RenderOperation &p_rop);
    /// \brief draw routines
    virtual void render();
    /// \brief adjust  rendering FPS
    void adjustFPS();
};

}

#endif // CORE_VIEWERBASE_H
