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
#include "smCore/smEventData.h"
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

    // Init dispacther for events
    sdk->getEventDispatcher()->registerEventHandler(std::static_pointer_cast<smEventHandler>(defaultSimulator), smEventType(SIMMEDTK_EVENTTYPE_KEYBOARD));

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
    collisionModelA->getMesh()->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    collisionModelA->getMesh()->translate(7, 3, 0);
    collisionModelA->getMesh()->renderDetail.lineSize = 2;
    collisionModelA->getMesh()->renderDetail.pointSize = 5;

    std::shared_ptr<smMeshCollisionModel> collisionModelB = std::make_shared<smMeshCollisionModel>();
    collisionModelB->loadTriangleMesh("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->translate(smVec3f(2, 0, 0));
    collisionModelB->getMesh()->assignTexture("livertexture2");
    collisionModelB->getMesh()->renderDetail.shadowColor.rgba[0] = 1.0;
    collisionModelB->getMesh()->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

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
    std::shared_ptr<smLight> light = smLight::getDefaultLightning();
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

    // set event dispatcher to the viewer
    viewer->setEventDispatcher(sdk->getEventDispatcher());
    simulator->registerSimulationMain(std::static_pointer_cast<smSimulationMain>(shared_from_this()));
}

// Draw the collided triangles. This will be called due to the function call viewer->addObject(this)
void CollisionDetectionBVH::draw(const smDrawParam &/*p_params*/)
{
//     auto &triangles = collisionDetection->getCollidedTriangles();
//     glBegin(GL_TRIANGLES);
//
//     for (size_t i = 0; i < triangles.size(); i++)
//     {
//         glVertex3fv(triangles[i]->tri1.vert[0].data());
//         glVertex3fv(triangles[i]->tri1.vert[1].data());
//         glVertex3fv(triangles[i]->tri1.vert[2].data());
//
//         glVertex3fv(triangles[i]->tri2.vert[0].data());
//         glVertex3fv(triangles[i]->tri2.vert[1].data());
//         glVertex3fv(triangles[i]->tri2.vert[2].data());
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

void CollisionDetectionBVH::handleEvent(std::shared_ptr<smEvent> p_event)
{
    switch (p_event->getEventType().eventTypeCode)
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            std::shared_ptr<smCamera> cam = scene->getCamera();
            auto kbData = std::static_pointer_cast<smKeyboardEventData>(p_event->getEventData());
            smKey key = kbData->keyBoardKey;
            if (key == smKey::Escape && kbData->pressed)
            {
                //Tell the framework to shutdown
                sdk->shutDown();
            }
            else if (key == smKey::W && kbData->pressed)
            {
                if (smModKey::shift == (kbData->modKeys & smModKey::shift))
                {
                    //Move the camera up
                    cam->setCameraPos(cam->pos.x, cam->pos.y + 1, cam->pos.z);
                    cam->setCameraFocus(cam->fp.x, cam->fp.y + 1, cam->fp.z);
                    cam->genViewMat();
                }
                else
                {
                    //Move the camera forward
                    cam->setCameraPos(cam->pos.x, cam->pos.y, cam->pos.z - 1);
                    cam->setCameraFocus(cam->fp.x, cam->fp.y, cam->fp.z - 1);
                    cam->genViewMat();
                }
            }
            else if (key == smKey::A && kbData->pressed)
            {
                //Move the camera to the left
                cam->setCameraPos(cam->pos.x - 1, cam->pos.y, cam->pos.z);
                cam->setCameraFocus(cam->fp.x - 1, cam->fp.y, cam->fp.z);
                cam->genViewMat();
            }
            else if (key == smKey::S && kbData->pressed)
            {
                //Move the camera backward
                if (smModKey::shift == (kbData->modKeys & smModKey::shift))
                {
                    cam->setCameraPos(cam->pos.x, cam->pos.y - 1, cam->pos.z);
                    cam->setCameraFocus(cam->fp.x, cam->fp.y - 1, cam->fp.z);
                    cam->genViewMat();
                }
                else
                {
                    cam->setCameraPos(cam->pos.x, cam->pos.y, cam->pos.z + 1);
                    cam->setCameraFocus(cam->fp.x, cam->fp.y, cam->fp.z + 1);
                    cam->genViewMat();
                }
            }
            else if (key == smKey::D && kbData->pressed)
            {
                //Move the camera to the right
                cam->setCameraPos(cam->pos.x + 1, cam->pos.y, cam->pos.z);
                cam->setCameraFocus(cam->fp.x + 1, cam->fp.y, cam->fp.z);
                cam->genViewMat();
            }
            break;
        }
        case SIMMEDTK_EVENTTYPE_MOUSE_BUTTON:
        {
            auto mbData = std::static_pointer_cast<smMouseButtonEventData>(p_event->getEventData());
            std::cout << "mbData: button: ";
            if (mbData->mouseButton == smMouseButton::Left)
                std::cout << "Left";
            else if (mbData->mouseButton == smMouseButton::Right)
                std::cout << "Right";
            else if (mbData->mouseButton == smMouseButton::Middle)
                std::cout << "Middle";
            else
                std::cout << "Unknown";

            std::cout << " pressed: ";
            if(mbData->pressed)
                std::cout << "true";
            else
                std::cout << "false";

            std::cout << " x: " << mbData->windowX << " y: " << mbData->windowY << "\n";
            break;
        }
        case SIMMEDTK_EVENTTYPE_MOUSE_MOVE:
        {
            auto mpData = std::static_pointer_cast<smMouseMoveEventData>(p_event->getEventData());
            std::cout << "mpData: x: " << mpData->windowX
            << " y: " << mpData->windowY << "\n";
            break;
        }
        default:
            break;
    }
}

