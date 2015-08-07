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

#include "Rendering/OpenGLViewer.h"

#include "Core/EventHandler.h"
#include "Core/IOStream.h"
#include "Core/Factory.h"
#include "Event/KeyboardEvent.h"
#include "Event/KeySFMLInterface.h"
#include "Event/MouseButtonEvent.h"
#include "Event/MouseMoveEvent.h"
#include "Rendering/OpenGLRenderer.h"
#include "Rendering/Shader.h"
#include "Rendering/VAO.h"
#include "Rendering/VBO.h"
#include "Rendering/TextureManager.h"
#include "Rendering/FrameBuffer.h"

#ifdef __linux__
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>
#   include <GL/glxew.h>
#   include <GL/glxext.h>
#   include <GL/glx.h>
#endif

#ifdef _WIN32
typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
#endif

void OpenGLViewer::setVSync(bool sync)
{
  this->sfmlWindow->setVerticalSyncEnabled(sync);
}

OpenGLViewer::OpenGLViewer()
{
  this->windowOutput = std::make_shared<OpenGLWindowStream>();
}

OpenGLViewer::~OpenGLViewer() {}
void OpenGLViewer::exitViewer()
{
}

/// \brief Initializes OpenGL capabilities and flags
void OpenGLViewer::initRenderingCapabilities()
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

/// \brief Initializes FBOs, textures, shaders and VAOs
void OpenGLViewer::initResources()
{
    TextureManager::initGLTextures();
    Shader::initGLShaders();
    VAO::initVAOs();

    initFboListItems();
}

/// \brief Initializes the OpenGL context, and window containing it
void OpenGLViewer::initRenderingContext()
{

    // Init OpenGL context
    sfmlContext = std::unique_ptr<sf::Context>(new sf::Context);
    sfmlWindow = std::unique_ptr<sf::Window>(new sf::Window);
    // Init the rest of window system
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
    {
        this->sfmlWindow->create(sf::VideoMode(this->width(), this->height()),
                            windowTitle, sf::Style::Fullscreen);
    }
    else
    {
        this->sfmlWindow->create(sf::VideoMode(this->width(), this->height()),
                            windowTitle, (sf::Style::Titlebar | sf::Style::Close));
    }

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

/// \brief Cleans up after initGLContext()
void OpenGLViewer::destroyRenderingContext()
{
    //nothing to do
}

/// \brief render depth texture for debugging
void OpenGLViewer::renderTextureOnView()
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
    TextureManager::activateTexture("depth");
    glEnable(GL_TEXTURE_2D);
    glTranslated(0, 0, -1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3d(0, 0, 0);
    glTexCoord2d(1, 0);
    glVertex3d(1, 0, 0);
    glTexCoord2d(1, 1);
    glVertex3d(1, 1.0, 0);
    glTexCoord2d(0, 1);
    glVertex3d(0, 1.0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

/// \brief Adds an FBO to the viewer to allow rendering to it.
///
/// \detail The FBO will be created an initialized in the viewer.
///
/// \param p_fboName String to reference the FBO by
/// \param p_colorTex A texture that will contain the fbo's color texture.
/// \param p_depthTex A texture that will contain the fbo's depth texture.
/// \param p_width The width of the fbo
/// \param p_height The height of the fbo
void OpenGLViewer::addFBO(const std::string &p_fboName,
                      Texture *p_colorTex,
                      Texture *p_depthTex,
                      unsigned int p_width, unsigned int p_height)
{
    FboListItem item;

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

/// \brief Initializes the FBOs in the FBO list
void OpenGLViewer::initFboListItems()
{
    for (size_t i = 0; i < this->fboListItems.size(); i++)
    {
        FboListItem *item = &fboListItems[i];
        item->fbo = new FrameBuffer();
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
        item->fbo->disable();
    }
}

/// \brief Destroys all the FBOs in the FBO list
void OpenGLViewer::destroyFboListItems()
{
    for (size_t i = 0; i < this->fboListItems.size(); i++)
    {
        if (fboListItems[i].fbo)
        {
            delete (fboListItems[i].fbo);
            fboListItems[i].fbo = nullptr;
        }
    }
}

/// \brief Processes viewerRenderDetail options
void OpenGLViewer::processViewerOptions()
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FADEBACKGROUND)
    {
        GLUtils::fadeBackgroundDraw();
    }
}

///\brief Render and then process window events until the event queue is empty.
void OpenGLViewer::processWindowEvents()
{
  sf::Event event;
  this->render();
  while (this->sfmlWindow->pollEvent(event))
    this->processSFMLEvents(event);
}

/// \brief Renders the render operation to an FBO
void OpenGLViewer::renderToFBO(const RenderOperation &p_rop)
{
    assert(p_rop.fbo);
    //Enable FBO for rendering
    p_rop.fbo->enable();
    //Setup Viewport & Clear buffers
    glViewport(0, 0, p_rop.fbo->getWidth(), p_rop.fbo->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    processViewerOptions();
    //Render Scene
     OpenGLRenderer::renderScene(p_rop.scene);
    //Disable FBO
    p_rop.fbo->disable();
}

/// \brief Renders the render operation to screen
void OpenGLViewer::renderToScreen(const RenderOperation &p_rop)
{
    //Setup Viewport & Clear buffers
    glViewport(0, 0, this->width(), this->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    processViewerOptions();
    //Render Scene
    OpenGLRenderer::renderScene(p_rop.scene);

    //Render axis
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS)
    {
        Matrix44f proj = p_rop.scene->getCamera()->getProjMat();
        Matrix44f view = p_rop.scene->getCamera()->getViewMat();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(proj.data());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(view.data());

        //Enable lights
        p_rop.scene->enableLights();
        p_rop.scene->placeLights();

        OpenGLRenderer::drawAxes(this->globalAxisLength);

        p_rop.scene->disableLights();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

    }
}

/// \brief Set the color and other viewer defaults
void OpenGLViewer::setToDefaults()
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiffuseColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpecularColor.toGLColor());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmbientColor.toGLColor());
    glColor4fv(defaultDiffuseColor.toGLColor());
}

