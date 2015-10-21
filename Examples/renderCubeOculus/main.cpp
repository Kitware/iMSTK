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

#include <memory>

#include "IO/initIO.h"
#include "RenderDelegates/initRenderDelegates.h"
#include "VTKRendering/initVTKRendering.h"
#include "Rendering/TextureManager.h"
#include "Rendering/OculusViewer.h"
#include "Geometry/MeshModel.h"
#include "Core/Factory.h"
#include "Core/ViewerBase.h"

int main()
{
    using ExampleCube = mstk::Examples::Common::ExampleCube;

    ExampleCube cube;
    initRenderDelegates();
    initIODelegates();

    auto scene = std::make_shared<Scene>();

    // Create a viewer
    auto viewer = std::make_shared<OculusViewer>();

    assert(viewer);

    cube.useVTKRenderer(false);
    cube.setup();

    // Add the cube to the scene to be rendered
    scene->addSceneObject(cube.getStaticSceneObject());

    // Register the scene with the viewer, and setup render target
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");

    // Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TEST");

    // Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Make the window fullscreen for oculus
    viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    auto light = Light::getDefaultLighting();
    assert(light);
    scene->addLight(light);

    // Camera setup
    auto sceneCamera = Camera::getDefaultCamera();
    assert(sceneCamera);
    sceneCamera->setPos(3, 3, 5);
    sceneCamera->setFocus(0, 0, -1);
    sceneCamera->genProjMat();
    sceneCamera->genViewMat();
    scene->addCamera(sceneCamera);

    // Create the camera controller
    auto camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    auto keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();
    auto pzrCamCtl = std::make_shared<mstk::Examples::Common::pzrMouseCameraController>();
    camCtl->setCamera(sceneCamera);
    pzrCamCtl->setCamera(sceneCamera);

    // Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(core::EventType::Keyboard, camCtl);
    viewer->attachEvent(core::EventType::Keyboard, keyShutdown);
    viewer->attachEvent(core::EventType::MouseMove, pzrCamCtl);
    viewer->attachEvent(core::EventType::MouseButton, pzrCamCtl);

    viewer->exec();

    return 0;
}
