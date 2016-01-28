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
#include "Mesh/VegaVolumetricMesh.h"
#include "SceneModels/StaticSceneObject.h"
#include "SceneModels/VegaFEMDeformableSceneObject.h"
#include "Simulators/ObjectSimulator.h"

#include "Core/CollisionManager.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneToMeshCollision.h"

#include "ContactHandling/PenaltyContactFemToStatic.h"

#include "IO/InitIO.h"
#include "Rendering/InitVTKRendering.h"
#include "Rendering/ViewerBase.h"
#include "IO/IOMesh.h"

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

    auto dataPath = std::get<imstk::Path::Binary>(paths);
    std::string configFile = dataPath+"/nidus.config";
    std::string meshFile = dataPath+"/nidusV1764.vtk";
    std::string meshWeightsFile = dataPath+"/nidusV1764.interp";
    std::string meshSurfaceFile = dataPath+"/nidusS7415.vtk";

    if(configFile.empty())
    {
        std::cerr << "Configuration file not found." << std::endl;
        return EXIT_FAILURE;
    }


    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------
    // create a FEM simulator
    auto femSimulator = std::make_shared<imstk::ObjectSimulator>();

    // create a Vega based FEM object and attach it to the fem simulator
    auto femModel = std::make_shared<imstk::VegaFEMDeformableSceneObject>(meshFile,configFile);

    // Load rendering mesh
    auto volumeMesh = std::static_pointer_cast<imstk::VegaVolumetricMesh>(femModel->getPhysicsModel()->getMesh());

    auto visualModel = std::make_shared<imstk::MeshModel>();
    visualModel->load(meshSurfaceFile);

    auto visualMesh = visualModel->getMeshAs<imstk::SurfaceMesh>();

    //-------------------------------------------------------
    // Mesh render detail
    // Setup Shaders and textures
    //-------------------------------------------------------
    imstk::Shaders::createShader("wetshader", dataPath+"/shaders/wet_vert.glsl", dataPath+"/shaders/wet_frag.glsl", "");

    auto meshRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_FACES | IMSTK_RENDER_NORMALS );
    meshRenderDetail->setAmbientColor(imstk::Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(imstk::Color::colorRed);
    meshRenderDetail->setSpecularColor(imstk::Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(100);

//     meshRenderDetail->addShaderProgram("wetshader");
//     meshRenderDetail->addTexture("decal",
//                                  dataPath+"/textures/brainx.bmp",
//                                  "textureDecal",
//                                  "wetshader");
//
//     meshRenderDetail->addTexture("bump",
//                                  dataPath+"/textures/metalbump.jpg",
//                                  "textureBump",
//                                  "wetshader");

    if(visualMesh)
    {
        visualMesh->updateInitialVertices();
        visualMesh->setRenderDetail(meshRenderDetail);
        volumeMesh->attachSurfaceMesh(visualMesh/*,meshWeightsFile*/);
    }
    femModel->setVisualModel(visualModel);
    sdk->addSceneActor(femModel, femSimulator);

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewer();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                IMSTK_VIEWERRENDER_FADEBACKGROUND |
                                IMSTK_VIEWERRENDER_GLOBAL_AXIS;

    viewer->setGlobalAxisLength(0.8);

    //-------------------------------------------------------
    // Customize the scene
    //-------------------------------------------------------
    // Get Scene
    auto scene = sdk->getScene();

    // Setup Scene lighting
    auto light1 = imstk::Light::getDefaultLighting();
    light1->lightPos.setPosition(imstk::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = imstk::Light::getDefaultLighting();
    light2->lightPos.setPosition(imstk::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    // Camera setup
    auto sceneCamera = imstk::Camera::getDefaultCamera();
    sceneCamera->setPos(60,0,0);
    sceneCamera->setZoom(.5);
    scene->addCamera(sceneCamera);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    return 0;
}
