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

#ifndef SMVIEWERBASE_H
#define SMVIEWERBASE_H

// SimMedTK includes
#include "Config.h"
#include "Scene.h"
#include "Dispatcher.h"
#include "Module.h"
#include "StaticSceneObject.h"
#include "DataStructures.h"
#include "Pipe.h"

// Forward declaration
class smSDK;
class smOpenGLWindowStream;
class smMetalShader;
class smSceneTextureShader;
class smFrameBuffer;
class smTexture;

class smCameraCollisionInterface
{
public:
    virtual bool checkCameraCollision() = 0;
};

enum smRenderingStageType
{
    SMRENDERSTAGE_SHADOWPASS,
    SMRENDERSTAGE_DPMAPPASS,
    SMRENDERSTAGE_CUSTOMPASS,
    SMRENDERSTAGE_FINALPASS
};

enum smRenderTargetType
{
    SMRENDERTARGET_SCREEN,
    SMRENDERTARGET_FBO
};

/// \brief Describes what to render and where the rendering should take place
struct smRenderOperation
{
    smRenderOperation();
    std::shared_ptr<smScene> scene; ///< The scene full of objects to render
    smFrameBuffer *fbo; ///< Only required if rendering to FBO, specifies the FBO to render to
    smString fboName; ///< Only required if rendering to FBO, named reference to look up the FBO pointer
    smRenderTargetType target; ///< Specifies where the rendered result should be placed see smRenderTargetType
};

struct smFboListItem
{
    smString fboName; ///< String identification
    smFrameBuffer* fbo; ///< The FBO pointer
    smTexture *depthTex; ///< The FBO depth texture pointer
    smTexture *colorTex; ///< The FBO color texture pointer
    smUInt width; ///< The width of the FBO
    smUInt height; ///< The height of the FBO
};

/// \brief Handles all rendering routines.
class smViewerBase : public smModule
{
protected:
    std::vector<std::shared_ptr<smCoreClass>> objectList;
    std::vector<smRenderOperation> renderOperations;
    std::vector<smFboListItem> fboListItems;
    std::shared_ptr<smErrorLog> log;
    smInt unlimitedFPSVariableChanged;
    smBool unlimitedFPSEnabled;
    smInt screenResolutionWidth;
    smInt screenResolutionHeight;
    friend class smSDK;

public:

    smRenderingStageType renderStage;

    ///if the camera motion is enabled from other external devices
    smBool enableCameraMotion;

    /// \brief Viewer settings
    smUInt viewerRenderDetail;

    smFloat globalAxisLength;

    virtual smInt height(void);
    virtual smInt width(void);
    virtual smFloat aspectRatio(void);
    /// \brief disable vSync
    virtual void setUnlimitedFPS(smBool p_enableFPS);
    /// \brief default constructor
    smViewerBase();
    /// \brief initialization for viewer
    virtual void init() override;
    /// \brief for exit viewer
    virtual void exitViewer() = 0;
    /// \brief add object for rendering
    virtual void addObject(std::shared_ptr<smCoreClass> object);
    /// \brief add text for display
    virtual void addText(smString p_tag) = 0;
    /// \brief update text
    virtual void updateText(smString p_tag, smString p_string) = 0;
    virtual void updateText(smInt p_handle, smString p_string) = 0;
    /// \brief change window resolution
    virtual void setScreenResolution(smInt p_width, smInt p_height);
    /// \brief set the window title
    virtual void setWindowTitle(const smString &str);
    /// \brief enable/disable VSync
    virtual void setVSync(bool sync) = 0;
    /// \brief Registers a scene for rendering with the viewer
    virtual void registerScene(std::shared_ptr<smScene> p_scene, smRenderTargetType p_target, const smString &p_fboName);
    /// \brief Adds an FBO to the viewer to allow rendering to it.
    ///
    /// \detail The FBO will be created an initialized in the viewer.
    ///
    /// \param p_fboName String to reference the FBO by
    /// \param p_colorTex A texture that will contain the fbo's color texture.
    /// \param p_depthTex A texture that will contain the fbo's depth texture.
    /// \param p_width The width of the fbo
    /// \param p_height The height of the fbo
    virtual void addFBO(
      const smString &p_fboName,
      smTexture *p_colorTex, smTexture *p_depthTex,
      smUInt p_width, smUInt p_height);

    virtual void setGlobalAxisLength(const smFloat len);
    smString windowTitle;
    smColor defaultDiffuseColor;
    smColor defaultAmbientColor;
    smColor defaultSpecularColor;

protected:
    /// \brief Initializes rendering system (e.g., OpenGL) capabilities and flags
    virtual void initRenderingCapabilities() = 0;
    /// \brief Initializes the internal objectList
    virtual void initObjects();
    /// \brief Initializes FBOs, textures, shaders and VAOs
    virtual void initResources() = 0;
    /// \brief Initializes scenes in the sceneList
    virtual void initScenes();
    /// \brief Initilizes the rendering system (e.g., OpenGL) context, and window containing it
    virtual void initRenderingContext() = 0;
    /// \brief Cleans up after initGLContext()
    virtual void destroyRenderingContext() = 0;
    /// \brief Cleanup function called on exit to ensure resources are cleaned up
    virtual void cleanUp();
    /// \brief Renders the internal sceneList
    void renderSceneList();
    /// \brief Processes a render operation
    virtual void processRenderOperation(const smRenderOperation &p_rop);
    /// \brief Processes viewerRenderDetail options
    virtual void processViewerOptions() = 0;
    /// \brief Process window events and render as the major part of an event loop
    virtual void processWindowEvents() = 0;
    /// \brief Renders the render operation to screen
    virtual void renderToScreen(const smRenderOperation &p_rop) = 0;
    /// \brief Renders the render operation to an FBO
    virtual void renderToFBO(const smRenderOperation &p_rop) = 0;
    /// \brief Set the color and other viewer defaults
    virtual void setToDefaults() = 0;
    /// \brief draw routines
    virtual void render();
    /// \brief adjust  rendering FPS
    void adjustFPS();
    /// \brief render depth texture for debugging
    virtual void renderTextureOnView() = 0;
    /// \brief initialize, run the event loop (processWindowEvents) and clean up.
    virtual void exec();
};

#endif // SMVIEWERBASE_H
