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
#include "SimulationManager/SDK.h"

// Include required types scene objects
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/PlaneToMeshCollision.h"
#include "ContactHandling/PenaltyContactFemToStatic.h"
#include "Core/CollisionPair.h"
#include "Devices/VRPNDeviceServer.h"
#include "Devices/VRPNForceDevice.h"
#include "IO/InitIO.h"
#include "IO/IOMesh.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "SceneModels/StaticSceneObject.h"
#include "SceneModels/VegaFEMDeformableSceneObject.h"
#include "Simulators/DefaultSimulator.h"
#include "Simulators/ObjectSimulator.h"
#include "VirtualTools/ToolCoupler.h"
#include "VTKRendering/InitVTKRendering.h"

int main(int ac, char** av)
{
    std::string configFile = "./box.config";
    if(ac > 1)
    {
        configFile = av[1];
    }

    // initRenderDelegates();
    InitVTKRendering();
    InitIODelegates();

    //-------------------------------------------------------
    // 1. Create an instance of the iMSTK framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    auto sdk = SDK::createStandardSDK();
    auto client = std::make_shared<VRPNForceDevice>();
    auto server = std::make_shared<VRPNDeviceServer>();

    //get some user input and setup device url
    std::string input = "Phantom@localhost";
    client->setDeviceURL(input);

    auto controller = std::make_shared<ToolCoupler>(client);
    controller->setScalingFactor(20.0);
    sdk->registerModule(server);
//     sdk->registerModule(client);
    sdk->registerModule(controller);

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a FEM simulator
    auto femSimulator = std::make_shared<ObjectSimulator>();
//     femSimulator->setHapticTool(controller);

    // create a Vega based FEM object and attach it to the fem simulator
    auto femObject =
        std::make_shared<VegaFEMDeformableSceneObject>("./box.veg",configFile);
    femObject->setContactForcesOn();

    auto meshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES);
    meshRenderDetail->setAmbientColor(Color(0.2,0.2,0.2,1.0));
    meshRenderDetail->setDiffuseColor(Color::colorGray);
    meshRenderDetail->setSpecularColor(Color(1.0, 1.0, 1.0,0.5));
    meshRenderDetail->setShininess(10.0);

    // Load rendering mesh
    auto volumeMesh = std::static_pointer_cast<VegaVolumetricMesh>(femObject->getPhysicsModel()->getMesh());

    auto visualModel = std::make_shared<MeshModel>();
    visualModel->load("./box.vtk");
    femObject->setVisualModel(visualModel);

    auto visualMesh = visualModel->getMeshAs<SurfaceMesh>();

    if(visualMesh)
    {
        visualMesh->updateInitialVertices();
        visualMesh->setRenderDetail(meshRenderDetail);
        volumeMesh->attachSurfaceMesh(visualMesh,"./box.interp");
    }
    sdk->addSceneActor(femObject, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane
    //-------------------------------------------------------
    auto staticSimulator = std::make_shared<ObjectSimulator>();

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
    // Create scene actor 2:  loli tool
    // create a static object to hold the lolitool scene object of given normal and position
    //-------------------------------------------------------
    auto loliSceneObject = std::make_shared<StaticSceneObject>();

    auto loliCollisionModel = std::make_shared<MeshCollisionModel>();
    loliCollisionModel->loadTriangleMesh("./loli.vtk");
    loliSceneObject->setModel(loliCollisionModel);

    auto loliMesh = loliCollisionModel->getMesh();
    Core::BaseMesh::TransformType transform =
        Eigen::Translation3d(core::Vec3d(0,0,0))*Eigen::Scaling(0.1);

    auto loliRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_WIREFRAME);
    loliRenderDetail->setAmbientColor(Color(0.2, 0.2, 0.2, 0.5));
    loliRenderDetail->setDiffuseColor(Color::colorYellow);
    loliRenderDetail->setSpecularColor(Color(1.0, 1.0, 1.0, 0.5));
    loliRenderDetail->setShininess(20.0);

    loliMesh->setRenderDetail(loliRenderDetail);

    loliMesh->transform(transform);
    loliMesh->updateInitialVertices();

    auto loliSimulator = std::make_shared<ObjectSimulator>();
    sdk->addSceneActor(loliSceneObject, loliSimulator);
//     controller->setMesh(loliCollisionModel->getMesh());

    //-------------------------------------------------------
    // Enable collision between scene actors 1 and 2
    //-------------------------------------------------------
    auto sdkSimulator = sdk->getSimulator();
    auto meshModel = femObject->getCollisionModel();
    if(!meshModel)
    {
        std::cout << "There is no collision model attached to this scene object" << std::endl;
    }
    else
    {
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
    }
    //-------------------------------------------------------
    // Customize the viewer
    //-------------------------------------------------------
    auto viewer = sdk->getViewerInstance();

    viewer->setViewerRenderDetail(SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS);

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
