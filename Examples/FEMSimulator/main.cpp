// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include <memory>

// Core iMSTK includes
#include "SimulationManager/SDK.h"

// Include required types scene objects
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/PlaneToMeshCollision.h"
#include "ContactHandling/PenaltyContactFemToStatic.h"
#include "Core/CollisionManager.h"
#include "Devices/VRPNDeviceServer.h"
#include "Devices/VRPNForceDevice.h"
#include "IO/IOMesh.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "SceneModels/StaticSceneObject.h"
#include "SceneModels/VegaFEMDeformableSceneObject.h"
#include "Simulators/ObjectSimulator.h"
#include "VirtualTools/ToolCoupler.h"
#include "SimulationManager/Simulator.h"
#include "Rendering/ViewerBase.h"

#include "Testing/ReadPaths.h"

int main(int ac, char** av)
{
    //-------------------------------------------------------
    // 1. Create an instance of the iMSTK framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    auto sdk = imstk::SDK::createSDK();
    sdk->initialize();

    // Load paths for configuration files.
    std::string configPaths = "./Config.paths";
    if(ac > 1)
    {
        configPaths = av[1];
    }
    auto paths = imstk::ReadPaths(configPaths);
    if(std::get<imstk::Path::Binary>(paths).empty() &&
       std::get<imstk::Path::Source>(paths).empty())
    {
        std::cerr << "Error: Configuration file not found." << std::endl;
        std::cerr << std::endl;
        std::cerr << "\tUsage: " << av[0] << " /path_to/Config.paths" << std::endl;
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    std::string configFile = std::get<imstk::Path::Binary>(paths)+"/box.config";
    std::string meshFile = std::get<imstk::Path::Binary>(paths)+"/box.veg";
    std::string meshWeightsFile = std::get<imstk::Path::Binary>(paths)+"/box.interp";
    std::string meshSurfaceFile = std::get<imstk::Path::Binary>(paths)+"/box.vtk";
    std::string loliMeshFile = std::get<imstk::Path::Binary>(paths)+"/loli.vtk";

    if(configFile.empty())
    {
        std::cerr << "Vega configuration file not found." << std::endl;
        return EXIT_FAILURE;
    }

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a Vega based FEM object and attach it to the fem simulator
    auto femObject = sdk->createDeformableModel(meshFile,configFile);
    femObject->setContactForcesOn();

    auto meshRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_SURFACE);
    meshRenderDetail->setAmbientColor(imstk::Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(imstk::Color::colorGray);
    meshRenderDetail->setSpecularColor(imstk::Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(10.0);

    // Load rendering mesh
    auto volumeMesh = std::static_pointer_cast<imstk::VegaVolumetricMesh>(femObject->getPhysicsModel()->getMesh());

    auto visualModel = std::make_shared<imstk::MeshModel>();
    visualModel->load(meshSurfaceFile);
    femObject->setVisualModel(visualModel);

    auto visualMesh = visualModel->getMeshAs<imstk::SurfaceMesh>();

    if(visualMesh)
    {
        visualMesh->updateInitialVertices();
        visualMesh->setRenderDetail(meshRenderDetail);
        volumeMesh->attachSurfaceMesh(visualMesh);
    }

    //-------------------------------------------------------
    // Create scene actor 2:  plane
    //-------------------------------------------------------

    // create a static plane scene object of given normal and position
    auto staticObject = sdk->createStaticModel();

    auto plane = std::make_shared<imstk::PlaneCollisionModel>(
      imstk::Vec3d(0.0, -3.0, 0.0),
      imstk::Vec3d(0.0, 1.0, 0.0));
    plane->getPlaneModel()->setWidth(5);
    if (ac > 2)
    {
      plane->getPlaneModel()->setWidth(atof(av[2]));
    }
    staticObject->setModel(plane);

    //-------------------------------------------------------
    // Create scene actor 2:  loli tool
    // create a static object to hold the lolitool scene object of given normal and position
    //-------------------------------------------------------
    auto loliSceneObject = sdk->createStaticModel();

    auto loliCollisionModel = std::make_shared<imstk::MeshCollisionModel>();
    loliCollisionModel->loadTriangleMesh(loliMeshFile);
    loliSceneObject->setModel(loliCollisionModel);

    auto loliMesh = loliCollisionModel->getMesh();
    imstk::BaseMesh::TransformType transform =
        Eigen::Translation3d(imstk::Vec3d(0,0,0))*Eigen::Scaling(0.1);

    auto loliRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_SURFACE);
    loliRenderDetail->setAmbientColor(imstk::Color(0.2, 0.2, 0.2, 0.5));
    loliRenderDetail->setDiffuseColor(imstk::Color::colorGray);
    loliRenderDetail->setSpecularColor(imstk::Color(1.0, 1.0, 1.0, 0.5));
    loliRenderDetail->setShininess(20.0);

    loliMesh->setRenderDetail(loliRenderDetail);

    loliMesh->transform(transform);
    loliMesh->updateInitialVertices();

    // Set up a controller to control this mesh
    std::string deviceURL = "Phantom1@10.171.2.217";
    auto controller = sdk->createForceDeviceController(deviceURL);
    controller->setScalingFactor(30.0);
    controller->setMesh(loliCollisionModel->getMesh());

    //-------------------------------------------------------
    // Enable collision between scene actors 1 and 2
    //-------------------------------------------------------
    auto meshModel = std::make_shared<imstk::MeshCollisionModel>();

    auto collisionMesh = volumeMesh->getCollisionMesh();
    if(collisionMesh)
    {
        meshModel->setMesh(collisionMesh);
    }

    femObject->setCollisionModel(meshModel);

    auto planeMeshCollisionPairs = std::make_shared<imstk::CollisionManager>();
    auto planeToMeshCollisionDetection = std::make_shared<imstk::PlaneToMeshCollision>();

    planeMeshCollisionPairs->setModels(meshModel, plane);

    //-------------------------------------------------------
    // Enable contact handling between scene actors 1 and 2
    //-------------------------------------------------------
    auto planeToMeshContact = std::make_shared<imstk::PenaltyContactFemToStatic>(false);

    planeToMeshContact->setCollisionPairs(planeMeshCollisionPairs);

    planeToMeshContact->setInteractionSceneModels(staticObject, femObject);

    sdk->addInteraction(planeMeshCollisionPairs,
                        planeToMeshCollisionDetection,
                        planeToMeshContact);

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewer();

    viewer->setViewerRenderDetail(IMSTK_VIEWERRENDER_GLOBAL_AXIS);

    // Get Scene
    auto scene = sdk->getScene();

    // Setup Scene lighting
    auto light1 = imstk::Light::getDefaultLighting();
    light1->lightPos.setPosition(imstk::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = imstk::Light::getDefaultLighting();
    light2->lightPos.setPosition(imstk::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    return 0;
}
