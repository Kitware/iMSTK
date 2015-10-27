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

#include "Examples/common/wasdCameraController.h"
#include "Examples/common/KeyPressSDKShutdown.h"
#include "Examples/common/pzrMouseCameraController.h"
#include "Examples/common/ExampleCube.h"
#include "Examples/common/TexturedSquare.h"

#include <memory>

#include "IO/initIO.h"
#include "RenderDelegates/initRenderDelegates.h"
#include "VTKRendering/initVTKRendering.h"
#include "Rendering/TextureManager.h"
#include "Geometry/MeshModel.h"
#include "Core/Factory.h"
#include "Core/ViewerBase.h"

int main()
{
    using ExampleCube = mstk::Examples::Common::ExampleCube;
    using TexturedSquare = mstk::Examples::Common::TexturedSquare;

    ExampleCube cube;
    TexturedSquare square;

    initRenderDelegates();
    initIODelegates();

    auto scene1 = std::make_shared<Scene>();
    auto scene2 = std::make_shared<Scene>();

    // Create a viewer
    auto viewer = std::make_shared<OculusViewer>();

    assert(viewer);
    assert(scene1);
    assert(scene2);

    cube.useVTKRenderer(false);
    cube.setup();

    square.useVTKRenderer(false);
    square.setup();

    //setup scene for displaying the rendered to texture object
    //Create a color and depth texture for the FBO
    TextureManager::createColorTexture("colorTex1", 64, 64);
    TextureManager::createDepthTexture("depthTex1", 64, 64);

    viewer->addFBO("fbo1",
                   TextureManager::getTexture("colorTex1"),
                   TextureManager::getTexture("depthTex1"),
                   64, 64);

    square.setTexture("colorTex1");

    // Add the cube to the scene to be rendered
    scene1->addSceneObject(cube.getStaticSceneObject());

    //Add the square to the scene to be rendered
    scene2->addSceneObject(square.getStaticSceneObject());

    // Register the scene with the viewer, and setup render target
    viewer->registerScene(scene1, SMRENDERTARGET_FBO, "fbo1");
    viewer->registerScene(scene2, SMRENDERTARGET_SCREEN, "");

    // Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TO TEXTURE TEST");

    // Set some viewer properties
    viewer->setScreenResolution(800, 640);

    // Setup Scene lighting
    auto light1 = Light::getDefaultLighting();
    assert(light1);
    scene1->addLight(light1);

    auto light2 = Light::getDefaultLighting();
    assert(light2);
    light2->lightPos.setPosition(core::Vec3d(0.0, 0.0, 5.0));
    scene2->addLight(light2);

    // Camera setup
    auto sceneCamera1 = Camera::getDefaultCamera();
    assert(sceneCamera1);
    sceneCamera1->setPos(3, 3, 5);
    sceneCamera1->setFocus(0, 0, -1);
    sceneCamera1->genProjMat();
    sceneCamera1->genViewMat();
    scene1->addCamera(sceneCamera1);

    auto sceneCamera2 = Camera::getDefaultCamera();
    assert(sceneCamera2);
    sceneCamera2->setPos(0, 0, 5);
    sceneCamera2->setFocus(0, 0, -1);
    sceneCamera2->genProjMat();
    sceneCamera2->genViewMat();
    scene2->addCamera(sceneCamera2);

    // Create the camera controller
    auto camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    auto keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();
    auto pzrCamCtl = std::make_shared<mstk::Examples::Common::pzrMouseCameraController>();
    camCtl->setCamera(sceneCamera1);
    pzrCamCtl->setCamera(sceneCamera1);

    // Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(core::EventType::Keyboard, camCtl);
    viewer->attachEvent(core::EventType::Keyboard, keyShutdown);
    viewer->attachEvent(core::EventType::MouseMove, pzrCamCtl);
    viewer->attachEvent(core::EventType::MouseButton, pzrCamCtl);

    viewer->exec();

    return 0;
}
