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

// Include required simulators
#include "Simulators/VegaFemSimulator.h"
#include "Simulators/DefaultSimulator.h"

#include "Core/CollisionPair.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneToMeshCollision.h"

#include "ContactHandling/PenaltyContactFemToStatic.h"

#include "IO/initIO.h"
#include "RenderDelegates/initRenderDelegates.h"
#include "VTKRendering/initVTKRendering.h"

int main(int ac, char** av)
{
    std::string configFile = "./box.config";
    if(ac > 1)
    {
        configFile = av[1];
    }

    initRenderDelegates();
    initVTKRendering();
    initIODelegates();
    const bool useVTKRenderer = true;

    //-------------------------------------------------------
    // 1. Create an instance of the SoFMIS framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    auto sdk = SDK::createStandardSDK();

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a FEM simulator
    auto femSimulator = std::make_shared<VegaFemSimulator>(sdk->getErrorLog());

    // create a Vega based FEM object and attach it to the fem simulator
    auto femObject = std::make_shared<VegaFemSceneObject>(
        sdk->getErrorLog(),configFile);

    auto meshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_WIREFRAME |
    //| SIMMEDTK_RENDER_VERTICES
    SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_NORMALS
    );
    meshRenderDetail->setAmbientColor(Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(Color::colorGray);
    meshRenderDetail->setSpecularColor(Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(20.0);

    auto renderingMesh = femObject->getVolumetricMesh()->getRenderingMesh();
    if(renderingMesh)
    {
        renderingMesh->setRenderDetail(meshRenderDetail);
    }
    sdk->addSceneActor(femObject, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane + dummy simulator
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
    // Register both object simulators
    //-------------------------------------------------------
    auto sdkSimulator = sdk->getSimulator();
    sdkSimulator->registerObjectSimulator(femSimulator);
    //sdkSimulator->registerObjectSimulator(staticSimulator);

    //-------------------------------------------------------
    // Enable collision between scene actors 1 and 2
    //-------------------------------------------------------
    auto meshModel = std::make_shared<MeshCollisionModel>();

    auto collisionMesh = femObject->getVolumetricMesh()->getCollisionMesh();
    if(collisionMesh)
    {
        meshModel->setMesh(collisionMesh);
    }

    auto planeMeshCollisionPairs = std::make_shared<CollisionPair>();
    planeMeshCollisionPairs->setModels(meshModel, plane);

    sdkSimulator->addCollisionPair(planeMeshCollisionPairs);

    auto planeToMeshCollisionDetection = std::make_shared<PlaneToMeshCollision>();

    sdkSimulator->registerCollisionDetection(planeToMeshCollisionDetection);

    //-------------------------------------------------------
    // Enable contact handling between scene actors 1 and 2
    //-------------------------------------------------------
    auto planeToMeshContact = std::make_shared<PenaltyContactFemToStatic>(false);
    planeToMeshContact->setCollisionPairs(planeMeshCollisionPairs);
    planeToMeshContact->setSceneObjects(staticObject, femObject);

    sdkSimulator->registerContactHandling(planeToMeshContact);

    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewerInstance();

    if(!useVTKRenderer)
    {
        viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                    SIMMEDTK_VIEWERRENDER_FADEBACKGROUND |
                                    SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS;
        viewer->setGlobalAxisLength(0.8);
    }

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
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}
