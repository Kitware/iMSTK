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

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "RenderExample.h"

/// \brief A simple example of how to render and object using SimMedTK
///
/// \detail This is the default constructor, however, this is where the main
/// program runs.  This program will create a cube with a texture pattern
/// numbering each side of the cube, that's all it does.
RenderExample::RenderExample()
{
    //Create an instance of the SimMedTK framework/SDK
    simmedtkSDK = smSDK::createSDK();

    //Create a new scene to work in
    scene1 = simmedtkSDK->createScene();

    //Create a viewer to see the scene through
    viewer = simmedtkSDK->createViewer();

    //Initialize the texture manager
    smTextureManager::init(smSDK::getErrorLog());

    //Load in the texture for the cube model
    smTextureManager::loadTexture("textures/cube.png", "cubetex");

    //Load the cube model
    cube.mesh->loadMesh("models/cube.obj", SM_FILETYPE_OBJ);

    //Assign the previously loaded texture to the cube model
    cube.mesh->assignTexture("cubetex");
    //Tell SimMedTK to render the faces of the model, and the texture assigned
    cube.renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

    //Add the cube to the scene to be rendered
    scene1->addSceneObject(&cube);

    //Register the scene with the viewer, and setup render target
    viewer->registerScene(scene1, SMRENDERTARGET_SCREEN, "");

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TEST");

    //Add the RenderExample object we are in to the viewer from the SimMedTK SDK
    viewer->addObject(this);

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Setup lights
    this->setupLights();

    //Set some camera parameters
    this->setupCamera();

    //Link up the event system between the viewer and the SimMedTK SDK
    //Note: This allows some default behavior like mouse and keyboard control
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_KEYBOARD);

    //Run the simulator framework
    simmedtkSDK->run();
}

void RenderExample::setupLights()
{
     //Setup Scene lighting
    smLight* light = new smLight("SceneLight1",
                                 SIMMEDTK_LIGHT_SPOTLIGHT,
                                 SIMMEDTK_LIGHTPOS_WORLD);
    light->lightPos.pos.setValue(10.0, 10.0, 10.0);
    light->lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
    light->lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light->lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
    light->spotCutOffAngle = 60;
    light->direction = smVec3f(0.0, 0.0, -1.0);
    light->drawEnabled = false;
    light->attn_constant = 1.0;
    light->attn_linear = 0.0;
    light->attn_quadratic = 0.0;
    viewer->addLight(light);
}

void RenderExample::setupCamera()
{
    viewer->camera.setAspectRatio(800.0/640.0); //Doesn't have to match screen resolution
    viewer->camera.setFarClipDist(1000);
    viewer->camera.setNearClipDist(0.001);
    viewer->camera.setViewAngle(0.785398f); //45 degrees
    viewer->camera.setCameraPos(3, 3, 5);
    viewer->camera.setCameraFocus(0, 0, 0);
    viewer->camera.setCameraUpVec(0, 1, 0);
    viewer->camera.genProjMat();
    viewer->camera.genViewMat();
}

void RenderExample::simulateMain(smSimulationMainParam p_param)
{
}

void renderExample()
{
    RenderExample *re = new RenderExample();
    delete re;
}
