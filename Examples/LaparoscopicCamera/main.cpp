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

#include <memory>
#include <string>
#include <cmath>

// Core SimMedTK includes
#include "Core/SDK.h"

// Include required types scene objects
#include "Simulators/VegaFemSceneObject.h"
#include "Core/StaticSceneObject.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "Devices/VRPNForceDevice.h"
#include "Devices/VRPNDeviceServer.h"
#include "VirtualTools/ToolCoupler.h"
#include "VirtualTools/LaparoscopicCameraCoupler.h"

// Include required simulators
#include "Simulators/VegaFemSimulator.h"
#include "Simulators/DefaultSimulator.h"
#include "Core/CollisionPair.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneToMeshCollision.h"
#include "ContactHandling/PenaltyContactFemToStatic.h"
#include "IO/initIO.h"

// VTK includes
#include "VTKRendering/initVTKRendering.h"
#include "VTKRendering/VTKViewer.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"

#define SPACE_EXPLORER_DEVICE true

bool createCameraNavigationScene(std::shared_ptr<SDK> sdk, char* fileName)
{
    auto meshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_NORMALS);

    meshRenderDetail->setAmbientColor(Color(0.2, 0.2, 0.2, 1.0));
    meshRenderDetail->setDiffuseColor(Color(0.8, 0.0, 0.0, 1.0));
    meshRenderDetail->setSpecularColor(Color(0.4, 0.4, 0.4, 1.0));
    meshRenderDetail->setShininess(100.0);

    double radius = 3.0;
    for (int i = 0; i < 6; i++)
    {
        auto staticSimulator2 = std::make_shared<DefaultSimulator>(sdk->getErrorLog());

        auto staticTarget = std::make_shared<StaticSceneObject>();

        auto targetModel = std::make_shared<MeshCollisionModel>();
        targetModel->loadTriangleMesh(fileName);
        targetModel->getMesh()->scale(Eigen::UniformScaling<double>(0.15));//0.2
        staticTarget->setModel(targetModel);

        targetModel->setRenderDetail(meshRenderDetail);

        sdk->addSceneActor(staticTarget, staticSimulator2);

        targetModel->getMesh()->translate(Eigen::Translation3d(0, 0, -radius));

        Eigen::Quaterniond q(cos(i*22.0/42), 0, sin(i*22.0/42), 0);
        q.normalize();
        targetModel->getMesh()->rotate(q);
    }
    return true;
}

int main(int ac, char** av)
{
    initVTKRendering();
    initIODelegates();
    const bool useVTKRenderer = true;

    auto sdk = SDK::createStandardSDK();

    //-------------------------------------------------------
    // Create scene actor 1:  plane
    //-------------------------------------------------------
    auto staticSimulator = std::make_shared<DefaultSimulator>(sdk->getErrorLog());

    // create a static plane scene object of given normal and position
    auto staticObject = std::make_shared<StaticSceneObject>();

    auto plane = std::make_shared<PlaneCollisionModel>(
      core::Vec3d(0.0, -0.01, 0.0),
      core::Vec3d(0.0, 1.0, 0.0));
    plane->getPlaneModel()->setWidth(5);
    if (ac > 2)
      plane->getPlaneModel()->setWidth(atof(av[2]));
    staticObject->setModel(plane);

    auto planeRendDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES);

    Color grey(0.32, 0.32, 0.32, 1.0);

    planeRendDetail->setAmbientColor(grey);
    planeRendDetail->setDiffuseColor(grey);
    planeRendDetail->setSpecularColor(grey);
    planeRendDetail->setShininess(50.0);

    plane->getPlaneModel()->setRenderDetail(planeRendDetail);

    sdk->addSceneActor(staticObject, staticSimulator);

    //-------------------------------------------------------
    // Create camera navigation scene
    //-------------------------------------------------------

    createCameraNavigationScene(sdk, "./Target.vtk");

    //-------------------------------------------------------
    // Set up the viewer
    //-------------------------------------------------------

    auto viewer = sdk->getViewerInstance();

    viewer->setViewerRenderDetail(
        SIMMEDTK_VIEWERRENDER_GLOBALAXIS
        | SIMMEDTK_VIEWERRENDER_FADEBACKGROUND
        | SIMMEDTK_DISABLE_MOUSE_INTERACTION
        );

    // Get Scene
    auto scene = sdk->getScene(0);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "Collision pipeline demo");

    // Setup Scene lighting
    auto light1 = Light::getDefaultLighting();
    light1->lightPos.setPosition(core::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = Light::getDefaultLighting();
    light2->lightPos.setPosition(core::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    auto light3 = Light::getDefaultLighting();
    light3->lightPos.setPosition(core::Vec3d(0.0, 25.0, 0.0));
    scene->addLight(light3);

    //-------------------------------------------------------
    // Create a Laparoscopic camera controller and connect
    // it to the vtk viewer
    //-------------------------------------------------------
    auto camClient = std::make_shared<VRPNForceDevice>();
    std::shared_ptr<VRPNDeviceServer> server;
    std::string input;

    if (SPACE_EXPLORER_DEVICE)
    {
        server = std::make_shared<VRPNDeviceServer>();

        //get some user input and setup device url
        input = "navigator@localhost";
    }
    else
    {
        //get some user input and setup device url
        std::string input = "Phantom0@localhost";//"Phantom@10.171.2.217"
        std::cout << "Enter the VRPN device URL(" << camClient->getDeviceURL() << "): ";
        std::getline(std::cin, input);
    }

    if (!input.empty())
    {
        camClient->setDeviceURL(input);
    }
    auto camController = std::make_shared<LaparoscopicCameraCoupler>(camClient);
    camController->setScalingFactor(40.0);

    viewer->init(); // viewer should be initialized to be able to retrieve the camera

    std::shared_ptr<VTKViewer> vtkViewer = std::static_pointer_cast<VTKViewer>(viewer);

    vtkCamera* cam = vtkViewer->getVtkCamera();

    // set the view angle of the camera. 80 deg for laparoscopic camera
    cam->SetViewAngle(80.0);

    // set the camera to be controlled by the camera controller
    camController->setCamera(cam);

    // Connect the camera controller to the vtk viewer to enable camera manipulation
    vtkViewer->setCameraControllerData(camController->getCameraData());

    sdk->registerModule(camClient);
    sdk->registerModule(camController);

    if (SPACE_EXPLORER_DEVICE)
    {
        sdk->registerModule(server);
    }

    // Enable screenshot
    camController->enableScreenCapture();
    vtkViewer->setScreenCaptureData(camController->getScreenCaptureData());

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}
