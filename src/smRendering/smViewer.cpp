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

#include "smCore/smConfig.h"
#include "smCore/smSDK.h"
#include "smUtilities/smIOStream.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include "smShader/smShader.h"
#include "smShader/smMetalShader.h"
#include "smShader/smSceneTextureShader.h"
#include "smUtilities/smDataStructures.h"
#include "smRendering/smVBO.h"
#include "smRendering/smVAO.h"
#include "smExternal/tree.hh"

#ifdef SIMMEDTK_OPERATINGSYSTEM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glxew.h>
#include <GL/glxext.h>
#include <GL/glx.h>
#endif

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
#endif

void SetVSync(bool sync)
{
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(sync);
    }

#elif defined(SIMMEDTK_OPERATINGSYSTEM_LINUX)
    Display *dpy = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();

    if (drawable && dpy)
    {
        glXSwapIntervalEXT(dpy, drawable, sync);
    }

#endif
}

static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods)
{
    smEvent *eventKeyboard;
    smSDK *sdk = smSDK::getInstance();
    assert(sdk);
    smViewer *viewer = sdk->getViewerInstance();
    assert(viewer);
    smKeyboardEventData* kbData = nullptr;

    eventKeyboard = new smEvent(); //Need to handle failure to allocate
    eventKeyboard->eventType = SIMMEDTK_EVENTTYPE_KEYBOARD;
    eventKeyboard->senderId = viewer->getModuleId();
    eventKeyboard->senderType = SIMMEDTK_SENDERTYPE_MODULE;
    eventKeyboard->data = new smKeyboardEventData(); //Need to handle failure to allocate
    kbData = reinterpret_cast<smKeyboardEventData*>(eventKeyboard->data);
    kbData->keyBoardKey = GLFWKeyToSmKey(key);
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
    {
        kbData->pressed = true;
    }
    else
    {
        kbData->pressed = false;
    }

    kbData->modKeys = smModKey::none;
    if (mods & GLFW_MOD_SHIFT)
        kbData->modKeys |= smModKey::shift;
    if (mods & GLFW_MOD_CONTROL)
        kbData->modKeys |= smModKey::control;
    if (mods & GLFW_MOD_ALT)
        kbData->modKeys |= smModKey::alt;
    if (mods & GLFW_MOD_SUPER)
        kbData->modKeys |= smModKey::super;

    sdk->getEventDispatcher()->sendEventAndDelete(eventKeyboard);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    smSDK *sdk = smSDK::getInstance();
    assert(sdk);
    smViewer *viewer = sdk->getViewerInstance();
    assert(viewer);
    smMouseButtonEventData* mbData = nullptr;

    smEvent *eventMouseButton = new smEvent(); //Need to handle failure to allocate
    eventMouseButton->eventType = SIMMEDTK_EVENTTYPE_MOUSE_BUTTON;
    eventMouseButton->senderId = viewer->getModuleId();
    eventMouseButton->senderType = SIMMEDTK_SENDERTYPE_MODULE;
    eventMouseButton->data = new smMouseButtonEventData(); //Need to handle failure to allocate
    mbData = reinterpret_cast<smMouseButtonEventData*>(eventMouseButton->data);

    //Get the current cursor position
    glfwGetCursorPos(window, &(mbData->windowX), &(mbData->windowY));

    //
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        mbData->mouseButton = smMouseButton::Left;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        mbData->mouseButton = smMouseButton::Right;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        mbData->mouseButton = smMouseButton::Middle;
    else
        mbData->mouseButton = smMouseButton::Unknown;

    if (action == GLFW_PRESS)
        mbData->pressed = true;
    else
        mbData->pressed = false;

    sdk->getEventDispatcher()->sendEventAndDelete(eventMouseButton);
}