/// \brief Called at the beginning of each frame by the module
void OpenGLViewer::beginFrame()
{
    if (terminateExecution == true)
    {
        terminationCompleted = true;
    }

    this->sfmlWindow->setActive(true); //activates opengl context
}

///\brief Called at the end of each frame by the module
void OpenGLViewer::endFrame()
{
    this->sfmlWindow->display(); //swaps buffers
}

void OpenGLViewer::processSFMLEvents(const sf::Event& p_event)
{
    switch(p_event.type)
    {
    case sf::Event::Closed:
        //TODO: some type of terminate event
        break;
    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased:
    {
        auto keyboardEvent =
            std::make_shared<event::KeyboardEvent>(event::SFMLKeyToSmKey(p_event.key.code));
        keyboardEvent->setPressed(sf::Event::KeyPressed == p_event.type);

        keyboardEvent->setModifierKey(event::ModKey::none);
        if (p_event.key.shift)
            keyboardEvent->setModifierKey(keyboardEvent->getModifierKey() | event::ModKey::shift);
        if (p_event.key.control)
            keyboardEvent->setModifierKey(keyboardEvent->getModifierKey() | event::ModKey::control);
        if (p_event.key.alt)
            keyboardEvent->setModifierKey(keyboardEvent->getModifierKey() | event::ModKey::alt);
        if (p_event.key.system)
            keyboardEvent->setModifierKey(keyboardEvent->getModifierKey() | event::ModKey::super);

        eventHandler->triggerEvent(keyboardEvent);
        break;
    }
    case sf::Event::MouseButtonPressed:
    case sf::Event::MouseButtonReleased:
    {
        event::MouseButton mouseButton;
        if (sf::Mouse::Left == p_event.mouseButton.button)
            mouseButton = event::MouseButton::Left;
        else if (sf::Mouse::Right == p_event.mouseButton.button)
            mouseButton = event::MouseButton::Right;
        else if (sf::Mouse::Middle == p_event.mouseButton.button)
            mouseButton = event::MouseButton::Middle;
        else
            mouseButton = event::MouseButton::Unknown;

        auto mouseEvent = std::make_shared<event::MouseButtonEvent>(mouseButton);
        mouseEvent->setPresed(sf::Event::MouseButtonPressed == p_event.type);
        mouseEvent->setWindowCoord(core::Vec2d(p_event.mouseButton.x,p_event.mouseButton.y));
        eventHandler->triggerEvent(mouseEvent);
        break;
    }
    case sf::Event::MouseMoved:
    {
        auto mouseEvent = std::make_shared<event::MouseMoveEvent>();
        mouseEvent->setSender(core::EventSender::Module);
        mouseEvent->setWindowCoord(core::Vec2d(p_event.mouseMove.x, p_event.mouseMove.y));
        eventHandler->triggerEvent(mouseEvent);
        break;
    }
    default:
        break;
    }
}

void OpenGLViewer::addObject(std::shared_ptr<CoreClass> object)
{

    SDK::getInstance()->addRef(object);
    objectList.push_back(object);
}

void OpenGLViewer::handleEvent(std::shared_ptr<core::Event> /*p_event*/ )
{

}

void OpenGLViewer::addText(std::string p_tag)
{

    windowOutput->addText(p_tag, std::string(""));
}

void OpenGLViewer::updateText(std::string p_tag, std::string p_string)
{

    windowOutput->updateText(p_tag, p_string);
}
void OpenGLViewer::updateText(int p_handle, std::string p_string)
{

    windowOutput->updateText(p_handle, p_string);
}

void OpenGLViewer::setWindowTitle(const std::string &str)
{
    windowTitle = str;
}

void OpenGLViewer::cleanUp()
{
    destroyFboListItems();
    destroyRenderingContext();

    //Must be set when all cleanup is done
    terminationCompleted = true;
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_rendering_viewer)
    SIMMEDTK_REGISTER_CLASS(CoreClass,ViewerBase,OpenGLViewer,100);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
