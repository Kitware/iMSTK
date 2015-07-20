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
#include "../common/pzrMouseCameraController.h"

#include <memory>

#include "Core/SDK.h"
#include "Rendering/TextureManager.h"
#include "Geometry/MeshModel.h"
#include "RenderDelegates/Config.h"

int main()
{
    SIMMEDTK_REGISTER_RENDER_DELEGATES();
    std::shared_ptr<SDK> sdk;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<Scene> scene1;
    std::shared_ptr<Light> light;
    std::shared_ptr<smCamera> sceneCamera;
    std::shared_ptr<StaticSceneObject> cube;
    std::shared_ptr<mstk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<mstk::Examples::Common::KeyPressSDKShutdown> keyShutdown;
    std::shared_ptr<mstk::Examples::Common::pzrMouseCameraController> pzrCamCtl;
    //Create an instance of the SimMedTK framework/SDK
    sdk = SDK::getInstance();

    //Create a new scene to work in
    scene1 = sdk->createScene();

    //Create a viewer to see the scene through
    viewer = std::make_shared<smViewer>();
    sdk->addViewer(viewer);

    //Create the camera controller
    camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();
    pzrCamCtl = std::make_shared<mstk::Examples::Common::pzrMouseCameraController>();

    auto cubeModel = std::make_shared<smMeshModel>();
    cubeModel->load("models/cube.obj", "textures/cube.png", "cubetex");

    auto renderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    cubeModel->setRenderDetail(renderDetail);

    cube = std::make_shared<StaticSceneObject>();
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
    light = Light::getDefaultLighting();
    assert(light);
    scene1->addLight(light);

    // Camera setup
    sceneCamera = smCamera::getDefaultCamera();
    assert(sceneCamera);
    sceneCamera->setPos(3, 3, 5);
    sceneCamera->setFocus(0, 0, -1);
    sceneCamera->genProjMat();
    sceneCamera->genViewMat();
    scene1->addCamera(sceneCamera);
    camCtl->setCamera(sceneCamera);
    pzrCamCtl->setCamera(sceneCamera);

    //Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(mstk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(mstk::Event::EventType::Keyboard, keyShutdown);
    viewer->attachEvent(mstk::Event::EventType::MouseMove, pzrCamCtl);
    viewer->attachEvent(mstk::Event::EventType::MouseButton, pzrCamCtl);

    //run the framework
    sdk->run();

    //cleanup
    sdk->releaseScene(scene1);

    return 0;
}
