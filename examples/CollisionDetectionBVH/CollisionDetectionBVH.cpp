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

#include <memory>

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smCollision/smCollisionPair.h"

CollisionDetectionBVH::CollisionDetectionBVH()
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
    collisionModelA->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    collisionModelA->getMesh()->translate(7, 3, 0);
    collisionModelA->getMesh()->getRenderDetail()->lineSize = 2;
    collisionModelA->getMesh()->getRenderDetail()->pointSize = 5;

    std::shared_ptr<smMeshCollisionModel> collisionModelB = std::make_shared<smMeshCollisionModel>();
    collisionModelB->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->translate(smVec3d(2, 0, 0));
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->getRenderDetail()->shadowColor.rgba[0] = 1.0;
    collisionModelB->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

    // Add models to a collision pair so they can be queried for collision
    std::shared_ptr<smCollisionPair> collisionPair = std::make_shared<smCollisionPair>();
    collisionPair->setModels(collisionModelA,collisionModelB);

    // Collision detection to be used
    collisionDetection = std::make_shared<smMeshToMeshCollision>();

    // Create a static scene
    modelA = std::make_shared<smStaticSceneObject>();
    modelA->setMesh(collisionModelA->getMesh());
    sdk->registerSceneObject(modelA);
    sdk->registerMesh(modelA->mesh);

    // Attach object to the dummy simulator. it will be simulated by dummy simulator
    modelA->attachObjectSimulator(defaultSimulator);

    // Initialize the scecond object
    modelB = std::make_shared<smStaticSceneObject>();
    modelB->setMesh(collisionModelB->getMesh());
    sdk->registerSceneObject(modelB);
    sdk->registerMesh(modelB->mesh);

    // Add object to the scene
    scene->addSceneObject(modelA);
    scene->addSceneObject(modelB);

    // Setup Scene lighting
    std::shared_ptr<smLight> light = smLight::getDefaultLighting();
    scene->addLight(light);

    // Camera setup
    std::shared_ptr<smCamera> sceneCamera = smCamera::getDefaultCamera();
    scene->addCamera(sceneCamera);

    // Create a simulator module
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(defaultSimulator);
    simulator->registerCollisionDetection(collisionDetection);
    simulator->addCollisionPair(collisionPair);

    // setup viewer
    viewer->setWindowTitle("SimMedTK CollisionHash Example");
    viewer->setScreenResolution(800, 640);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");
    viewer->addObject(collisionModelA->getAABBTree());
    viewer->addObject(collisionModelB->getAABBTree());
    // set event dispatcher to the viewer
//     viewer->setEventDispatcher(sdk->getEventDispatcher());
//     simulator->registerSimulationMain(std::static_pointer_cast<smSimulationMain>(shared_from_this()));
}

// Draw the collided triangles. This will be called due to the function call viewer->addObject(this)
void CollisionDetectionBVH::draw(const smDrawParam &/*p_params*/)
{
//     auto &triangles = collisionDetection->getCollidedTriangles();
//     glBegin(GL_TRIANGLES);
//
//     for (size_t i = 0; i < triangles.size(); i++)
//     {
//         glVertex3dv(triangles[i]->tri1.vert[0].data());
//         glVertex3dv(triangles[i]->tri1.vert[1].data());
//         glVertex3dv(triangles[i]->tri1.vert[2].data());
//
//         glVertex3dv(triangles[i]->tri2.vert[0].data());
//         glVertex3dv(triangles[i]->tri2.vert[1].data());
//         glVertex3dv(triangles[i]->tri2.vert[2].data());
//     }
//     glEnd();
}

void CollisionDetectionBVH::simulateMain(const smSimulationMainParam &/*p_param*/)
{
    if ((10 > moveObj) && (moveObj > 0))
    {
        modelB->mesh->translate(1, 0, 0);
        moveObj--;
    }
    else
    {
        moveObj = 9; // reset
        modelB->mesh->translate(-moveObj, 0, 0);
    }
}

void CollisionDetectionBVH::run()
{
    this->sdk->run();
}
