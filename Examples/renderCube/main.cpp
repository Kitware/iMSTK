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

#include "IO/initIO.h"
#include "RenderDelegates/initRenderDelegates.h"
#include "VTKRendering/initVTKRendering.h"
#include "Rendering/TextureManager.h"
#include "Geometry/MeshModel.h"
#include "Core/Factory.h"
#include "Core/ViewerBase.h"

int main()
{
    initRenderDelegates();
    initVTKRendering();
    initIODelegates();
    const bool useVTKRenderer = true; // VTK is the default viewer.

    auto scene = std::make_shared<Scene>();

    // Create a viewer
    std::shared_ptr<ViewerBase> viewer
        = Factory<ViewerBase>::createSubclassForGroup("ViewerBase",RenderDelegate::VTK);
    if(!useVTKRenderer)
    {
        viewer.reset();
        viewer
            = Factory<ViewerBase>::createSubclassForGroup("ViewerBase",RenderDelegate::Other);
    }

    auto cubeModel = std::make_shared<MeshModel>();
    cubeModel->load("models/cube.obj");

    auto renderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    renderDetail->setTextureFilename("textures/cube.jpg");

    cubeModel->setRenderDetail(renderDetail);

    if(!useVTKRenderer)
    {
        TextureManager::addTexture("textures/cube.jpg", "cubetex");
        std::static_pointer_cast<SurfaceMesh>(cubeModel->getMesh())->assignTexture("cubetex");
    }

    auto cube = std::make_shared<StaticSceneObject>();

    // If you want to use the GL renderer you need to specify the appropiate render delegates
    // This can be automated in the future, for now VTK is the default renderer and the delegates
    // need to be reset.
    cube->setModel(cubeModel);
    if(!useVTKRenderer)
    {
        auto renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "StaticSceneObjectRenderDelegate",RenderDelegate::Other);
        cube->setRenderDelegate(renderDelegate);

        renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "MeshRenderDelegate",RenderDelegate::Other);
        cubeModel->getMesh()->setRenderDelegate(renderDelegate);
    }

    // Add the cube to the scene to be rendered
    scene->addSceneObject(cube);

    // Register the scene with the viewer, and setup render target
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "");

    // Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK RENDER TEST");

    // Set some viewer properties
    viewer->setScreenResolution(800, 640);

    // Setup Scene lighting
    if(!useVTKRenderer)
    {
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
    }

    viewer->exec();

    return 0;
}
