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

#include "Core/Config.h"
#include "Core/SDK.h"
#include "Core/IOStream.h"
#include "Core/ViewerBase.h"
#include "Core/DataStructures.h"

RenderOperation::RenderOperation()
{
    fbo = nullptr;
    scene = nullptr;
    fboName = "";
}

ViewerBase::ViewerBase()
{
    type = core::ClassType::Viewer;
    viewerRenderDetail = SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;

    defaultAmbientColor.setValue(0.1, 0.1, 0.1, 1.0);
    defaultDiffuseColor.setValue(0.8, 0.8, 0.8, 1.0);
    defaultSpecularColor.setValue(0.9, 0.9, 0.9, 1.0);

    this->log = NULL;

    this->globalAxisLength = 1.0;

    unlimitedFPSEnabled = false;
    unlimitedFPSVariableChanged = 1;
    screenResolutionWidth = 1680;
    screenResolutionHeight = 1050;
}

///affects the framebuffer size and depth buffer size
void ViewerBase::setScreenResolution(int p_width, int p_height)
{
    this->screenResolutionHeight = p_height;
    this->screenResolutionWidth = p_width;
}

void ViewerBase::setUnlimitedFPS(bool p_enableFPS)
{
    unlimitedFPSEnabled = p_enableFPS;
    unlimitedFPSVariableChanged++;
}

void ViewerBase::initObjects()
{
    for (size_t i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->getType() != core::ClassType::Shader)
        {
            objectList[i]->initDraw();
        }
        else
        {
            continue;
        }
    }
}

void ViewerBase::initScenes()
{
    //traverse all the scene and the objects in the scene
    for(auto&& scene : sceneList)
    {
        SceneLocal sceneLocal;

        scene->initLights();
        scene->copySceneToLocal(sceneLocal);

        for (auto sceneObject: sceneLocal.sceneObjects)
        {
            //initialize the custom Render if there is any
            if ( sceneObject->customRender != NULL && sceneObject->getType() != core::ClassType::Shader )
            {
                sceneObject->customRender->initDraw();
            }
            sceneObject->initDraw();
        }
    }//scene traverse
}

///initialization of the viewer module
void ViewerBase::init()
{
    if (isInitialized)
    {
        return;
    }

    this->initRenderingContext();
    this->initRenderingCapabilities();
    this->initObjects();
    this->initResources();
    this->initScenes();

    isInitialized = true;
}

void ViewerBase::addFBO(const std::string &p_fboName,
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

void ViewerBase::processRenderOperation(const RenderOperation &p_rop)
{
    switch (p_rop.target)
    {
    case SMRENDERTARGET_SCREEN:
        renderToScreen(p_rop);
        break;
    case SMRENDERTARGET_FBO:
        renderToFBO(p_rop);
        break;
    default:
        assert(0);
    }
}

void ViewerBase::registerScene(std::shared_ptr<Scene> p_scene,
                             RenderTargetType p_target,
                             const std::string &p_fboName)
{
    RenderOperation rop;

    //sanity checks
    assert(p_scene);
    if (p_target == SMRENDERTARGET_FBO)
    {
        assert(p_fboName != "");
    }

    rop.target = p_target;
    rop.scene = p_scene;

    rop.fboName = p_fboName;

    renderOperations.push_back(rop);
}

inline void ViewerBase::adjustFPS()
{

    static int _unlimitedFPSVariableChanged = 0;
    int unlimitedFPSVariableChangedCurrent;
    unlimitedFPSVariableChangedCurrent = unlimitedFPSVariableChanged;

    if (_unlimitedFPSVariableChanged < unlimitedFPSVariableChangedCurrent)
    {
        _unlimitedFPSVariableChanged = unlimitedFPSVariableChangedCurrent;

        if (unlimitedFPSEnabled)
        {
            setVSync(false);
        }
        else
        {
            setVSync(true);
        }
    }
}

///main drawing routine for Rendering of all objects in the scene
void ViewerBase::render()
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    beginModule();

    adjustFPS();

    RenderDelegate::Ptr delegate;
    for (size_t i = 0; i < objectList.size(); i++)
    {
        delegate = objectList[i]->getRenderDelegate();
        if (delegate)
          delegate->draw();
    }

    for (size_t i = 0; i < renderOperations.size(); i++)
    {
        processRenderOperation(renderOperations[i]);
    }

    for (size_t i = 0; i < objectList.size(); i++)
    {
        delegate = objectList[i]->getRenderDelegate();
        if (delegate)
          delegate->draw();
    }

    endModule();
}

void ViewerBase::addObject(std::shared_ptr<CoreClass> object)
{

    SDK::getInstance()->addRef(object);
    objectList.push_back(object);
}

void ViewerBase::setWindowTitle(const std::string &str)
{
    windowTitle = str;
}

void ViewerBase::exec()
{
    // Init the viewer
    this->init();

    while (!terminateExecution)
      this->processWindowEvents();

    cleanUp();
}

void ViewerBase::cleanUp()
{
    //Must be set when all cleanup is done
    terminationCompleted = true;
}

int ViewerBase::height(void)
{
    return screenResolutionHeight;
}

int ViewerBase::width(void)
{
    return screenResolutionWidth;
}

float ViewerBase::aspectRatio(void)
{
    return screenResolutionHeight / screenResolutionWidth;
}

void ViewerBase::setGlobalAxisLength(const float len)
{
    this->globalAxisLength = len;
}