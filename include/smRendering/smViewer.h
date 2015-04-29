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

// GLFW includes
#include "GLFW/glfw3.h"

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smShader/smShader.h"
#include "smCore/smScene.h"
#include "smCore/smDispatcher.h"
#include "smCore/smModule.h"
#include "smCore/smStaticSceneObject.h"
#include "smUtilities/smGLUtils.h"
#include "smRendering/smVBO.h"
#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smFemSceneObject.h"
#include "smUtilities/smDataStructures.h"
#include "smSimulators/smStylusObject.h"
#include "smCore/smDoubleBuffer.h"
#include "smRendering/smFrameBuffer.h"
#include "smRendering/smCamera.h"
#include "smEvent/smEventHandler.h"


//forward declaration
class smSDK;
class smOpenGLWindowStream;
class smMetalShader;
class smSceneTextureShader;

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
class smViewer : public smModule
{
protected:
    std::vector<std::shared_ptr<smCoreClass>> objectList;
    std::vector<smRenderOperation> renderOperations;
    std::vector<smFboListItem> fboListItems;

    static std::shared_ptr<smtk::Event::smEventHandler> eventHandler;

    std::shared_ptr<smErrorLog> log;
    smInt unlimitedFPSVariableChanged;
    smBool unlimitedFPSEnabled;
    smInt screenResolutionWidth;
    smInt screenResolutionHeight;
    friend class smSDK;

public:

    static void keyboardEventTrigger(GLFWwindow*, int, int, int, int);
    static void mouseButtonEventTrigger(GLFWwindow*, int, int, int);
    static void mouseMoveEventTrigger(GLFWwindow*, double, double);

    smRenderingStageType renderStage;

    GLFWwindow* window;

    std::shared_ptr<smOpenGLWindowStream> windowOutput;
    /// \brief Viewer settings
    smUInt viewerRenderDetail;

    smInt height(void);
    smInt width(void);
    smFloat aspectRatio(void);
    /// \brief disable vSync
    void setUnlimitedFPS(smBool p_enableFPS);
    /// \brief default constructor
    smViewer();
    /// \brief initialization for viewer
    virtual  void init();
    /// \brief for exit viewer
    void exitViewer();
    /// \brief add object for rendering
    void addObject(std::shared_ptr<smCoreClass> object);
    /// \brief add text for display
    void addText(smString p_tag);
    /// \brief update text
    void updateText(smString p_tag, smString p_string);
    void updateText(smInt p_handle, smString p_string);
    /// \brief change window resolution
    void setScreenResolution(smInt p_width, smInt p_height);
    /// \brief set scene as texture
    void setSceneAsTextureShader(std::shared_ptr<smSceneTextureShader> p_shader);
    /// \brief set the window title
    void setWindowTitle(smString);
    /// \brief Registers a scene for rendering with the viewer
    void registerScene(std::shared_ptr<smScene> p_scene, smRenderTargetType p_target, const smString &p_fboName);
    /// \brief Adds an FBO to the viewer to allow rendering to it.
    ///
    /// \detail The FBO will be created an initialized in the viewer.
    ///
    /// \param p_fboName String to reference the FBO by
    /// \param p_colorTex A texture that will contain the fbo's color texture.
    /// \param p_depthTex A texture that will contain the fbo's depth texture.
    /// \param p_width The width of the fbo
    /// \param p_height The height of the fbo
    void addFBO(const smString &p_fboName,
                smTexture *p_colorTex, smTexture *p_depthTex,
                smUInt p_width, smUInt p_height);
    smString windowTitle;
    smColor defaultDiffuseColor;
    smColor defaultAmbientColor;
    smColor defaultSpecularColor;

protected:
    /// \brief Initializes OpenGL capabilities and flags
    void initGLCaps();
    /// \brief Initializes the internal objectList
    void initObjects(smDrawParam p_param);
    /// \brief Initializes FBOs, textures, shaders and VAOs
    void initResources(smDrawParam p_param);
    /// \brief Initializes scenes in the sceneList
    void initScenes(smDrawParam p_param);
    /// \brief Initilizes the OpenGL context, and window containing it
    void initGLContext();
    /// \brief Cleans up after initGLContext()
    void destroyGLContext();
    /// \brief Cleanup function called on exit to ensure resources are cleaned up
    virtual void cleanUp();
    /// \brief Renders the internal sceneList
    void renderSceneList(smDrawParam p_param);
    /// \brief Processes a render operation
    void processRenderOperation(const smRenderOperation &p_rop, smDrawParam p_param);
    /// \brief Processes viewerRenderDetail options
    void processViewerOptions();
    /// \brief Renders the render operation to screen
    virtual void renderToScreen(const smRenderOperation &p_rop, smDrawParam p_param);
    /// \brief Renders the render operation to an FBO
    void renderToFBO(const smRenderOperation &p_rop, smDrawParam p_param);
    /// \brief Initializes the FBOs in the FBO list
    void initFboListItems();
    /// \breif Destroys all the FBOs in the FBO list
    void destroyFboListItems();
    /// \brief
    void initDepthBuffer();
    /// \brief Set the color and other viewer defaults
    void setToDefaults();
    /// \brief called in the beginning of each frame
    virtual void beginFrame();
    /// \brief called in the end of each frame
    virtual void endFrame();
    /// \brief draw routines
    virtual void draw(const smDrawParam &){};
    virtual void draw();
    /// \brief adjust  rendering FPS
    void adjustFPS();
    /// \brief render depth texture for debugging
    void renderTextureOnView();
    /// \brief  event handler
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;
    /// \brief  launches the the viewer. don't call sdk will call this
    virtual void exec();
};

#endif
