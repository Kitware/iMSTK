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
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smSDK.h"
#include "smCore/smStaticSceneObject.h"
#include "smSimulators/smDummySimulator.h"
#include "smCore/smEventData.h"
#include <GL/glew.h>
#include "smCore/smTextureManager.h"
#include "smExternalDevices/smPhantomInterface.h"
#include "smSimulators/smStylusObject.h"
#include "smSimulators/smMyStylus.h"
#include "smCollision/smSpatialHash.h"
#include "smUtilities/smMotionTransformer.h"
#include "smShader/smShader.h"
#include "smShader/metalShader.h"
#include "smSimulators/smToolSimulator.h"

#include <iostream>
#include <stack>


using namespace std;


class myTool: public MyStylus
{

public:

    myTool(): MyStylus("../../resources/models/blunt_diss_pivot.3DS",
                           "../../resources/models/blunt_diss_upper.3DS", "../../resources/models/blunt_diss_lower.3DS")
    {
    }

    void  handleEvent(smEvent *p_event)
    {
        s
        MyStylus::handleEvent(p_event);
        smKeyboardEventData *keyBoardData = (smKeyboardEventData*)p_event->data;

        if (keyBoardData->keyBoardKey == Qt::Key_U)
        {
            meshContainer.posOffsetPos.z -= 0.01;
        }

        if (keyBoardData->keyBoardKey == Qt::Key_I)
        {
            meshContainer.posOffsetPos.z += 0.01;
        }
    }
};


void main()
{

    MyStylus *hapticStylus;
    HookCautery *hapticStylus1;
    smPhantomInterface * hapticInterface;
    smSDK* simmedtkSDK;
    smScene *scene1;
    smViewer *viewer;
    smSimulator *simulator;
    smToolSimulator *toolSim;

    ///Initializes the lights
    smLight light("light0", SIMMEDTK_LIGHT_INFINITELIGHT, SIMMEDTK_LIGHTPOS_EYE);
    light.lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
    light.lightColorAmbient.setValue(0.4, 0.4, 0.4, 1);
    light.lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
    light.spotCutOffAngle = 60;


    light.lightPos.pos.setValue(0, 20.0, 20);
    light.direction = smVec3f(0.0, 0.0, -1.0);
    light.drawEnabled = false;


    smLight light2("light1", SIMMEDTK_LIGHT_SPOTLIGHT, SIMMEDTK_LIGHTPOS_WORLD);
    light2.lightColorDiffuse.setValue(0.4, 0.4, 0.4, 1);
    light2.lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light2.lightColorSpecular.setValue(0.1, 0.1, 0.1, 1);
    light2.spotCutOffAngle = 30.0;

    light2.lightPos.pos.setValue(20, 20, 20);
    light2.direction.setValue(-1, -1, -1);

    light2.drawEnabled = false;
    light2.castShadow = true;

    ///init texture manager and load the textures
    smTextureManager::init(simmedtkSDK->getErrorLog());
    smTextureManager::loadTexture("../../resources/textures/metal.bmp", "metal", true);
    smTextureManager::loadTexture("../../resources/textures/hook_cautery3.bmp", "hookCautery");
    smTextureManager::loadTexture("../../resources/textures/metalbump.bmp", "bump");
    smTextureManager::loadTexture("../../resources/textures/burn1024.bmp", "specTex"); ///for OCC I'll use the same texture

    ///create a grasper
    hapticStylus = new myTool();

    ///assign device id
    hapticStylus->phantomID = 0;

    ///there are 3 containers by default. Assign a name and texture for each
    hapticStylus->meshContainer.mesh->name = "Pivot";
    hapticStylus->meshContainerUpper.mesh->name = "Upper";
    hapticStylus->meshContainerLower.mesh->name = "Lower";
    hapticStylus->meshContainer.mesh->assignTexture("metal");
    hapticStylus->meshContainerLower.mesh->assignTexture("metal");
    hapticStylus->meshContainerUpper.mesh->assignTexture("metal");

    ///cupdates the normals and tangents in case
    hapticStylus->meshContainer.mesh->calcNeighborsVertices();
    hapticStylus->meshContainer.mesh->calcTriangleTangents();
    hapticStylus->meshContainer.mesh->updateVertexNormals();

    ///set the rendering details
    hapticStylus->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);

    ///create another tool. This is a cautery tool that has  only a shaft
    hapticStylus1 = new HookCautery();

    ///assign this to second phantom
    hapticStylus1->phantomID = 1;

    //hapticStylus1->renderDetail.renderType=hapticStylus1->renderDetail.renderType|SIMMEDTK_RENDER_NONE;
    hapticStylus1->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);

    ///creates tool simulator
    toolSim = new smToolSimulator(smSDK::getErrorLog());

    ///define the execution of the simulator.
    toolSim->setExecutionType(SIMMEDTK_SIMEXECUTION_ASYNCMODE);

    ///assigns the tools to tool simulator
    hapticStylus->attachObjectSimulator(toolSim);
    hapticStylus1->attachObjectSimulator(toolSim);

    ///creates SDK
    simmedtkSDK = smSDK::createSDK();

    ///create a scene.
    scene1 = simmedtkSDK->createScene();
    scene1->setName("Scene1");
    scene1->addSceneObject(hapticStylus);
    scene1->addSceneObject(hapticStylus1);

    ///create the simulator module
    simulator = simmedtkSDK->createSimulator();
    simulator->registerObjectSimulator(toolSim);

    ///creates viewer
    viewer = simmedtkSDK->createViewer();
    viewer->viewerRenderDetail = viewer->viewerRenderDetail;
    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_GLOBALAXIS | SIMMEDTK_VIEWERRENDER_RESTORELASTCAMSETTINGS;
    viewer->camera()->setZClippingCoefficient(100);
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    ///add lights
    viewer->addLight(&light);
    viewer->addLight(&light2);

    ///register viewer with the  set of events
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_HAPTICOUT);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_HAPTICIN);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_CAMERA_UPDATE);

    ///create a phanotm interface
    hapticInterface = new smPhantomInterface();
    hapticInterface->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    ///register tools for haptic out and keyboard events
    simmedtkSDK->getEventDispatcher()->registerEventHandler(hapticStylus, SIMMEDTK_EVENTTYPE_HAPTICOUT);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(hapticStylus, SIMMEDTK_EVENTTYPE_KEYBOARD);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(hapticStylus1, SIMMEDTK_EVENTTYPE_KEYBOARD);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(hapticStylus1, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    ///register haptic module
    simmedtkSDK->registerModule(hapticInterface);

    ///we want to call haptic inteface draw routine too
    viewer->addObject(hapticInterface);

    ///run  the SDK
    simmedtkSDK->run();
}




