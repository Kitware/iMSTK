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

#include "CollisionDetectionExample.h"
#include "Core/SDK.h"
#include "Rendering/TextureManager.h"
#include "Mesh/Lattice.h"
#include "Mesh/LatticeTypes.h"
#include "Core/EventData.h"

void CollisionDetectionExample::initHapticCamMotion()
{

    hapticInterface = new smPhantomInterface();
    hapticInterface->forceEnabled = false; //disable forces right now for all devices
    hapticInterface->startDevice();
    hapticInterface->setEventDispatcher(simmedtkSDK->getEventDispatcher());
    motionTrans = new smHapticCameraTrans(0);
    motionTrans->setMotionScale(0.1);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_CAMERA_UPDATE);
    viewer->enableCameraMotion = true;
}

CollisionDetectionExample::CollisionDetectionExample()
{

    motionTrans = NULL;
    hapticInterface = NULL;

    //initializes the spatial grid
    spatGrid = new smSpatialGrid();

    ///create the sdk
    simmedtkSDK = smSDK::createSDK();
    ///create scene objects
    object1 = new smStaticSceneObject();
    object2 = new smStaticSceneObject();

    ///create a 3D lattice for each object
    lat = new smLattice();
    lat2 = new smLattice();

    ///create a scene
    scene1 = simmedtkSDK->createScene();

    ///dummy simulator. it translates the object
    dummySim = new smDummySimulator(smSDK::getErrorLog());
    simmedtkSDK->getEventDispatcher()->registerEventHandler(dummySim, SIMMEDTK_EVENTTYPE_KEYBOARD);

    ///init texture manager and give the texture file names to be loaded
    smTextureManager::init(smSDK::getErrorLog());
    smTextureManager::loadTexture("../../resources/textures/fat9.bmp", "livertexture1");
    smTextureManager::loadTexture("../../resources/textures/blood.jpg", "livertexture2");

    ///For rendering the ground
    smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage"); //ground decal image
    smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage"); //gorund bum map image
    smTextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage"); //ground wall image
    smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage"); //ground wall bump image

    ///load a mesh
    object1->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);

    ///texture attachment needed for fixed opengl rendering if texture is needed
    object1->mesh->assignTexture("livertexture1");
    object1getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    object1->mesh->translate(7, 0, 0);
    object1getRenderDetail()->lineSize = 2;
    object1getRenderDetail()->pointSize = 5;

    ///add object1 to lattice
    lat->addObject(object1);

    ///add lattice to the grid
    spatGrid->addLattice(lat);

    ///the similiar routines for object2
    object2 = new smStaticSceneObject();
    object2->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    object2->mesh->translate(smVec3d(2, 0, 0));

    object2->mesh->assignTexture("livertexture2");
    object2getRenderDetail()->shadowColor.rgba[0] = 1.0;
    object2getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);

    lat2->addObject(object2);
    spatGrid->addLattice(lat2);
    spatGrid->pipe->registerListener(&this->myCollInformation);
    this->myCollInformation.listenerObject = &*this;
    this->myCollInformation.regType = SIMMEDTK_PIPE_BYREF;

    ///register the module for spatial grid
    simmedtkSDK->registerModule(spatGrid);

    //add object to the scene
    scene1->addSceneObject(object1);
    scene1->addSceneObject(object2);

    ///create the simulator
    simulator = simmedtkSDK->createSimulator();
    ///attach the dummy simulator
    simulator->registerObjectSimulator(dummySim);

    ///create a viewer
    viewer = simmedtkSDK->createViewer();

    //specify the viewer global settings
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_GROUND;
    viewer->camera()->setFieldOfView(SM_DEGREES2RADIANS(60));
    viewer->camera()->setZClippingCoefficient(1000);
    viewer->camera()->setZNearCoefficient(0.001);
    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");

    ///assign dispatcher
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    ///You can either add object to the viewer or add object to the scene. Draw function will be called
    viewer->addObject(spatGrid);
    viewer->addObject(this);

    ///run the simulation
    simmedtkSDK->run();
}

CollisionDetectionExample::~CollisionDetectionExample()
{

    delete object1;
    delete object2;
    delete scene1;
    delete dummySim;
    delete lat;
    delete lat2;
    delete spatGrid;

    if (motionTrans != NULL)
    {
        delete motionTrans;
    }

    if (hapticInterface != NULL)
    {
        delete hapticInterface;
    }
}

///Draw collided triangles
void CollisionDetectionExample::draw()
{

    smCollidedTriangles *tris;

    if (myCollInformation.data.dataReady)
    {
        if (myCollInformation.data.nbrElements > 0)
        {
            tris = (smCollidedTriangles *)myCollInformation.data.dataLocation;
            glBegin(GL_TRIANGLES);

            for (int i = 0; i < myCollInformation.data.nbrElements; i++)
            {
                glVertex3dv((GLfloat*)&tris[i].tri1.vert[0]);
                glVertex3dv((GLfloat*)&tris[i].tri1.vert[1]);
                glVertex3dv((GLfloat*)&tris[i].tri1.vert[2]);

                glVertex3dv((GLfloat*)&tris[i].tri2.vert[0]);
                glVertex3dv((GLfloat*)&tris[i].tri2.vert[1]);
                glVertex3dv((GLfloat*)&tris[i].tri2.vert[2]);
            }

            glEnd();
        }
    }

}

//extern function
void main()
{
    CollisionDetectionExample *ex = new CollisionDetectionExample();
    delete ex;
}
