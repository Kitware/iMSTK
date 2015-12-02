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

#include <string>
#include <chrono>
#include <thread>

#include "Examples/common/ExampleCube.h"

#include "Core/SDK.h"
#include "Core/Factory.h"
#include "Core/ViewerBase.h"
#include "Devices/VRPNForceDevice.h"
#include "Devices/VRPNDeviceServer.h"
#include "IO/initIO.h"
#include "RenderDelegates/initRenderDelegates.h"
#include "VTKRendering/initVTKRendering.h"
#include "VirtualTools/ToolCoupler.h"
#include "Geometry/PlaneModel.h"

int main()
{
    using ExampleCube = mstk::Examples::Common::ExampleCube;

    initRenderDelegates();
    initVTKRendering();
    initIODelegates();
    const bool useVTKRenderer = true; // VTK is the default viewer.

    ExampleCube cube;
    auto plane = std::make_shared<PlaneModel>(core::Vec3d(0.0, -5.0, 0.0),
                                              core::Vec3d(0.0, 1.0, 0.0));
    plane->getPlaneModel()->setWidth(5);
    auto staticPlane = std::make_shared<StaticSceneObject>();
    staticPlane->setModel(plane);
    std::string input = "navigator@localhost";

    auto sdk = SDK::getInstance();
//     auto server = std::make_shared<VRPNDeviceServer>();
    auto client = std::make_shared<VRPNForceDevice>();
    core::Vec4f contactPlane;
    contactPlane << 0.0, 1.0, 0.0, -5.0;
    client->setContactPlane(contactPlane);
    client->setDampingCoefficient(.0001);
    client->setSpringCoefficient(.01);
    auto controller = std::make_shared<ToolCoupler>(client);
    controller->setScalingFactor(5.0);

//     sdk->registerModule(server);
    sdk->registerModule(client);
    sdk->registerModule(controller);

    //get some user input and setup device url
    std::cout << "Enter the VRPN device URL(" << client->getDeviceURL() << "): ";
    std::getline(std::cin, input);
    if(!input.empty())
    {
        client->setDeviceURL(input);
    }

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

    //Set up the cube object
    cube.useVTKRenderer(useVTKRenderer);
    cube.setup();
    controller->setMesh(cube.getStaticSceneObject()->getModel()->getMesh());

    // Add the cube to the scene to be rendered
    scene->addSceneObject(cube.getStaticSceneObject());
    scene->addSceneObject(staticPlane);

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
    }

    sdk->addViewer(viewer);
    sdk->run();
    return 0;
}
