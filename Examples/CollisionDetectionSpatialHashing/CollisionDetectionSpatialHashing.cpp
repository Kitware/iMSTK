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

#include "Core/SDK.h"
#include "Rendering/TextureManager.h"
#include "Core/CollisionPair.h"
#include "Collision/MeshCollisionModel.h"

CollisionDetectionSpatialHashing::CollisionDetectionSpatialHashing()
{
    moveObj = 9;

    // Create the sdk
    sdk = SDK::getInstance();

    // Create scene
    scene = sdk->createScene();

    // Create viewer
    viewer = std::make_shared<OpenGLViewer>();

    // Add our viewer to the SDK
    sdk->addViewer(viewer);

    // Intializes the spatial spatialHashinging
    spatialHashing = std::make_shared<SpatialHashCollision>(10000, 2, 2, 2);

    //Create the camera controller
    std::shared_ptr<mstk::Examples::Common::wasdCameraController> camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    std::shared_ptr<mstk::Examples::Common::KeyPressSDKShutdown> keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();

    // Create dummy simulator
    defaultSimulator = std::make_shared<DefaultSimulator>(sdk->getErrorLog());
    sdk->registerObjectSimulator(defaultSimulator);

    // Init texture manager and specify the textures needed for the current application
    TextureManager::init(sdk->getErrorLog());
    TextureManager::loadTexture("textures/fat9.bmp", "livertexture1");
    TextureManager::loadTexture("textures/blood.jpg", "livertexture2");

    TextureManager::loadTexture("textures/4351-diffuse.jpg", "groundImage");
    TextureManager::loadTexture("textures/4351-normal.jpg", "groundBumpImage");
    TextureManager::loadTexture("textures/brick.jpg", "wallImage");
    TextureManager::loadTexture("textures/brick-normal.jpg", "wallBumpImage");

        // Create collision models
    std::shared_ptr<MeshCollisionModel> collisionModelA = std::make_shared<MeshCollisionModel>();
    collisionModelA->loadTriangleMesh("models/liverNormalized_SB2.3DS");
    auto mesh = std::static_pointer_cast<SurfaceMesh>(collisionModelA->getMesh());
    mesh->assignTexture("livertexture1");
    mesh->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);
    mesh->translate(Eigen::Translation3d(core::Vec3d(7, 3, 0)));
    mesh->getRenderDetail()->lineSize = 2;
    mesh->getRenderDetail()->pointSize = 5;

    std::shared_ptr<MeshCollisionModel> collisionModelB = std::make_shared<MeshCollisionModel>();
    collisionModelB->loadTriangleMesh("models/liverNormalized_SB2.3DS");
    mesh = std::static_pointer_cast<SurfaceMesh>(collisionModelB->getMesh());
    mesh->assignTexture("livertexture2");
    mesh->translate(Eigen::Translation3d(core::Vec3d(7, 3, 0)));
    mesh->assignTexture("livertexture2");
    mesh->getRenderDetail()->shadowColor.rgba[0] = 1.0;
    mesh->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_WIREFRAME);

    // Create a static scene
    modelA = std::make_shared<StaticSceneObject>();
    modelA->setModel(collisionModelA);
    sdk->registerSceneObject(modelA);
    sdk->registerMesh(collisionModelA->getMesh());

    // Attach object to the dummy simulator. it will be simulated by dummy simulator
    modelA->attachObjectSimulator(defaultSimulator);
    spatialHashing->addModel(collisionModelA);

    // Initialize the scecond object
    modelB = std::make_shared<StaticSceneObject>();
    sdk->registerSceneObject(modelB);
    sdk->registerMesh(collisionModelB->getMesh());

    spatialHashing->addModel(collisionModelB);

    // Add object to the scene
    scene->addSceneObject(modelA);
    scene->addSceneObject(modelB);

    // Setup Scene lighting
    auto light = Light::getDefaultLighting();
    scene->addLight(light);

    // Camera setup
    std::shared_ptr<Camera> sceneCamera = Camera::getDefaultCamera();
    scene->addCamera(sceneCamera);
    camCtl->setCamera(sceneCamera);

    // Create a simulator module
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(defaultSimulator);
    simulator->registerCollisionDetection(spatialHashing);

    // Create dummy collision pair
    std::shared_ptr<CollisionPair> collisionPair = std::make_shared<CollisionPair>();
    collisionPair->setModels(collisionModelA,collisionModelB);
    simulator->addCollisionPair(collisionPair);

    // setup viewer
    viewer->setWindowTitle("SimMedTK CollisionHash Example");
    viewer->setScreenResolution(800, 640);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(core::EventType::Keyboard, camCtl);
    viewer->attachEvent(core::EventType::Keyboard, keyShutdown);
}

void CollisionDetectionSpatialHashing::simulateMain(const SimulationMainParam &/*p_param*/)
{
    Eigen::Translation3d t(core::Vec3d(1, 0, 0));
    if ((10 > moveObj) && (moveObj > 0))
    {
        modelB->getModel()->getMesh()->translate(t);
        moveObj--;
    }
    else
    {
        t.vector() = core::Vec3d(-moveObj, 0, 0);
        moveObj = 9; // reset
        modelB->getModel()->getMesh()->translate(t);
    }

	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void CollisionDetectionSpatialHashing::run()
{
    this->sdk->run();
}
