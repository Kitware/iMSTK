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

#include "CollisionDetectionSpatialHashing.h"

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"

#include "smCore/smSDK.h"
#include "smRendering/smTextureManager.h"
#include "smCore/smCollisionPair.h"
#include "smCollision/smMeshCollisionModel.h"

CollisionDetectionSpatialHashing::CollisionDetectionSpatialHashing()
{
    moveObj = 9;

    // Create the sdk
    sdk = smSDK::getInstance();

    // Create scene
    scene = sdk->createScene();

    // Create viewer
    viewer = std::make_shared<smViewer>();

    // Add our viewer to the SDK
    sdk->addViewer(viewer);

    // Intializes the spatial spatialHashinging
    spatialHashing = std::make_shared<smSpatialHashCollision>(10000, 2, 2, 2);

    //Create the camera controller
    std::shared_ptr<smtk::Examples::Common::wasdCameraController> camCtl = std::make_shared<smtk::Examples::Common::wasdCameraController>();
    std::shared_ptr<smtk::Examples::Common::KeyPressSDKShutdown> keyShutdown = std::make_shared<smtk::Examples::Common::KeyPressSDKShutdown>();

    // Create dummy simulator
    defaultSimulator = std::make_shared<smDummySimulator>(sdk->getErrorLog());
    sdk->registerObjectSim(defaultSimulator);

    // Init texture manager and specify the textures needed for the current application
    smTextureManager::init(sdk->getErrorLog());
    smTextureManager::loadTexture("textures/fat9.bmp", "livertexture1");
    smTextureManager::loadTexture("textures/blood.jpg", "livertexture2");

    smTextureManager::loadTexture("textures/4351-diffuse.jpg", "groundImage");
    smTextureManager::loadTexture("textures/4351-normal.jpg", "groundBumpImage");
    smTextureManager::loadTexture("textures/brick.jpg", "wallImage");
    smTextureManager::loadTexture("textures/brick-normal.jpg", "wallBumpImage");

        // Create collision models
    std::shared_ptr<smMeshCollisionModel> collisionModelA = std::make_shared<smMeshCollisionModel>();
    collisionModelA->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelA->getMesh()->assignTexture("livertexture1");
    collisionModelA->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);
    collisionModelA->getMesh()->translate(7, 3, 0);
    collisionModelA->getMesh()->getRenderDetail()->lineSize = 2;
    collisionModelA->getMesh()->getRenderDetail()->pointSize = 5;

    std::shared_ptr<smMeshCollisionModel> collisionModelB = std::make_shared<smMeshCollisionModel>();
    collisionModelB->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->translate(smVec3d(2, 0, 0));
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->getRenderDetail()->shadowColor.rgba[0] = 1.0;
    collisionModelB->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);
    
    // Create a static scene
    modelA = std::make_shared<smStaticSceneObject>();
    modelA->setModel(collisionModelA);
    sdk->registerSceneObject(modelA);
    sdk->registerMesh(collisionModelA->getMesh());

    // Attach object to the dummy simulator. it will be simulated by dummy simulator
    modelA->attachObjectSimulator(defaultSimulator);
    spatialHashing->addMesh(collisionModelA->getMesh());

    // Initialize the scecond object
    modelB = std::make_shared<smStaticSceneObject>();
    sdk->registerSceneObject(modelB);
    sdk->registerMesh(collisionModelB->getMesh());

    spatialHashing->addMesh(collisionModelB->getMesh());

    // Add object to the scene
    scene->addSceneObject(modelA);
    scene->addSceneObject(modelB);

    // Setup Scene lighting
    auto light = smLight::getDefaultLighting();
    assert(light);
    scene->addLight(light);

    // Camera setup
    std::shared_ptr<smCamera> sceneCamera = smCamera::getDefaultCamera();
    assert(sceneCamera);
    scene->addCamera(sceneCamera);
    camCtl->setCamera(sceneCamera);

    // Create a simulator module
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(defaultSimulator);
    simulator->registerCollisionDetection(spatialHashing);

    // Create dummy collision pair
    std::shared_ptr<smCollisionPair> collisionPair = std::make_shared<smCollisionPair>();
    collisionPair->setModels(collisionModelA,collisionModelB);
    simulator->addCollisionPair(collisionPair);

    // setup viewer
    viewer->setWindowTitle("SimMedTK CollisionHash Example");
    viewer->setScreenResolution(800, 640);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(smtk::Event::EventType::Keyboard, keyShutdown);
}

void CollisionDetectionSpatialHashing::simulateMain(const smSimulationMainParam &/*p_param*/)
{
    if ((10 > moveObj) && (moveObj > 0))
    {
        modelB->getModel()->getMesh()->translate(1, 0, 0);
        moveObj--;
    }
    else
    {
        moveObj = 9; // reset
        modelB->getModel()->getMesh()->translate(-moveObj, 0, 0);
    }

	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void CollisionDetectionSpatialHashing::run()
{
    this->sdk->run();
}
