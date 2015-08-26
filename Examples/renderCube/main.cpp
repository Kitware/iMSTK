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
    const bool useVTKRenderer = true;

    std::shared_ptr<SDK> sdk;
    std::shared_ptr<Scene> scene1;
    std::shared_ptr<Light> light;
    std::shared_ptr<Camera> sceneCamera;
    std::shared_ptr<StaticSceneObject> cube;
    std::shared_ptr<mstk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<mstk::Examples::Common::KeyPressSDKShutdown> keyShutdown;
    std::shared_ptr<mstk::Examples::Common::pzrMouseCameraController> pzrCamCtl;
    //Create an instance of the SimMedTK framework/SDK
    sdk = SDK::getInstance();

    // Default viewer is based on vtk
    std::shared_ptr<ViewerBase> viewer;
    if(useVTKRenderer)
    {
        viewer = sdk->createViewer();
    }
    else
    {
        viewer = std::make_shared<OpenGLViewer>();
        sdk->addViewer(viewer);
    }

    //Create a new scene to work in
    scene1 = sdk->createScene();

    //Create the camera controller
    if(!useVTKRenderer)
    {
        camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
        keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();
        pzrCamCtl = std::make_shared<mstk::Examples::Common::pzrMouseCameraController>();
    }
    auto cubeModel = std::make_shared<MeshModel>();
    TextureManager::addTexture("textures/cube.jpg", "cubetex");
    cubeModel->load("models/cube.obj");
    std::static_pointer_cast<SurfaceMesh>(cubeModel->getMesh())->assignTexture("cubetex");

    auto renderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    cubeModel->setRenderDetail(renderDetail);

    cube = std::make_shared<StaticSceneObject>();

    // If you want to use the GL renderer you need to specify the appropiate render delegates
    // This can be automated in the future, for now VTK is the default renderer and the delegates
    // need to be reset.
    cube->setModel(cubeModel);
    if(!useVTKRenderer)
    {
        auto renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "StaticSceneObjectRenderDelegate",RenderDelegate::RendererType::Other);
        cube->setRenderDelegate(renderDelegate);

        renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "MeshRenderDelegate",RenderDelegate::RendererType::Other);
        cubeModel->getMesh()->setRenderDelegate(renderDelegate);
    }

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
    if(!useVTKRenderer)
    {
        light = Light::getDefaultLighting();
        assert(light);
        scene1->addLight(light);

        // Camera setup
        sceneCamera = Camera::getDefaultCamera();
        assert(sceneCamera);
        sceneCamera->setPos(3, 3, 5);
        sceneCamera->setFocus(0, 0, -1);
        sceneCamera->genProjMat();
        sceneCamera->genViewMat();
        scene1->addCamera(sceneCamera);
        camCtl->setCamera(sceneCamera);
        pzrCamCtl->setCamera(sceneCamera);


        //Link up the event system between this the camera controller and the viewer
        viewer->attachEvent(core::EventType::Keyboard, camCtl);
        viewer->attachEvent(core::EventType::Keyboard, keyShutdown);
        viewer->attachEvent(core::EventType::MouseMove, pzrCamCtl);
        viewer->attachEvent(core::EventType::MouseButton, pzrCamCtl);
    }

    viewer->exec();
    //run the framework
//     sdk->run();

    //cleanup
//     sdk->releaseScene(scene1);

    return 0;
}
