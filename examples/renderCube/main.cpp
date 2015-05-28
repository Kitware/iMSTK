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

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"

#include <memory>

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smCollision/smMeshCollisionModel.h"

int main()
{
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smScene> scene1;
    std::shared_ptr<smLight> light;
    std::shared_ptr<smCamera> sceneCamera;
    std::shared_ptr<smStaticSceneObject> cube;
    std::shared_ptr<smtk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<smtk::Examples::Common::KeyPressSDKShutdown> keyShutdown;

    //Create an instance of the SimMedTK framework/SDK
    sdk = smSDK::getInstance();

    //Create a new scene to work in
    scene1 = sdk->createScene();

    //Create a viewer to see the scene through
    viewer = std::make_shared<smViewer>();
    sdk->addViewer(viewer);

    //Create the camera controller
    camCtl = std::make_shared<smtk::Examples::Common::wasdCameraController>();
    keyShutdown = std::make_shared<smtk::Examples::Common::KeyPressSDKShutdown>();

    //Initialize the texture manager
    smTextureManager::init(sdk->getErrorLog());

    std::shared_ptr<smMeshCollisionModel> cubeModel = std::make_shared<smMeshCollisionModel>();
    cubeModel->loadTriangleMesh("models/cube.obj", SM_FILETYPE_OBJ);
    cubeModel->getMesh()->assignTexture("cubetex");
    cubeModel->getMesh()->getRenderDetail()->renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);

    //Load in the texture for the cube model
    smTextureManager::loadTexture("textures/cube.png", "cubetex");

    cube = std::make_shared<smStaticSceneObject>();
    cube->setModel(cubeModel);

    //Add the cube to the scene to be rendered
    scene1->addSceneObject(cube);

    //Register the scene with the viewer, and setup render target
    viewer->registerScene(scene1, SMRENDERTARGET_SCREEN, "");

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TEST");

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    // Setup Scene lighting
    light = smLight::getDefaultLighting();
    assert(light);
    scene1->addLight(light);

    // Camera setup
    sceneCamera = smCamera::getDefaultCamera();
    assert(sceneCamera);
    sceneCamera->setCameraPos(3, 3, 5);
    sceneCamera->setCameraFocus(0, 0, -1);
    sceneCamera->genProjMat();
    sceneCamera->genViewMat();
    scene1->addCamera(sceneCamera);
    camCtl->setCamera(sceneCamera);

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(smtk::Event::EventType::Keyboard, keyShutdown);

    //run the framework
    sdk->run();

    //cleanup
    sdk->releaseScene(scene1);

    return 0;
}
