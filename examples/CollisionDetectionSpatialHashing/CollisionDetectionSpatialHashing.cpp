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

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smCore/smEventData.h"

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
    spatialHashing = std::make_shared<smSpatialHash>(10000, 2, 2, 2);

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

    // Create a static scene
    modelA = std::make_shared<smStaticSceneObject>();
    sdk->registerSceneObject(modelA);
    sdk->registerMesh(modelA->mesh);

    // Load mesh
    modelA->mesh->loadMeshLegacy("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);

    // Assign a texture
    modelA->mesh->assignTexture("livertexture1");

    // Set the rendering features
    modelA->mesh->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    modelA->mesh->translate(7, 3, 0);
    modelA->mesh->renderDetail.lineSize = 2;
    modelA->mesh->renderDetail.pointSize = 5;

    // Attach object to the dummy simulator. it will be simulated by dummy simulator
    modelA->attachObjectSimulator(defaultSimulator);
    spatialHashing->addMesh(modelA->mesh);

    // Initialize the scecond object
    modelB = std::make_shared<smStaticSceneObject>();
    sdk->registerSceneObject(modelB);
    sdk->registerMesh(modelB->mesh);

    modelB->mesh->loadMeshLegacy("models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    modelB->mesh->translate(smVec3f(2, 0, 0));

    modelB->mesh->assignTexture("livertexture2");
    modelB->mesh->renderDetail.shadowColor.rgba[0] = 1.0;
    modelB->mesh->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    spatialHashing->addMesh(modelB->mesh);

    // Add object to the scene
    scene->addSceneObject(modelA);
    scene->addSceneObject(modelB);

    // Setup Scene lighting
    auto light = std::make_shared<smLight>("SceneLight1",
                                           SIMMEDTK_LIGHT_SPOTLIGHT,
                                           SIMMEDTK_LIGHTPOS_WORLD);
    light->lightPos.setPosition(smVec3f(10.0, 10.0, 10.0));
    light->lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
    light->lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
    light->lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
    light->spotCutOffAngle = 60;
    light->direction = smVec3f(0.0, 0.0, -1.0);
    light->drawEnabled = false;
    light->attn_constant = 1.0;
    light->attn_linear = 0.0;
    light->attn_quadratic = 0.0;
    scene->addLight(light);

    // Camera setup
    smCamera& sceneCamera = scene->getCamera();
    sceneCamera.setAspectRatio(800.0/640.0); //Doesn't have to match screen resolution
    sceneCamera.setFarClipDist(1000);
    sceneCamera.setNearClipDist(0.001);
    sceneCamera.setViewAngle(0.785398f); //45 degrees
    sceneCamera.setCameraPos(0, 20, 10);
    sceneCamera.setCameraFocus(0, 0, 0);
    sceneCamera.setCameraUpVec(0, 1, 0);
    sceneCamera.genProjMat();
    sceneCamera.genViewMat();

    // Create a simulator module
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(defaultSimulator);
    simulator->registerCollisionDetection(spatialHashing);

    // setup viewer
    viewer->setWindowTitle("SimMedTK CollisionHash Example");
    viewer->setScreenResolution(800, 640);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");

    // we want viewer to render this object
//     viewer->addObject(shared_from_this());

    // set event dispatcher to the viewer
    viewer->setEventDispatcher(sdk->getEventDispatcher());
//     simulator->registerSimulationMain(safeDownCast<CollisionDetectionSpatialHashing>());
}

// Draw the collided triangles. This will be called due to the function call viewer->addObject(this)
void CollisionDetectionSpatialHashing::draw(const smDrawParam &/*p_params*/)
{
    auto &triangles = spatialHashing->getCollidedTriangles();
    glBegin(GL_TRIANGLES);

    for (size_t i = 0; i < triangles.size(); i++)
    {
        glVertex3fv(triangles[i]->tri1.vert[0].data());
        glVertex3fv(triangles[i]->tri1.vert[1].data());
        glVertex3fv(triangles[i]->tri1.vert[2].data());

        glVertex3fv(triangles[i]->tri2.vert[0].data());
        glVertex3fv(triangles[i]->tri2.vert[1].data());
        glVertex3fv(triangles[i]->tri2.vert[2].data());
    }
    glEnd();
}

void CollisionDetectionSpatialHashing::simulateMain(const smSimulationMainParam &/*p_param*/)
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

void CollisionDetectionSpatialHashing::run()
{
    this->sdk->run();
}

void CollisionDetectionSpatialHashing::handleEvent(std::shared_ptr<smEvent> p_event)
{
    switch (p_event->getEventType().eventTypeCode)
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            auto kbData = std::static_pointer_cast<smKeyboardEventData>(p_event->getEventData());
            smKey key = kbData->keyBoardKey;
            if (key == smKey::Escape && kbData->pressed)
            {
                //Tell the framework to shutdown
                sdk->shutDown();
            }
            else if (key == smKey::W && kbData->pressed)
            {
                smCamera cam = scene->getCamera();
                if (smModKey::shift == (kbData->modKeys & smModKey::shift))
                {
                    //Move the camera up
                    scene->getCamera().setCameraPos(cam.pos.x, cam.pos.y + 1, cam.pos.z);
                    scene->getCamera().setCameraFocus(cam.fp.x, cam.fp.y + 1, cam.fp.z);
                    scene->getCamera().genViewMat();
                }
                else
                {
                    //Move the camera forward
                    scene->getCamera().setCameraPos(cam.pos.x, cam.pos.y, cam.pos.z - 1);
                    scene->getCamera().setCameraFocus(cam.fp.x, cam.fp.y, cam.fp.z - 1);
                    scene->getCamera().genViewMat();
                }
            }
            else if (key == smKey::A && kbData->pressed)
            {
                //Move the camera to the left
                smCamera cam = scene->getCamera();
                scene->getCamera().setCameraPos(cam.pos.x - 1, cam.pos.y, cam.pos.z);
                scene->getCamera().setCameraFocus(cam.fp.x - 1, cam.fp.y, cam.fp.z);
                scene->getCamera().genViewMat();
            }
            else if (key == smKey::S && kbData->pressed)
            {
                //Move the camera backward
                smCamera cam = scene->getCamera();
                if (smModKey::shift == (kbData->modKeys & smModKey::shift))
                {
                    scene->getCamera().setCameraPos(cam.pos.x, cam.pos.y - 1, cam.pos.z);
                    scene->getCamera().setCameraFocus(cam.fp.x, cam.fp.y - 1, cam.fp.z);
                    scene->getCamera().genViewMat();
                }
                else
                {
                    scene->getCamera().setCameraPos(cam.pos.x, cam.pos.y, cam.pos.z + 1);
                    scene->getCamera().setCameraFocus(cam.fp.x, cam.fp.y, cam.fp.z + 1);
                    scene->getCamera().genViewMat();
                }
            }
            else if (key == smKey::D && kbData->pressed)
            {
                //Move the camera to the right
                smCamera cam = scene->getCamera();
                scene->getCamera().setCameraPos(cam.pos.x + 1, cam.pos.y, cam.pos.z);
                scene->getCamera().setCameraFocus(cam.fp.x + 1, cam.fp.y, cam.fp.z);
                scene->getCamera().genViewMat();
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

