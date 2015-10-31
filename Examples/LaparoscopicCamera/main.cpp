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

// Core SimMedTK includes
#include "Core/SDK.h"

// Include required types scene objects
#include "Simulators/VegaFemSceneObject.h"
#include "Core/StaticSceneObject.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "Devices/VRPNForceDevice.h"

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

#include "VTKRendering/initVTKRendering.h"
#include "VTKRendering/VTKViewer.h"

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
      core::Vec3d(0.0, -3.0, 0.0),
      core::Vec3d(0.0, 1.0, 0.0));
    plane->getPlaneModel()->setWidth(5);
    if (ac > 2)
      plane->getPlaneModel()->setWidth(atof(av[2]));
    staticObject->setModel(plane);

    sdk->addSceneActor(staticObject, staticSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  dragon
    //-------------------------------------------------------
    auto staticSimulator2 = std::make_shared<DefaultSimulator>(sdk->getErrorLog());

    // create a static plane scene object of given normal and position
    auto staticDragon = std::make_shared<StaticSceneObject>();

    auto dragenModel = std::make_shared<MeshCollisionModel>();
    dragenModel->loadTriangleMesh("./asianDragon.vtk");
    dragenModel->getMesh()->scale(Eigen::UniformScaling<double>(0.2));
    staticDragon->setModel(dragenModel);

    auto dragonRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES);

    dragonRenderDetail->setAmbientColor(Color(0.2, 0.2, 0.2, 1.0));
    dragonRenderDetail->setDiffuseColor(Color(0.0, 0.8, 0.0, 1.0));
    dragonRenderDetail->setSpecularColor(Color(0.4, 0.4, 0.4, 1.0));
    dragonRenderDetail->setShininess(100.0);

    dragenModel->setRenderDetail(dragonRenderDetail);

    sdk->addSceneActor(staticDragon, staticSimulator2);

    //-------------------------------------------------------
    // Customize the viewer
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

    //-------------------------------------------------------
    // Create a Laparoscopic camera controller
    //-------------------------------------------------------
    auto camClient = std::make_shared<VRPNForceDevice>();

    //get some user input and setup device url
    std::string input = "Phantom@10.171.2.217";//"Phantom0@localhost";
    std::cout << "Enter the VRPN device URL(" << camClient->getDeviceURL() << "): ";
    std::getline(std::cin, input);

    if (!input.empty())
    {
        camClient->setDeviceURL(input);
    }
    auto camController = std::make_shared<LaparoscopicCameraCoupler>(camClient);
    camController->setScalingFactor(40.0);

    viewer->init(); // viewer should be initialized to be able to retrieve the camera
    camController->setCamera(
        (std::static_pointer_cast<VTKViewer>(viewer))->getVtkCamera());

    sdk->registerModule(camClient);
    sdk->registerModule(camController);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}
