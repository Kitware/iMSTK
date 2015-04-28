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

#include "VAORendering.h"

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smMesh/smLattice.h"
#include "smMesh/smLatticeTypes.h"
#include "smCore/smEventData.h"
#include "smRendering/smVAO.h"


class MyShader: public smShader
{

public:
    GLint textUnit;
    MyShader()
    {
        setShaderFileName("shaders/Phong_VS.glsl", NULL, "shaders/Phong_FS.glsl");
        createParam("textureUnit");
    }

    virtual void initDraw(const smDrawParam &p_param)
    {
        smShader::initDraw(p_param);
        textUnit = this->getFragmentShaderParam("textureUnit");
    }
    void draw(const smDrawParam &p_param)
    {
        this->checkShaderUpdate(2000);
        //this->drawOrder=
    }
};

void VAORendering::initHapticCamMotion()
{
    hapticInterface = new smPhantomInterface();
    hapticInterface->forceEnabled = false; //disable forces right now for all devices
    hapticInterface->startDevice();
    hapticInterface->setEventDispatcher(simmedtkSDK->getEventDispatcher());
    //motionTrans=new   smHapticCameraTrans (0);
    //motionTrans->setMotionScale(0.1);
    simmedtkSDK->getEventDispatcher()->registerEventHandler(viewer, SIMMEDTK_EVENTTYPE_CAMERA_UPDATE);
    //viewer->enableCameraMotion=true;

}



///Vertex Array Object based rendering. This uses the latest rendering calling scheme called VAO
///that minimmizes the state changes. It is expected to perform better than previous rendering schemes
VAORendering::VAORendering()
{


    hapticInterface = NULL;



    ///create the SDK
    simmedtkSDK = smSDK::createSDK();
    ///create objects
    object1 = new smStaticSceneObject();
    object2 = new smStaticSceneObject();


    ///create a scene
    scene1 = simmedtkSDK->createScene();
    ///create a dummy simulator
    dummySim = new smDummySimulator(smSDK::getErrorLog());
    ///registers the dummy simulato with keyboard event
    simmedtkSDK->getEventDispatcher()->registerEventHandler(dummySim, SIMMEDTK_EVENTTYPE_KEYBOARD);

    ///texture manager init and image loading
    smTextureManager::init(smSDK::getErrorLog());
    ///For rendering the ground
    smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage"); //ground decal image
    smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage"); //gorund bum map image
    smTextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage"); //ground wall image
    smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage"); //ground wall bump image


    smTextureManager::loadTexture("../../resources/textures/fat9.bmp", "livertexture1");
    smTextureManager::loadTexture("../../resources/textures/blood.jpg", "livertexture2");


    ///load 3DS mesh for mesh
    object1->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);


    //object1->mesh->assignTexture("livertexture1");
    object1->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_VAO);
    object1->mesh->translate(7, 3, 0);
    object1->renderDetail.lineSize = 2;
    object1->renderDetail.pointSize = 5;
    object1->attachObjectSimulator(dummySim);

    ///create a second static object
    object2 = new smStaticSceneObject();
    object2->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    object2->mesh->translate(smVec3f(2, 0, 0));
    ///assigne a texture for fixed shading( not for shader enabled rendeirng)
    object2->mesh->assignTexture("livertexture2");
    object2->renderDetail.shadowColor.rgba[0] = 1.0;
    object2->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);



    //add object to the scene
    scene1->addSceneObject(object1);
    //scene1->addSceneObject(object2);

    ///create a simulator module
    simulator = simmedtkSDK->createSimulator();
    //simulator->registerObjectSimulator(dummySim);
    simulator->registerSimulationMain(this);

    ///create viewer
    viewer = simmedtkSDK->createViewer();
    viewer->viewerRenderDetail = viewer->viewerRenderDetail;
    viewer->camera()->setFieldOfView(SM_DEGREES2RADIANS(60));
    viewer->camera()->setZClippingCoefficient(10000);
    viewer->camera()->setZNearCoefficient(0.001);
    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");


    ///creaate a shader
    MyShader *myTestShader = new MyShader();
    ///Shader needs to be registered
    myTestShader->registerShader();

    ///Two very primitive shaders. One has is legacy shader and Phong_VS1 uses recent GLSL versions
    //myTestShader->setShaderFileName("shaders/Phong_VS1.glsl",NULL,"shaders/Phong_FS1.glsl");
    myTestShader->setShaderFileName("../../resources/shaders/Phong_VS.glsl", NULL, "../../resources/shaders/Phong_FS.glsl");

    ///creates a VAO
    smVAO *vao = new smVAO(NULL);
    ///fills the buffer with mesh information..assign a shader for VAO binding
    vao->setBufferDataFromMesh(object1->mesh, myTestShader);
    ///addshader to the object
    object1->renderDetail.addShader(myTestShader->uniqueId);
    ///add VAO to the object
    object1->renderDetail.addVAO(vao->uniqueId);

    ///if needed; ask viewer  to  call vao.
    viewer->addObject(vao);
    //viewer->addObject(shader);



    ///set viewer dispatcher
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());

    //viewer->addObject(&lat2);
    //viewer->addObject(spatGrid);
    //viewer->addObject(this);

    ///init hsptic camera motion. This won't have any effect if the recent GLSL shading mode is used.
    initHapticCamMotion();//initiate the haptic motion with camera

    ///run the sdk
    simmedtkSDK->run();
}

///clean up
VAORendering::~VAORendering()
{
    delete object1;
    delete object2;
    delete scene1;
    delete dummySim;

    if (hapticInterface != NULL)
    {
        delete hapticInterface;
    }


}

void VAORendering::draw(const smDrawParam &p_params)
{



}

void main()
{
    VAORendering *ex = new VAORendering();
    delete ex;

}