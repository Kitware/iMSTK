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

#include "RenderCube.h"

#include <memory>

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"

/// \brief A simple example of how to render an object using SimMedTK
///
/// \detail This is the default constructor, however, this is where the main
/// program runs.  This program will create a cube with a texture pattern
/// numbering each side of the cube, that's all it does.
RenderCube::RenderCube()
{
    //Create an instance of the SimMedTK framework/SDK
    sdk = smSDK::getInstance();

    //Create a new scene to work in
    scene1 = sdk->createScene();

    //Create a viewer to see the scene through
    viewer = std::make_shared<smViewer>();
    sdk->addViewer(viewer);

    //Create the camera controller
    camCtl = std::make_shared<smtk::Examples::Common::wasdCameraController>();

    //Initialize the texture manager
    smTextureManager::init(sdk->getErrorLog());

    //Load in the texture for the cube model
    smTextureManager::loadTexture("textures/cube.png", "cubetex");

    cube = std::make_shared<smStaticSceneObject>();

    //Load the cube model
    cube->mesh->loadMesh("models/cube.obj", SM_FILETYPE_OBJ);
    //Assign the previously loaded texture to the cube model
    cube->mesh->assignTexture("cubetex");
    //Tell SimMedTK to render the faces of the model, and the texture assigned
    cube->mesh->getRenderDetail()->renderType = SIMMEDTK_RENDER_WIREFRAME;//(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

    //Add the cube to the scene to be rendered
    scene1->addSceneObject(cube);

    //Register the scene with the viewer, and setup render target
    viewer->registerScene(scene1, SMRENDERTARGET_SCREEN, "");

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TEST");

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Setup lights
    this->setupLights();

    //Set some camera parameters
    this->setupCamera();

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, camCtl);
}

RenderCube::~RenderCube()
{
    sdk->releaseScene(scene1);
}

void RenderCube::setupLights()
{
    // Setup Scene lighting
    light = smLight::getDefaultLighting();
    assert(light);
    scene1->addLight(light);
}

void RenderCube::setupCamera()
{
    // Camera setup
    sceneCamera = smCamera::getDefaultCamera();
    assert(sceneCamera);
    sceneCamera->setCameraPos(3, 3, 5);
    sceneCamera->setCameraFocus(0, 0, -1);
    sceneCamera->genProjMat();
    sceneCamera->genViewMat();
    scene1->addCamera(sceneCamera);
    camCtl->setCamera(sceneCamera);
}

void RenderCube::simulateMain(const smSimulationMainParam &p_param)
{
    //Run the simulator framework
    sdk->run();
}

void runRenderCube()
{
    smSimulationMainParam simulationParams;
    RenderCube rc;

    rc.simulateMain(simulationParams);

    return;
}
