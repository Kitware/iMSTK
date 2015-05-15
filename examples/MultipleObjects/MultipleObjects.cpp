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

#include "MultipleObjects.h"
#include "smSDK.h"
#include "smFemSceneObject.h"
#include "smFemSimulator.h"
#include "smPBDSceneObject.h"
#include "smPBDObjectSimulator.h"

void createPBDandFEM()
{

    smSDK *simmedtkSDK;
    smFemSceneObject  *femobj;
    smFemSimulator *femSim;
    smPBDSurfaceSceneObject *pbdObject;
    smPBDObjectSimulator * pbd;
    smMatrix33d mat;
    smSimulator *simulator;
    smViewer *viewer;
    smScene *scene1;

    ///create rotation matrix
    mat.rotAroundX(SM_PI);

    ///SDK is singlenton class so create SDK and return SDK returns the same SDK
    simmedtkSDK = smSDK::getInstance();

    ///init texture manager and load the textures
    smTextureManager::init(simmedtkSDK->getErrorLog());
    smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage");
    smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage");
    smTextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage");
    smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage");

    //for cloth texture
    smTextureManager::loadTexture("../../resources/textures/cloth.jpg", "clothtexture");

    ///create a FEM simulator
    femSim = new smFemSimulator(simmedtkSDK->getErrorLog());

    ///set the dispatcher for FEM. it will be used for sending events
    femSim->setDispatcher(simmedtkSDK->getEventDispatcher());
    simmedtkSDK->getEventDispatcher()->registerEventHandler(femSim, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    ///create a position dynamics object
    pbd = new smPBDObjectSimulator(simmedtkSDK->getErrorLog());

    ///create a FEM object
    femobj = new smFemSceneObject();
    femobj->v_mesh->LoadTetra("../../resources/models/cylinder_Mesh.txt");
    femobj->v_mesh->getSurface("../../resources/models/cylinder_smesh.txt");
    femobj->v_mesh->readBC("../../resources/models/cylinder_BC.txt");
    femobj->v_mesh->scaleVolumeMesh(smVec3d(1, 1, 1));
    femobjgetRenderDetail()->colorDiffuse = smColor::colorBlue;
    femobjgetRenderDetail()->colorAmbient = smColor::colorBlue;
    femobjgetRenderDetail()->shadowColor = smColor(0.2, 0.2, 0.2);
    femobj->v_mesh->rotate(mat);
    femobj->v_mesh->scaleVolumeMesh(smVec3d(0.5, 0.5, 1.0));
    femobj->v_mesh->translateVolumeMesh(smVec3d(20, 15, 20));
    femobjgetRenderDetail()->renderType = (SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_SHADOWS | SIMMEDTK_RENDER_FACES);
    femobjgetRenderDetail()->lineSize = 1.0;
    femobjgetRenderDetail()->pointSize = 5;
    femobj->buildLMmatrix();
    femobj->computeStiffness();
    femobj->attachObjectSimulator(femSim);

    ///create a position based dynamics(PBD) surface object
    pbdObject = new smPBDSurfaceSceneObject();
    pbdObjectgetRenderDetail()->colorDiffuse = smColor::colorWhite;
    pbdObjectgetRenderDetail()->colorAmbient = smColor::colorWhite;
    pbdObject->mesh->loadMeshLegacy("../../resources/models/clothtextured.3ds", SM_FILETYPE_3DS);

    //pbdObject->mesh->rotate(mat);
    pbdObject->mesh->scale(smVec3d(2.3, 0.5, 2));
    pbdObject->mesh->translate(smVec3d(11, 25, 0));
    pbdObject->mesh->assignTexture("clothtexture");
    pbdObjectgetRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    pbdObject->initMeshStructure();
    pbdObject->InitSurfaceObject();

    ///find the corners and fixed it
    pbdObject->findFixedCorners();

    ///can use sphere shape to fix the vertices
    ///pbdObject->findFixedMassWrtSphere(smVec3d(11,23,0),5);
    ///attach object to the simulator
    pbdObject->attachObjectSimulator(pbd);

    ///create a scene and add FEM and PDB
    scene1 = simmedtkSDK->createScene();
    scene1->addSceneObject(femobj);
    scene1->addSceneObject(pbdObject);

    ///create a simulator module and attache FEM and PBD
    simulator = simmedtkSDK->createSimulator();
    simulator->registerObjectSimulator(femSim);
    simulator->registerObjectSimulator(pbd);

    ///create a viewer
    viewer = simmedtkSDK->createViewer();
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_GROUND;
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());
    viewer->camera()->setZClippingCoefficient(100);

    ///run the SDK
    simmedtkSDK->run();
}

void main()
{
    ///Example routines for creating the PBD and FEM
    createPBDandFEM();
}

