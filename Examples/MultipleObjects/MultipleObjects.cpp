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
#include "SDK.h"
#include "FemSceneObject.h"
#include "FemSimulator.h"
#include "PBDSceneObject.h"
#include "PBDObjectSimulator.h"

void createPBDandFEM()
{

    SDK *simmedtkSDK;
    FemSceneObject  *femobj;
    smFemSimulator *femSim;
    PBDSurfaceSceneObject *pbdObject;
    PBDObjectSimulator * pbd;
    Matrix33d mat;
    Simulator *simulator;
    Viewer *viewer;
    Scene *scene1;

    ///create rotation matrix
    mat.rotAroundX(M_PI);

    ///SDK is singlenton class so create SDK and return SDK returns the same SDK
    simmedtkSDK = SDK::getInstance();

    ///init texture manager and load the textures
    TextureManager::init(simmedtkSDK->getErrorLog());
    TextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage");
    TextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage");
    TextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage");
    TextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage");

    //for cloth texture
    TextureManager::loadTexture("../../resources/textures/cloth.jpg", "clothtexture");

    ///create a FEM simulator
    femSim = new smFemSimulator(simmedtkSDK->getErrorLog());

    ///set the dispatcher for FEM. it will be used for sending events
    femSim->setDispatcher(simmedtkSDK->getEventDispatcher());
    simmedtkSDK->getEventDispatcher()->registerEventHandler(femSim, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    ///create a position dynamics object
    pbd = new PBDObjectSimulator(simmedtkSDK->getErrorLog());

    ///create a FEM object
    femobj = new FemSceneObject();
    femobj->v_mesh->LoadTetra("../../resources/models/cylinder_Mesh.txt");
    femobj->v_mesh->getSurface("../../resources/models/cylinder_smesh.txt");
    femobj->v_mesh->readBC("../../resources/models/cylinder_BC.txt");
    femobj->v_mesh->scaleVolumeMesh(core::Vec3d(1, 1, 1));
    femobjgetRenderDetail()->colorDiffuse = Color::colorBlue;
    femobjgetRenderDetail()->colorAmbient = Color::colorBlue;
    femobjgetRenderDetail()->shadowColor = Color(0.2, 0.2, 0.2);
    femobj->v_mesh->rotate(mat);
    femobj->v_mesh->scaleVolumeMesh(core::Vec3d(0.5, 0.5, 1.0));
    femobj->v_mesh->translateVolumeMesh(core::Vec3d(20, 15, 20));
    femobjgetRenderDetail()->renderType = (SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_SHADOWS | SIMMEDTK_RENDER_FACES);
    femobjgetRenderDetail()->lineSize = 1.0;
    femobjgetRenderDetail()->pointSize = 5;
    femobj->buildLMmatrix();
    femobj->computeStiffness();
    femobj->attachObjectSimulator(femSim);

    ///create a position based dynamics(PBD) surface object
    pbdObject = new PBDSurfaceSceneObject();
    pbdObjectgetRenderDetail()->colorDiffuse = Color::colorWhite;
    pbdObjectgetRenderDetail()->colorAmbient = Color::colorWhite;
    pbdObject->mesh->loadMeshLegacy("../../resources/models/clothtextured.3ds", BaseMesh::MeshFileType::ThreeDS);

    //pbdObject->mesh->rotate(mat);
    pbdObject->mesh->scale(core::Vec3d(2.3, 0.5, 2));
    pbdObject->mesh->translate(core::Vec3d(11, 25, 0));
    pbdObject->mesh->assignTexture("clothtexture");
    pbdObjectgetRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    pbdObject->initMeshStructure();
    pbdObject->InitSurfaceObject();

    ///find the corners and fixed it
    pbdObject->findFixedCorners();

    ///can use sphere shape to fix the vertices
    ///pbdObject->findFixedMassWrtSphere(core::Vec3d(11,23,0),5);
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

