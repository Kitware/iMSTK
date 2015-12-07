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
#include "Geometry/MeshModel.h"

#include "IO/InitIO.h"
#include "VTKRendering/InitVTKRendering.h"
#include "IO/IOMesh.h"

int main()
{
    InitVTKRendering();
    InitIODelegates();

    //-------------------------------------------------------
    // 1. Create an instance of the SimMedTK framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    auto sdk = SDK::createStandardSDK();

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewerInstance();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                SIMMEDTK_VIEWERRENDER_FADEBACKGROUND |
                                SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS;

    viewer->setGlobalAxisLength(0.8);

    //-------------------------------------------------------
    // Setup Shaders and textures
    //-------------------------------------------------------
    Shaders::createShader("wetshader", "ShadersData/shaders/wet_vert.glsl", "ShadersData/shaders/wet_frag.glsl", "");

    // Mesh render detail
    auto meshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_NORMALS );
    meshRenderDetail->setAmbientColor(Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(Color::colorGray);
    meshRenderDetail->setSpecularColor(Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(20.0);

    meshRenderDetail->addShaderProgram("wetshader");
    meshRenderDetail->addTexture("decal", "ShadersData/textures/brainx.bmp", "textureDecal", "wetshader");
    meshRenderDetail->addTexture("bump", "ShadersData/textures/metalbump.jpg", "textureBump", "wetshader");

    // Plane render detail
    auto planeMeshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_NORMALS);
    planeMeshRenderDetail->setAmbientColor(Color(0.2, 0.2, 0.2, 1.0));
    planeMeshRenderDetail->setDiffuseColor(Color::colorGray);
    planeMeshRenderDetail->setSpecularColor(Color(1.0, 1.0, 1.0, 0.5));
    planeMeshRenderDetail->setShininess(20.0);

    planeMeshRenderDetail->addShaderProgram("wetshader");
    planeMeshRenderDetail->addTexture("decal", "ShadersData/textures/brain_outside.jpg", "textureDecal", "wetshader");
    planeMeshRenderDetail->addTexture("bump", "ShadersData/textures/metalbump.jpg", "textureBump", "wetshader");

    //-------------------------------------------------------
    // Customize the scene
    //-------------------------------------------------------
    // Get Scene
    auto scene = sdk->getScene(0);
    viewer->registerScene(scene);

    // Setup Cube
    auto cubeModel = std::make_shared<MeshModel>();
    cubeModel->load("ShadersData/models/brain.obj");
    //cubeModel->load("ShadersData/models/brain.3ds");
    cubeModel->setRenderDetail(meshRenderDetail);

    auto cube = std::make_shared<StaticSceneObject>();
    cube->setModel(cubeModel);
    scene->addSceneObject(cube);

    // Setup Plane
    auto planeModel = std::make_shared<MeshModel>();
    planeModel->load("ShadersData/models/plane.obj");
    planeModel->setRenderDetail(planeMeshRenderDetail);

    auto planeObject = std::make_shared<StaticSceneObject>();
    planeObject->setModel(planeModel);
    scene->addSceneObject(planeObject);

    // Setup Scene lighting
    auto light1 = Light::getDefaultLighting();
    light1->lightPos.setPosition(core::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = Light::getDefaultLighting();
    light2->lightPos.setPosition(core::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    // Camera setup
    auto sceneCamera = Camera::getDefaultCamera();
    sceneCamera->setPos(-200,0,0);
    sceneCamera->setZoom(.5);
    scene->addCamera(sceneCamera);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}
