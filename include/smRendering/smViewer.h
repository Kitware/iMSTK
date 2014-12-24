/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#ifndef SMVIEWER_H
#define SMVIEWER_H
#include <QGLViewer/qglviewer.h>
#include <GL/glut.h>
#include <QDrag>
#include <QUrl>
#include <QDialog>

#include "smCore/smConfig.h"
#include "smShader/smShader.h"
#include "smCore/smScene.h"
#include "smCore/smDispatcher.h"
#include "smCore/smModule.h"
#include "smCore/smStaticSceneObject.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smMatrix44.h"
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
#include "smUtilities/smVec3.h"
#include "smShader/SceneTextureShader.h"

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

///Viewer Class. Right now it is of type QGLViewer, which could be changed later on if needed.
class smViewer : public QGLViewer, public smModule, public smEventHandler
{
protected:
    vector<smCoreClass*> objectList;
    smIndiceArray<smLight*> *lights;

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
    smMatrix44<smFloat> shadowMatrix;
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
    smBool boostViewer;

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
    void setLightPos(smInt p_lightId, smLightPos p_pos, smVec3<smFloat> p_direction);
    /// \brief disable vSync
    void setUnlimitedFPS(smBool p_enableFPS);
    /// \brief constructor. requires error log.
    smViewer(smErrorLog *log);
    /// \brief initialization for viewer
    virtual  void init();
    /// \brief for exit viewer
    void exitViewer();
    /// \brief add object for rendering
    void addObject(smCoreClass *object);
    /// \brief add text for display
    void addText(QString p_tag);
    /// \brief update text
    void updateText(QString p_tag, QString p_string);
    void updateText(smInt p_handle, QString p_string);
    /// \brief change window resolution
    void setScreenResolution(smInt p_width, smInt p_height);
    /// \brief set scene as texture
    void setSceneAsTextureShader(SceneTextureShader *p_shader);
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
    /// \brief render scene objects
    virtual void renderScene(smDrawParam p_param);
    /// \brief draw with shadows enabled
    void drawWithShadows(smDrawParam &p_param);
    /// \brief render depth texture for debugging
    void renderTextureOnView();
    /// \brief draw console. legacy code
    void drawConsole();
    /// \brief key press event. This is called when key is pressed
    void keyPressEvent(QKeyEvent *e);
    /// \brief  event handler
    void handleEvent(smEvent *p_event);
    /// \brief  enable attached lights
    void enableLights();
    /// \brief  scale for light drawing in the scene.
    smFloat lightDrawScale;
    //delete this..this is for demo..
    smVec3<smDouble> hapticPosition;
    smVec3<smDouble>  hapticForce;
    /// \brief  drop an object
    void dropEvent(QDropEvent *event);
    /// \brief  drag an object
    void dragEnterEvent(QDragEnterEvent *event);
    /// \brief  launches the the viewer. don't call sdk will call this
    virtual void exec();

public:
    /// \brief device camera position. This is used for manipulation of the camera with haptic device
    smVec3<smDouble> deviceCameraPos;
    smVec3<smDouble> deviceCameraDir;
    smVec3<smDouble> deviceCameraUpDir;
    /// \brief  save camera position
    qglviewer::Camera prevCamera;
    /// \brief  check if the camera is collided or not
    smBool  checkCameraCollisionWithScene();
    void addCollisionCheckMeshes(smMesh *mesh);
    /// \brief  stores the  meshes that the collision check  will be performed with camera.
    vector<smMesh*> collisionMeshes;
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
    /// \brief  forms that will be lauched for GUI
    vector<QDialog *>forms;
    /// \brief   adding form  the viewer.
    void addForm(QDialog *p_form);
};

#endif
