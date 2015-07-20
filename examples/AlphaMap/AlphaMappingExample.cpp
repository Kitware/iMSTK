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

#include "AlphaMappingExample.h"

#include "Core/SDK.h"
#include "Rendering/TextureManager.h"
#include "Mesh/Lattice.h"
#include "Mesh/LatticeTypes.h"
#include "Core/EventData.h"

/// \brief !!
void AlphaMapExample::initHapticCamMotion()
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

/// \brief !!
AlphaMapExample::AlphaMapExample()
{

    motionTrans = NULL;
    hapticInterface = NULL;

    simmedtkSDK = SDK::createSDK();
    object1 = new StaticSceneObject();

    smMetalShader *metalShader = new smMetalShader("../../resources/shaders/VertexBumpMap1.cg",
            "../../resources/shaders/FragmentBumpMap1.cg");
    metalShader->registerShader();

    scene1 = simmedtkSDK->createScene();


    //  TCHAR szEXEPath[2048];
    //  cout << GetModuleFileName ( NULL, szEXEPath, 2048 );



    smTextureManager::init(SDK::getErrorLog());
    smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage");
    smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage");
    smTextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage");
    smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage");
    smTextureManager::loadTexture("../../resources/textures/Tissue.jpg", "diffuse");
    smTextureManager::loadTexture("../../resources/textures/Tissue_Alpha.jpg", "alpha");
    smTextureManager::loadTexture("../../resources/textures/Tissue_NORM.jpg", "norm");
    smTextureManager::loadTexture("../../resources/textures/Tissue_SPEC.jpg", "spec");
    smTextureManager::loadTexture("../../resources/textures/band.bmp", "noOCC");

    object1->mesh->loadMeshLegacy("../../resources/models/gall_tissue.3DS", SM_FILETYPE_3DS);

    metalShader->attachTexture(object1->mesh->getUniqueId(), "norm", "BumpTex");
    metalShader->attachTexture(object1->mesh->getUniqueId(), "diffuse", "DecalTex");
    metalShader->attachTexture(object1->mesh->getUniqueId(), "spec", "SpecularTex");
    metalShader->attachTexture(object1->mesh->getUniqueId(), "noOCC", "OCCTex");
    metalShader->attachTexture(object1->mesh->getUniqueId(), "alpha", "AlphaTex");

    object1->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES |
                                        SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_TRANSPARENT);
    object1->getRenderDetail()->addShader(metalShader->getUniqueId());
    object1->mesh->translate(0, 10, -110);
    object1->mesh->scale(core::Vec3d(5, 5, 5));
    object1->getRenderDetail()->lineSize = 2;
    object1->getRenderDetail()->pointSize = 5;

    /// add object to the scene
    scene1->addSceneObject(object1);

    viewer = simmedtkSDK->createViewer();
    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");

    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());
    viewer->camera()->setZClippingCoefficient(1000);
    viewer->camera()->setZNearCoefficient(0.001);
    viewer->list();
    viewer->addObject(this);
    viewer->addObject(metalShader);

    simmedtkSDK->run();
}

/// \brief free alocated memory
AlphaMapExample::~AlphaMapExample()
{
    delete object1;
    delete scene1;

    if (motionTrans != NULL)
    {
        delete motionTrans;
    }

    if (hapticInterface != NULL)
    {
        delete hapticInterface;
    }
}

/// \brief
void AlphaMapExample::draw()
{
}

/// \brief extern function
int main()
{
    AlphaMapExample *ex = new AlphaMapExample();
    delete ex;
    return 0;
}