static void cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos)
{
    smSDK *sdk = smSDK::getInstance();
    assert(sdk);
    smViewer *viewer = sdk->getViewerInstance();
    assert(viewer);
    smMouseMoveEventData* mpData = nullptr;

    smEvent *eventMouseMove = new smEvent(); //Need to handle failure to allocate
    eventMouseMove->eventType = SIMMEDTK_EVENTTYPE_MOUSE_MOVE;
    eventMouseMove->senderId = viewer->getModuleId();
    eventMouseMove->senderType = SIMMEDTK_SENDERTYPE_MODULE;
    eventMouseMove->data = new smMouseMoveEventData(); //Need to handle failure to allocate
    mpData = reinterpret_cast<smMouseMoveEventData*>(eventMouseMove->data);

    mpData->windowX = xpos;
    mpData->windowY = ypos;

    sdk->getEventDispatcher()->sendEventAndDelete(eventMouseMove);
}

smRenderOperation::smRenderOperation()
{
    fbo = NULL;
    scene = NULL;
    fboName = "";
}

smViewer::smViewer()
{
    type = SIMMEDTK_SMVIEWER;
    viewerRenderDetail = SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;

    defaultAmbientColor.setValue(0.1, 0.1, 0.1, 1.0);
    defaultDiffuseColor.setValue(0.8, 0.8, 0.8, 1.0);
    defaultSpecularColor.setValue(0.9, 0.9, 0.9, 1.0);

    this->log = NULL;
    windowOutput = new smOpenGLWindowStream();

    unlimitedFPSEnabled = false;
    unlimitedFPSVariableChanged = 1;
    screenResolutionWidth = 1680;
    screenResolutionHeight = 1050;
}

///affects the framebuffer size and depth buffer size
void smViewer::setScreenResolution(smInt p_width, smInt p_height)
{
    this->screenResolutionHeight = p_height;
    this->screenResolutionWidth = p_width;
}

void smViewer::setUnlimitedFPS(smBool p_enableFPS)
{

    unlimitedFPSEnabled = p_enableFPS;
    unlimitedFPSVariableChanged++;
}

void smViewer::initGLCaps()
{
    //use multiple fragment samples in computing the final color of a pixel
    glEnable(GL_MULTISAMPLE);
    //do depth comparisons and update the depth buffer
    glEnable(GL_DEPTH_TEST);
    //cull polygons based on their winding in window coordinates
    glEnable(GL_CULL_FACE);
    //DEPRECIATED AS OF v3.3 have one or more material parameters
    // track the current color
    glDisable(GL_COLOR_MATERIAL);
    //DEPRECIATED AS OF v3.3 If enabled and no vertex shader is active,
    // use the current lighting parameters to compute the vertex color or index
    glEnable(GL_LIGHTING);
    //DEPRECIATED AS OF v3.3 If enabled and no vertex shader is active,
    // normal vectors are normalized to unit length after transformation and
    // before lighting
    glEnable(GL_NORMALIZE);

    //Fill the face of the polygon for all front and back facing polygons
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    glFrontFace(GL_CCW);
    //DEPRECIATED AS OF v3.3 Smooth shading
    glShadeModel(GL_SMOOTH);
    //DEPRECIATED AS OF v3.3 Specifies the specular component of a material
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 50);
}

void smViewer::initObjects(smDrawParam p_param)
{
    for (size_t i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->getType() != SIMMEDTK_SMSHADER)
        {
            objectList[i]->initDraw(p_param);
        }
        else
        {
            continue;
        }
    }
}

void smViewer::initResources(smDrawParam p_param)
{
    smTextureManager::initGLTextures();
    smShader::initGLShaders(p_param);
    smVAO::initVAOs(p_param);

    initFboListItems();
}

void smViewer::initScenes ( smDrawParam p_param )
{
    smSceneObject *sceneObject;
    smScene *scene;
    smScene::smSceneIterator sceneIter;
    //traverse all the scene and the objects in the scene
    for ( size_t i = 0; i < sceneList.size(); i++ )
    {
        scene = sceneList[i];
        scene->registerForScene(this);
        scene->initLights();
        sceneIter.setScene(scene, this);

        for ( smInt j = sceneIter.start(); j < sceneIter.end(); j++ )
        {
            //sceneObject=scene->sceneObjects[j];
            sceneObject = sceneIter[j];

            //initialize the custom Render if there is any
            if ( sceneObject->customRender != NULL && sceneObject->getType() != SIMMEDTK_SMSHADER )
            {
                sceneObject->customRender->initDraw ( p_param );
            }
            sceneObject->initDraw ( p_param );
        }//object traverse
    }//scene traverse
}

