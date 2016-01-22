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
#include "VTKRendering/InitVTKRendering.h"
#include "IO/IOMesh.h"

int main(int ac, char **av)
{
    std::string configFile = "./nidus.config";
    if(ac == 2)
    {
        configFile = av[1];
    }

    InitVTKRendering();
    InitIODelegates();

    //-------------------------------------------------------
    // 1. Create an instance of the iMSTK framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    auto sdk = SDK::createStandardSDK();
    auto sdkSimulator = sdk->getSimulator();

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------
    // create a FEM simulator
    auto femSimulator = std::make_shared<ObjectSimulator>();

    // create a Vega based FEM object and attach it to the fem simulator
    auto femModel = std::make_shared<VegaFEMDeformableSceneObject>("nidusV1764.vtk",
                                                                   configFile);

    sdk->addSceneActor(femModel, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane + dummy simulator
    //-------------------------------------------------------
    // Create dummy simulator
    auto staticSimulator = std::make_shared<ObjectSimulator>();

    // Create a static plane scene object of given normal and position
    auto staticObject = std::make_shared<StaticSceneObject>();

    auto plane = std::make_shared<PlaneCollisionModel>(core::Vec3d(0.0,0.0,-35.0),
                                                       core::Vec3d(0.0,0.0,1.0));

    staticObject->setModel(plane);
    sdk->addSceneActor(staticObject, staticSimulator);

    //-------------------------------------------------------
    // Enable collision between scene actors 1 and 2
    //-------------------------------------------------------
    auto meshModel = femModel->getCollisionModel();
    if(!meshModel)
    {
        std::cout << "There is no collision model attached to this scene object" << std::endl;
    }
    else
    {
        auto planeMeshCollisionPairs = std::make_shared<CollisionManager>();
        planeMeshCollisionPairs->setModels(meshModel, plane);

        sdkSimulator->addCollisionPair(planeMeshCollisionPairs);

        auto planeToMeshCollisionDetection = std::make_shared<PlaneToMeshCollision>();

        sdkSimulator->registerCollisionDetection(planeToMeshCollisionDetection);

        //-------------------------------------------------------
        // Enable contact handling between scene actors 1 and 2
        //-------------------------------------------------------
        auto planeToMeshContact = std::make_shared<PenaltyContactFemToStatic>(false);
        planeToMeshContact->setCollisionPairs(planeMeshCollisionPairs);
        planeToMeshContact->setSceneObjects(staticObject, femModel);

        sdkSimulator->registerContactHandling(planeToMeshContact);
    }

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewerInstance();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                IMSTK_VIEWERRENDER_FADEBACKGROUND |
                                IMSTK_VIEWERRENDER_GLOBAL_AXIS;

    viewer->setGlobalAxisLength(0.8);

    //-------------------------------------------------------
    // Customize the scene
    //-------------------------------------------------------
    // Get Scene
    auto scene = sdk->getScene(0);
    viewer->registerScene(scene);

    // Setup Scene lighting
    auto light1 = Light::getDefaultLighting();
    light1->lightPos.setPosition(core::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = Light::getDefaultLighting();
    light2->lightPos.setPosition(core::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    // Camera setup
    auto sceneCamera = Camera::getDefaultCamera();
    sceneCamera->setPos(-60,0,0);
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
