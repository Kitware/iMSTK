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
#include "Geometry/MeshModel.h"
#include "SceneModels/StaticSceneObject.h"

#include "IO/InitIO.h"
#include "Rendering/InitVTKRendering.h"
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

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewer();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                IMSTK_VIEWERRENDER_FADEBACKGROUND |
                                IMSTK_VIEWERRENDER_GLOBAL_AXIS;

    viewer->setGlobalAxisLength(0.8);

    //-------------------------------------------------------
    // Setup Shaders and textures
    //-------------------------------------------------------
    imstk::Shaders::createShader("wetshader",
                                 dataPath+"/shaders/wet_vert.glsl",
                                 dataPath+"/shaders/wet_frag.glsl", "");

    // Mesh render detail
    auto meshRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_FACES | IMSTK_RENDER_NORMALS );
    meshRenderDetail->setAmbientColor(imstk::Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(imstk::Color::colorGray);
    meshRenderDetail->setSpecularColor(imstk::Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(20.0);

    meshRenderDetail->addShaderProgram("wetshader");
    meshRenderDetail->addTexture("decal", dataPath+"/textures/brainx.bmp", "textureDecal", "wetshader");
    meshRenderDetail->addTexture("bump", dataPath+"/textures/metalbump.jpg", "textureBump", "wetshader");

    // Plane render detail
    auto planeMeshRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_FACES | IMSTK_RENDER_NORMALS);
    planeMeshRenderDetail->setAmbientColor(imstk::Color(0.2, 0.2, 0.2, 1.0));
    planeMeshRenderDetail->setDiffuseColor(imstk::Color::colorGray);
    planeMeshRenderDetail->setSpecularColor(imstk::Color(1.0, 1.0, 1.0, 0.5));
    planeMeshRenderDetail->setShininess(20.0);

    planeMeshRenderDetail->addShaderProgram("wetshader");
    planeMeshRenderDetail->addTexture("decal", dataPath+"/textures/brain_outside.jpg", "textureDecal", "wetshader");
    planeMeshRenderDetail->addTexture("bump", dataPath+"/textures/metalbump.jpg", "textureBump", "wetshader");

    //-------------------------------------------------------
    // Customize the scene
    //-------------------------------------------------------
    // Get Scene
    auto scene = sdk->getScene();
    viewer->registerScene(scene);

    // Setup Cube
    auto cubeModel = std::make_shared<imstk::MeshModel>();
    cubeModel->load(dataPath+"/models/brain.obj");
    //cubeModel->load("ShadersData/models/brain.3ds");
    cubeModel->setRenderDetail(meshRenderDetail);

    auto cube = std::make_shared<imstk::StaticSceneObject>();
    cube->setModel(cubeModel);
    scene->addSceneObject(cube);

    // Setup Plane
    auto planeModel = std::make_shared<imstk::MeshModel>();
    planeModel->load(dataPath+"/models/plane.obj");
    planeModel->setRenderDetail(planeMeshRenderDetail);

    auto planeObject = std::make_shared<imstk::StaticSceneObject>();
    planeObject->setModel(planeModel);
//     scene->addSceneObject(planeObject);

    // Setup Scene lighting
    auto light1 = imstk::Light::getDefaultLighting();
    light1->lightPos.setPosition(imstk::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = imstk::Light::getDefaultLighting();
    light2->lightPos.setPosition(imstk::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    // Camera setup
    auto sceneCamera = imstk::Camera::getDefaultCamera();
    sceneCamera->setPos(-200,0,0);
    sceneCamera->setZoom(.5);
    scene->addCamera(sceneCamera);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    return 0;
}
