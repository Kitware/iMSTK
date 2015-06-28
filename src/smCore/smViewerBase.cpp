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
#include "smCore/smIOStream.h"
#include "smCore/smViewerBase.h"
#include "smCore/smDataStructures.h"
#include "smExternal/tree.hh"

smRenderOperation::smRenderOperation()
{
    fbo = nullptr;
    scene = nullptr;
    fboName = "";
}

smViewerBase::smViewerBase()
{
    type = SIMMEDTK_SMVIEWER;
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
void smViewerBase::setScreenResolution(smInt p_width, smInt p_height)
{
    this->screenResolutionHeight = p_height;
    this->screenResolutionWidth = p_width;
}

void smViewerBase::setUnlimitedFPS(smBool p_enableFPS)
{
    unlimitedFPSEnabled = p_enableFPS;
    unlimitedFPSVariableChanged++;
}

void smViewerBase::initObjects()
{
    for (size_t i = 0; i < objectList.size(); i++)
    {
        if (objectList[i]->getType() != SIMMEDTK_SMSHADER)
        {
            objectList[i]->initDraw();
        }
        else
        {
            continue;
        }
    }
}

void smViewerBase::initScenes()
{
    //traverse all the scene and the objects in the scene
    for(auto&& scene : sceneList)
    {
        smSceneLocal sceneLocal;

        scene->initLights();
        scene->copySceneToLocal(sceneLocal);

        for (auto sceneObject: sceneLocal.sceneObjects)
        {
            //initialize the custom Render if there is any
            if ( sceneObject->customRender != NULL && sceneObject->getType() != SIMMEDTK_SMSHADER )
            {
                sceneObject->customRender->initDraw();
            }
            sceneObject->initDraw();
        }
    }//scene traverse
}

///initialization of the viewer module
void smViewerBase::init()
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

void smViewerBase::addFBO(const smString &p_fboName,
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

void smViewerBase::processRenderOperation(const smRenderOperation &p_rop)
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

void smViewerBase::registerScene(std::shared_ptr<smScene> p_scene,
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

    renderOperations.push_back(rop);
}

inline void smViewerBase::adjustFPS()
{

    static smInt _unlimitedFPSVariableChanged = 0;
    smInt unlimitedFPSVariableChangedCurrent;
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
void smViewerBase::render()
{
    if (viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    beginModule();

    adjustFPS();

    smRenderDelegate::Ptr delegate;
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

void smViewerBase::addObject(std::shared_ptr<smCoreClass> object)
{

    smSDK::getInstance()->addRef(object);
    objectList.push_back(object);
}

void smViewerBase::setWindowTitle(const smString &str)
{
    windowTitle = str;
}

void smViewerBase::exec()
{
    // Init the viewer
    this->init();

    while (!terminateExecution)
      this->processWindowEvents();

    cleanUp();
}

void smViewerBase::cleanUp()
{
    //Must be set when all cleanup is done
    terminationCompleted = true;
}

smInt smViewerBase::height(void)
{
    return screenResolutionHeight;
}

smInt smViewerBase::width(void)
{
    return screenResolutionWidth;
}

smFloat smViewerBase::aspectRatio(void)
{
    return screenResolutionHeight / screenResolutionWidth;
}

void smViewerBase::setGlobalAxisLength(const smFloat len)
{
    this->globalAxisLength = len;
}
