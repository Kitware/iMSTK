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

#include "CollisionDetectionBVH.h"

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"

#include <memory>

#include "Core/SDK.h"
#include "Rendering/TextureManager.h"
#include "Core/CollisionPair.h"

CollisionDetectionBVH::CollisionDetectionBVH()
{
    moveObj = 9;

    // Create the sdk
    sdk = SDK::getInstance();

    // Create scene
    scene = sdk->createScene();

    // Create viewer
    viewer = std::make_shared<smViewer>();

    // Add our viewer to the SDK
    sdk->addViewer(viewer);

    //Create the camera controller
    std::shared_ptr<mstk::Examples::Common::wasdCameraController> camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    std::shared_ptr<mstk::Examples::Common::KeyPressSDKShutdown> keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();

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
    std::shared_ptr<MeshCollisionModel> collisionModelA = std::make_shared<MeshCollisionModel>();
    collisionModelA->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelA->getMesh()->assignTexture("livertexture1");
    collisionModelA->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);
    collisionModelA->getMesh()->translate(7, 3, 0);
    collisionModelA->getMesh()->getRenderDetail()->lineSize = 2;
    collisionModelA->getMesh()->getRenderDetail()->pointSize = 5;

    std::shared_ptr<MeshCollisionModel> collisionModelB = std::make_shared<MeshCollisionModel>();
    collisionModelB->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->translate(core::Vec3d(2, 0, 0));
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->getRenderDetail()->shadowColor.rgba[0] = 1.0;
    collisionModelB->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);

    // Add models to a collision pair so they can be queried for collision
    std::shared_ptr<CollisionPair> collisionPair = std::make_shared<CollisionPair>();
    collisionPair->setModels(collisionModelA,collisionModelB);

    // Collision detection to be used
    collisionDetection = std::make_shared<MeshToMeshCollision>();

    // Create a static scene
    modelA = std::make_shared<StaticSceneObject>();
    modelA->setModel(collisionModelA);
    sdk->registerSceneObject(modelA);
    sdk->registerMesh(collisionModelA->getMesh());

    // Attach object to the dummy simulator. it will be simulated by dummy simulator
    modelA->attachObjectSimulator(defaultSimulator);

    // Initialize the scecond object
    modelB = std::make_shared<StaticSceneObject>();
    modelB->setModel(collisionModelB);
    sdk->registerSceneObject(modelB);
    sdk->registerMesh(collisionModelB->getMesh());

    // Add object to the scene
    scene->addSceneObject(modelA);
    scene->addSceneObject(modelB);

    // Setup Scene lighting
    auto light = Light::getDefaultLighting();
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
    simulator->registerCollisionDetection(collisionDetection);
    simulator->addCollisionPair(collisionPair);

    // setup viewer
    viewer->setWindowTitle("SimMedTK Collision BVH Example");
    viewer->setScreenResolution(800, 640);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");
    viewer->addObject(collisionModelA->getAABBTree());
    viewer->addObject(collisionModelB->getAABBTree());

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(mstk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(mstk::Event::EventType::Keyboard, keyShutdown);
}

void CollisionDetectionBVH::simulateMain(const SimulationMainParam &/*p_param*/)
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

void CollisionDetectionBVH::run()
{
    this->sdk->run();
}