void smViewer::initGLContext()
{
    int count;

    // Init GLFW(OpenGL context)
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    // Init the rest of GLFW
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
    {
        GLFWmonitor** glfwMonitors = glfwGetMonitors(&count);
        window = glfwCreateWindow(screenResolutionWidth, screenResolutionHeight,
            windowTitle.c_str(), glfwMonitors[count - 1], NULL);
    }
    else
    {
        window = glfwCreateWindow(screenResolutionWidth, screenResolutionHeight,
            windowTitle.c_str(), NULL, NULL);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Init GLEW
    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong.
         * Most likely an OpenGL context is not created yet */
        std::cout << "Error:" << glewGetErrorString(err) << "\n";
        assert(false);
    }
}

///initialization of the viewer module
void smViewer::init()
{
    static smDrawParam param;

    if (isInitialized)
    {
        return;
    }

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    this->initGLContext();
    this->initGLCaps();
    this->initObjects(param);
    this->initResources(param);
    this->initScenes(param);

    isInitialized = true;
}

void smViewer::destroyGLContext()
{
    //Shutdown glfw
    glfwDestroyWindow(window);
    glfwTerminate();
}

void smViewer::renderTextureOnView()
{

    glPushAttrib(GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_LIGHTING_BIT);
    glUseProgramObjectARB(0);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glActiveTextureARB(GL_TEXTURE0);
    smTextureManager::activateTexture("depth");
    glEnable(GL_TEXTURE_2D);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2d(1, 0);
    glVertex3f(1, 0, 0);
    glTexCoord2d(1, 1);
    glVertex3f(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3f(0, 1.0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void smViewer::addFBO(const smString &p_fboName,
                      smTexture *p_colorTex,
                      smTexture *p_depthTex,
                      smUInt p_width, smUInt p_height)
{
    smFboListItem item;

    item.fboName = p_fboName;
    item.width = p_width;
    item.height = p_height;
    if (p_colorTex)
    {
        item.colorTex = p_colorTex;
    }
    if (p_depthTex)
    {
        item.depthTex = p_depthTex;
    }

    this->fboListItems.push_back(item);
}

void smViewer::initFboListItems()
{
    for (size_t i = 0; i < this->fboListItems.size(); i++)
    {
        smFboListItem *item = &fboListItems[i];
        item->fbo = new smFrameBuffer();
        item->fbo->setDim(item->width, item->height);
        if (item->colorTex)
        {
            item->fbo->attachColorTexture(item->colorTex, 0);
        }
        if (item->depthTex)
        {
            item->fbo->attachDepthTexture(item->depthTex);
        }
        for (size_t j = 0; j < renderOperations.size(); j++)
        {
            if (renderOperations[j].fboName == item->fboName)
            {
                renderOperations[j].fbo = item->fbo;
            }
        }
    }
}

void smViewer::destroyFboListItems()
{
    for (size_t i = 0; i < this->fboListItems.size(); i++)
    {
        if (fboListItems[i].fbo)
        {
            delete (fboListItems[i].fbo);
            fboListItems[i].fbo = NULL;
        }
    }
}

void smViewer::renderSceneList(smDrawParam p_param)
{
    smScene::smSceneIterator sceneIter;

    //this routine is for rendering. if you implement different objects add rendering accordingly. Viewer knows to draw
    //only current objects and their derived classes
    for (size_t sceneIndex = 0; sceneIndex < sceneList.size(); sceneIndex++)
    {
        smGLRenderer::renderScene(sceneList[sceneIndex], p_param);
    }
}

void smViewer::processViewerOptions()
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FADEBACKGROUND)
    {
        smGLUtils::fadeBackgroundDraw();
    }
}

void smViewer::processRenderOperation(const smRenderOperation &p_rop, smDrawParam p_param)
{
    switch (p_rop.target)
    {
    case SMRENDERTARGET_SCREEN:
        renderToScreen(p_rop, p_param);
        break;
    case SMRENDERTARGET_FBO:
        renderToFBO(p_rop, p_param);
        break;
    default:
        assert(0);
    }
}

void smViewer::renderToFBO(const smRenderOperation &p_rop, smDrawParam p_param)
{
    assert(p_rop.fbo);
    //Enable FBO for rendering
    p_rop.fbo->enable();
    //Setup Viewport & Clear buffers
    glViewport(0, 0, p_rop.fbo->getWidth(), p_rop.fbo->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //Enable lights
    p_rop.scene->enableLights();
    processViewerOptions();
    //Render Scene
     smGLRenderer::renderScene(p_rop.scene, p_param);
    //Disable FBO
    p_rop.fbo->disable();
}

void smViewer::renderToScreen(const smRenderOperation &p_rop, smDrawParam p_param)
{
    //Setup Viewport & Clear buffers
    glViewport(0, 0, this->width(), this->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //Enable lights
    p_rop.scene->enableLights();
    processViewerOptions();
    //Render Scene
    smGLRenderer::renderScene(p_rop.scene, p_param);
}

void smViewer::registerScene(smScene *p_scene,
                             smRenderTargetType p_target,
                             const smString &p_fboName)
{
    smRenderOperation rop;

    //sanity checks
    assert(p_scene);
    if (p_target == SMRENDERTARGET_FBO)
    {
        assert(p_fboName != "");
    }

    rop.target = p_target;
    rop.scene = p_scene;

    rop.fboName = p_fboName;

    p_scene->registerForScene(this);
    renderOperations.push_back(rop);
}

inline void smViewer::setToDefaults()
{

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiffuseColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpecularColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmbientColor.toGLColor());
    glColor4fv(defaultDiffuseColor.toGLColor());
}

inline void smViewer::adjustFPS()
{

    static smInt _unlimitedFPSVariableChanged = 0;
    smInt unlimitedFPSVariableChangedCurrent;
    unlimitedFPSVariableChangedCurrent = unlimitedFPSVariableChanged;

    if (_unlimitedFPSVariableChanged < unlimitedFPSVariableChangedCurrent)
    {
        _unlimitedFPSVariableChanged = unlimitedFPSVariableChangedCurrent;

        if (unlimitedFPSEnabled)
        {
            SetVSync(false);
        }
        else
        {
            SetVSync(true);
        }
    }
}

///main drawing routine for Rendering of all objects in the scene
void smViewer::draw()
{

    static smDrawParam param;

    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    param.rendererObject = this;
    param.caller = this;
    param.data = NULL;

    beginModule();

    adjustFPS();

    for (size_t i = 0; i < objectList.size(); i++)
    {
        objectList[i]->draw(param);
    }

    for (int i = 0; i < renderOperations.size(); i++)
    {
        processRenderOperation(renderOperations[i], param);
    }

    for (size_t i = 0; i < objectList.size(); i++)
    {
        objectList[i]->draw(param);
    }

    endModule();
}

///called by the module before each frame starts
void smViewer::beginFrame()
{
    glfwMakeContextCurrent(window);
}

///called by the module after each frame ends
void smViewer::endFrame()
{
    glfwSwapBuffers(window);
}

void smViewer::addObject(smCoreClass *object)
{

    smSDK::addRef(object);
    objectList.push_back(object);
}

void smViewer::handleEvent ( smEvent *p_event )
{
}

void smViewer::addText(smString p_tag)
{

    windowOutput->addText(p_tag, smString(""));
}

void smViewer::updateText(smString p_tag, smString p_string)
{

    windowOutput->updateText(p_tag, p_string);
}
void smViewer::updateText(smInt p_handle, smString p_string)
{

    windowOutput->updateText(p_handle, p_string);
}

void smViewer::setWindowTitle(smString str)
{
    windowTitle = str;
}

void smViewer::exec()
{
    // Init the viewer
    this->init();

    while (!terminateExecution) {
        this->draw();
        glfwPollEvents();
    }

    cleanUp();
}

void smViewer::cleanUp()
{
    destroyFboListItems();
    destroyGLContext();

    //Must be set when all cleanup is done
    terminationCompleted = true;
}

smInt smViewer::height(void)
{
    return screenResolutionHeight;
}

smInt smViewer::width(void)
{
    return screenResolutionWidth;
}

smFloat smViewer::aspectRatio(void)
{
    return screenResolutionHeight / screenResolutionWidth;
}
