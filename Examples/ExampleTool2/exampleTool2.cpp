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
#include "Core/ErrorLog.h"
#include "Core/CoreClass.h"
#include "Core/SceneObject.h"
#include "Core/SDK.h"
#include "Core/StaticSceneObject.h"
#include "Core/EventData.h"
#include <GL/glew.h>
#include "Rendering/TextureManager.h"
#include "ExternalDevices/PhantomInterface.h"
#include "Simulators/StylusObject.h"
#include "Rendering/Shader.h"
#include "Rendering/metalShader.h"
#include "Simulators/ToolSimulator.h"
#include "Simulators/ToolSimulator.h"
#include "Tools/CurvedGrasper.h"
#include "Rendering/metalShader.h"

#include <iostream>
#include <stack>

using namespace std;

void main()
{
    PhantomInterface * hapticInterface;
    SDK* simmedtkSDK;
    Scene *scene1;
    Viewer *viewer;
    Simulator *simulator;

    ///create lights
    Light light("Light0", InfiniteLight, Eye);
    light.lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
    light.lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light.lightColorSpecular.setValue(0.9, 0.0, 0.0, 1);
    light.spotCutOffAngle = 180.0;

    light.lightPos.pos.setValue(0, 2.0, 0);
    light.direction = core::Vec3d(0.0, 0.0, 1.0);
    light.drawEnabled = true;


    Light light2("Light1", Spotlight, World);
    light2.lightColorDiffuse.setValue(0.4, 0.4, 0.4, 1);
    light2.lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light2.lightColorSpecular.setValue(0.1, 0.1, 0.1, 1);
    light2.spotCutOffAngle = 30.0;

    light2.lightPos.pos.setValue(20, 20, 20);
    light2.direction.setValue(-1, -1, -1);

    light2.drawEnabled = true;
    light2.castShadow = true;

    ///Creat the SDK. SDK is a singleton object.
    simmedtkSDK = SDK::createSDK();
    scene1 = simmedtkSDK->createScene();
    scene1->setName("Scene1");

    ///init texture manager and load the textures
    TextureManager::init(simmedtkSDK->getErrorLog());
    TextureManager::loadTexture("../../resources/textures/metal.bmp", "metal");
    TextureManager::loadTexture("../../resources/textures/hook_cautery3.bmp", "hookCautery");
    TextureManager::loadTexture("../../resources/textures/metalbump.bmp", "bump");
    TextureManager::loadTexture("../../resources/textures/sword_bump.bmp", "nobump");
    TextureManager::loadTexture("../../resources/textures/burn1024.bmp", "specTex"); ///for OCC I'll use the same texture

    ///initialize the vertex and fragment shader
    MetalShaderShadow *metalShader = new MetalShaderShadow(
        "../../resources/shaders/MultipleShadowsVertexBumpMap2.cg",
        "../../resources/shaders/MultipleShadowsFragmentBumpMap2.cg");

    ///register the shader. Otherwise it won't initialize
    metalShader->registerShader();

    ///tools are maniuplated with ToolSimulator
    ToolSimulator *toolSim = new ToolSimulator(SDK::getErrorLog());

    ///curved grasper requires device ID and models for pivot, upper and lower mesh
    CurvedGrasper *curvedTool;
    curvedTool = new CurvedGrasper(0, "../../resources/models/curved_pivot.3DS",
                                   "../../resources/models/curved_upper.3DS",
                                   "../../resources/models/curved_lower.3DS");

    ///textures assignments are necessary if shading will not be used
    curvedTool->mesh_pivot->assignTexture("toolPivot");
    curvedTool->mesh_lowerJaw->assignTexture("toolJaw");
    curvedTool->mesh_upperJaw->assignTexture("toolJaw");

    ///we want to use shader. This requires the unique shader ID
    curvedToolgetRenderDetail()->addShader(metalShader->getUniqueId());

    ///prevent casting shading on itself
    curvedTool->mesh_lowerJawgetRenderDetail()->canGetShadow = false;
    curvedTool->mesh_upperJawgetRenderDetail()->canGetShadow = false;
    curvedTool->mesh_lowerJawgetRenderDetail()->shininess = 5.0;
    curvedTool->mesh_upperJawgetRenderDetail()->shininess = 5.0;
    curvedTool->mesh_pivotgetRenderDetail()->shininess = 5.0;
    curvedTool->mesh_lowerJaw->scale(core::Vec3d(1.0, 1.0, 1.5));
    curvedTool->mesh_upperJaw->scale(core::Vec3d(1.0, 1.0, 1.5));

    ///Each mesh is attached with proper shader texture bindings
    metalShader->attachMesh(curvedTool->mesh_lowerJaw, "bump", "metal", "specTex", "specTex", "specTex");
    metalShader->attachMesh(curvedTool->mesh_upperJaw, "bump", "metal", "specTex", "specTex", "specTex");
    metalShader->attachMesh(curvedTool->mesh_pivot, "bump", "hookCautery", "specTex", "specTex", "specTex");

    ///Transformation hiearhcy is defined with the meshes
    curvedTool->addMeshContainer(&curvedTool->meshContainer_pivot);
    curvedTool->addMeshContainer(curvedTool->meshContainer_pivot.name, &curvedTool->meshContainer_lowerJaw);
    curvedTool->addMeshContainer(curvedTool->meshContainer_pivot.name, &curvedTool->meshContainer_upperJaw);
    curvedToolgetRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    scene1->addSceneObject(curvedTool);

    ///Tool is attached to Tool simulator
    curvedTool->attachObjectSimulator(toolSim);

    ///A seperate thread will be spawned for the tool simulator
    toolSim->setExecutionType(SIMMEDTK_SIMEXECUTION_ASYNCMODE);

    ///create the main simulator
    simulator = simmedtkSDK->createSimulator();

    ///register the tool simulator with the simulator.
    simulator->registerObjectSimulator(toolSim);

    ///create viewer
    viewer = simmedtkSDK->createViewer();
    viewer->viewerRenderDetail = viewer->viewerRenderDetail;

    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_GLOBALAXIS |
                                 | SIMMEDTK_VIEWERRENDER_RESTORELASTCAMSETTINGS;
    viewer->viewerRenderDetail = viewer->viewerRenderDetail;
    viewer->camera()->setZClippingCoefficient(100);
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    ///add lights for static opengl rendering
    viewer->addLight(&light);
    viewer->addLight(&light2);

    ///register event and event handlers
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_HAPTICOUT);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_HAPTICIN);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_CAMERA_UPDATE);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(curvedTool, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    ///create a phantom interface
    hapticInterface = new PhantomInterface();

    ///set dispacther. This will be used to distrubute events
    hapticInterface->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    ///register the haptic interface module
    simmedtkSDK->registerModule(hapticInterface);
    viewer->addObject(hapticInterface);

    ///run the SDK. SDK will initiate all modules etc.
    simmedtkSDK->run();
}