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
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smFemSceneObject.h"
#include "smUtilities/smDataStructs.h"
#include "smSimulators/smStylusObject.h"
#include "smRendering/smLight.h"
#include "smCore/smDoubleBuffer.h"
#include "smRendering/smFrameBuffer.h"
#include "smShader/SceneTextureShader.h"
#include "smRendering/smCamera.h"


//forward declaration
class smSDK;
class smOpenGLWindowStream;
class MetalShader;

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
    smScene *scene; ///< The scene full of objects to render
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
class smViewer : public smModule, public smEventHandler
{
protected:
    std::vector<smCoreClass*> objectList;
    smIndiceArray<smLight*> *lights;
    std::vector<smRenderOperation> renderOperations;
    std::vector<smFboListItem> fboListItems;

    ///Vertex Buffer objects
    smVBO *vboDynamicObject;
    smVBO * vboStaticObject;
    smShader shader;
    smShader shader1;
    GLuint paramColor;
    smErrorLog *log;

    //virtual void drawSurfaceMeshTriangles(smSurfaceMesh *p_surfaceMesh,smRenderDetail *renderDetail);
    virtual void drawSurfaceMeshTriangles(smMesh *p_surfaceMesh, smRenderDetail *renderDetail);
    virtual void drawSMStaticObject(smStaticSceneObject *p_smPhsyObject);
    virtual void drawSurfaceMeshTrianglesVBO(smSurfaceMesh *p_surfaceMesh,
            smRenderDetail *renderDetail,
            smInt p_objectId,
            smVBOType p_VBOType);
    void drawSMDeformableObject(smPBDSurfaceSceneObject *p_smPhsyObject);
    void drawFemObject(smFemSceneObject *p_smFEM);
    void drawNormals(smMesh *p_mesh);
    friend class smSDK;
    smMatrix44f shadowMatrix;
    smColor shadowColor;

    ///Frame Buffer for Shadow rendering
    smFrameBuffer *fbo;
    smFrameBuffer *backfbo;
    smFrameBuffer *frontfbo;

    ///Shadow Shader
    smShader *shadow;
    smGLInt shadowMapUniform;
    void drawSmLight(smLight *light);
    smInt unlimitedFPSVariableChanged;
    smBool unlimitedFPSEnabled;
    smInt screenResolutionWidth;
    smInt screenResolutionHeight;

public:
    smRenderingStageType renderStage;

    GLFWwindow* window;
    smCamera camera;

    smInt height(void);
    smInt width(void);
    smFloat aspectRatio(void);

    ///if the camera motion is enabled from other external devices
    smBool enableCameraMotion;

    //it is public for now
    smOpenGLWindowStream *windowOutput;
    /// \brief Viewer settings
    smUInt viewerRenderDetail;
    /// \brief enable console display
    smBool consoleDisplay;
    /// \brief camera collision enabled/disabled
    bool isCameraCollided;
    /// \brief addlight
    smInt addLight(smLight *p_light);
    /// \brief set light given with light ID
    smBool setLight(smInt lightId, smLight *p_light);
    /// \brief refresh lights. updates light  position based on the gl matrix
    void refreshLights();
    /// \brief update light information
    smBool updateLight(smInt p_lightId, smLight *p_light);
    void setLightPos(smInt p_lightId, smLightPos p_pos);
    void setLightPos(smInt p_lightId, smLightPos p_pos, smVec3f p_direction);
    /// \brief disable vSync
    void setUnlimitedFPS(smBool p_enableFPS);
    /// \brief default constructor
    smViewer();
    /// \brief initialization for viewer
    virtual  void init();
    /// \brief for exit viewer
    void exitViewer();
    /// \brief add object for rendering
    void addObject(smCoreClass *object);
    /// \brief add text for display
    void addText(smString p_tag);
    /// \brief update text
    void updateText(smString p_tag, smString p_string);
    void updateText(smInt p_handle, smString p_string);
    /// \brief change window resolution
    void setScreenResolution(smInt p_width, smInt p_height);
    /// \brief set scene as texture
    void setSceneAsTextureShader(SceneTextureShader *p_shader);
    /// \brief set the window title
    void setWindowTitle(smString);
    /// \brief Registers a scene for rendering with the viewer
    void registerScene(smScene *p_scene, smRenderTargetType p_target, const smString &p_fboName);
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
    /// \brief if you want to put offset in camera movement. The following is the properties
    smDouble offsetAngle_Direction;
    smDouble offsetAngle_UpDirection;
    smDouble offsetAngle_rightDirection;
    smVec3d finalDeviceCameraDir;
    smVec3d finalDeviceUpCameraDir;
    smVec3d finalDeviceRightCameraDir;

protected:
    /// \brief Initializes OpenGL capabilities and flags
    void initGLCaps();
    /// \brief Initializes lights for rendering
    void initLights();
    /// \brief Initializes the internal objectList
    void initObjects(smDrawParam p_param);
    /// \brief Initializes FBOs, textures, shaders and VAOs
    void initResources(smDrawParam p_param);
    /// \brief Initializes scenes in the sceneList
    void initScenes(smDrawParam p_param);
    /// \brief Initializes the viewer's camera
    void initCamera();
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
    /// \brief Texture for shadow mapping
    smGLUInt shadowMapTexture;
    /// \brief called in the beginning of each frame
    virtual void beginFrame();
    /// \brief called in the end of each frame
    virtual void endFrame();
    /// \brief draw routines
    virtual void draw();
    /// \brief adjust  rendering FPS
    void adjustFPS();
    /// \brief draw with shadows enabled
    void drawWithShadows(smDrawParam &p_param);
    /// \brief render depth texture for debugging
    void renderTextureOnView();
    /// \brief draw console. legacy code
    void drawConsole();
    /// \brief  event handler
    void handleEvent(smEvent *p_event);
    /// \brief  enable attached lights
    void enableLights();
    /// \brief  scale for light drawing in the scene.
    smFloat lightDrawScale;
    //delete this..this is for demo..
    smVec3f hapticPosition;
    smVec3f  hapticForce;
    /// \brief  launches the the viewer. don't call sdk will call this
    virtual void exec();

public:
    /// \brief device camera position. This is used for manipulation of the camera with haptic device
    smVec3f deviceCameraPos;
    smVec3f deviceCameraDir;
    smVec3f deviceCameraUpDir;
    /// \brief  check if the camera is collided or not
    smBool  checkCameraCollisionWithScene();
    void addCollisionCheckMeshes(smMesh *mesh);
    /// \brief  stores the  meshes that the collision check  will be performed with camera.
    std::vector<smMesh*> collisionMeshes;
    /// \brief  camera effective radius
    smFloat cameraRadius;
    /// \brief   previous state is collided.internal use
    smBool prevState_collided;
    /// \brief  last camera position
    smVec3f lastCamPos;
    /// \brief   check camera is collided
    smBool checkCameraCollision;
    /// \brief  camera collision callback
    smCameraCollisionInterface *notes_cameraCollision;
    /// \brief   for dynamic reflection
    MetalShader *renderandreflection;
};

#endif
