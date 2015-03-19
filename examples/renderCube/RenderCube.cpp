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
    simmedtkSDK = smSDK::createSDK();

    //Create a new scene to work in
    scene1 = simmedtkSDK->createScene();

    //Create a viewer to see the scene through
    //viewer = simmedtkSDK->createViewer();
    simmedtkSDK->addViewer(&viewer);

    //Initialize the texture manager
    smTextureManager::init(smSDK::getErrorLog());

    //Load in the texture for the cube model
    smTextureManager::loadTexture("textures/cube.png", "cubetex");

    //Load the cube model
    cube.mesh->loadMesh("models/cube.obj", SM_FILETYPE_OBJ);
    //Assign the previously loaded texture to the cube model
    cube.mesh->assignTexture("cubetex");
    //Tell SimMedTK to render the faces of the model, and the texture assigned
    cube.mesh->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

    //Add the cube to the scene to be rendered
    scene1->addSceneObject(&cube);

    //Register the scene with the viewer, and setup render target
    viewer.registerScene(scene1, SMRENDERTARGET_SCREEN, "");

    //Setup the window title in the window manager
    viewer.setWindowTitle("SimMedTK RENDER TEST");

    //Add the RenderExample object we are in to the viewer from the SimMedTK SDK
    viewer.addObject(this);

    //Set some viewer properties
    viewer.setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Setup lights
    this->setupLights();

    //Set some camera parameters
    this->setupCamera();

    //Link up the event system between this object and the SimMedTK SDK
    simmedtkSDK->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_KEYBOARD);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_MOUSE_BUTTON);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_MOUSE_MOVE);
}

RenderCube::~RenderCube()
{
    simmedtkSDK->releaseScene(scene1);
}

void RenderCube::setupLights()
{
     //Setup Scene lighting
    smLight* light = new smLight("SceneLight1",
                                 SIMMEDTK_LIGHT_SPOTLIGHT,
                                 SIMMEDTK_LIGHTPOS_WORLD);
    light->lightPos.pos << 10.0, 10.0, 10.0;
    light->lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
    light->lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light->lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
    light->spotCutOffAngle = 60;
    light->direction = smVec3f(0.0, 0.0, -1.0);
    light->drawEnabled = false;
    light->attn_constant = 1.0;
    light->attn_linear = 0.0;
    light->attn_quadratic = 0.0;
    scene1->addLight(light);
}

void RenderCube::setupCamera()
{
    scene1->camera.setAspectRatio(800.0/640.0); //Doesn't have to match screen resolution
    scene1->camera.setFarClipDist(1000);
    scene1->camera.setNearClipDist(0.001);
    scene1->camera.setViewAngle(0.785398f); //45 degrees
    scene1->camera.setCameraPos(3, 3, 5);
    scene1->camera.setCameraFocus(0, 0, -1);
    scene1->camera.setCameraUpVec(0, 1, 0);
    scene1->camera.genProjMat();
    scene1->camera.genViewMat();
}

void RenderCube::handleEvent(smEvent *p_event)
{
    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_KEYBOARD:
    {
        smKeyboardEventData* kbData =
            reinterpret_cast<smKeyboardEventData*>(p_event->data);
        smKey key = kbData->keyBoardKey;
        if (key == smKey::Escape && kbData->pressed)
        {
            //Tell the framework to shutdown
            simmedtkSDK->shutDown();
        }
        else if (key == smKey::W && kbData->pressed)
        {
            smCamera cam = scene1->camera;
            if (smModKey::shift == (kbData->modKeys & smModKey::shift))
            {
                //Move the camera up
                scene1->camera.setCameraPos(cam.pos.x, cam.pos.y + 1, cam.pos.z);
                scene1->camera.setCameraFocus(cam.fp.x, cam.fp.y + 1, cam.fp.z);
                scene1->camera.genViewMat();
            }
            else
            {
                //Move the camera forward
                scene1->camera.setCameraPos(cam.pos.x, cam.pos.y, cam.pos.z - 1);
                scene1->camera.setCameraFocus(cam.fp.x, cam.fp.y, cam.fp.z - 1);
                scene1->camera.genViewMat();
            }
        }
        else if (key == smKey::A && kbData->pressed)
        {
            //Move the camera to the left
            smCamera cam = scene1->camera;
            scene1->camera.setCameraPos(cam.pos.x - 1, cam.pos.y, cam.pos.z);
            scene1->camera.setCameraFocus(cam.fp.x - 1, cam.fp.y, cam.fp.z);
            scene1->camera.genViewMat();
        }
        else if (key == smKey::S && kbData->pressed)
        {
            //Move the camera backward
            smCamera cam = scene1->camera;
            if (smModKey::shift == (kbData->modKeys & smModKey::shift))
            {
                scene1->camera.setCameraPos(cam.pos.x, cam.pos.y - 1, cam.pos.z);
                scene1->camera.setCameraFocus(cam.fp.x, cam.fp.y - 1, cam.fp.z);
                scene1->camera.genViewMat();
            }
            else
            {
                scene1->camera.setCameraPos(cam.pos.x, cam.pos.y, cam.pos.z + 1);
                scene1->camera.setCameraFocus(cam.fp.x, cam.fp.y, cam.fp.z + 1);
                scene1->camera.genViewMat();
            }
        }
        else if (key == smKey::D && kbData->pressed)
        {
            //Move the camera to the right
            smCamera cam = scene1->camera;
            scene1->camera.setCameraPos(cam.pos.x + 1, cam.pos.y, cam.pos.z);
            scene1->camera.setCameraFocus(cam.fp.x + 1, cam.fp.y, cam.fp.z);
            scene1->camera.genViewMat();
        }
        break;
    }
    case SIMMEDTK_EVENTTYPE_MOUSE_BUTTON:
    {
        smMouseButtonEventData* mbData =
            reinterpret_cast<smMouseButtonEventData*>(p_event->data);
        std::cout << "mbData: button: ";
        if (mbData->mouseButton == smMouseButton::Left)
            std::cout << "Left";
        else if (mbData->mouseButton == smMouseButton::Right)
            std::cout << "Right";
        else if (mbData->mouseButton == smMouseButton::Middle)
            std::cout << "Middle";
        else
            std::cout << "Unknown";

        std::cout << " pressed: ";
        if(mbData->pressed)
            std::cout << "true";
        else
            std::cout << "false";

        std::cout << " x: " << mbData->windowX << " y: " << mbData->windowY << "\n";
        break;
    }
    case SIMMEDTK_EVENTTYPE_MOUSE_MOVE:
    {
        smMouseMoveEventData* mpData =
            reinterpret_cast<smMouseMoveEventData*>(p_event->data);
        std::cout << "mpData: x: " << mpData->windowX
            << " y: " << mpData->windowY << "\n";
        break;
    }
    default:
        break;
    }
}

void RenderCube::simulateMain(smSimulationMainParam /*p_param*/)
{
    //Run the simulator framework
    simmedtkSDK->run();
}

void runRenderCube()
{
    smSimulationMainParam simulationParams;
    RenderCube rc;

    rc.simulateMain(simulationParams);

    return;
}
