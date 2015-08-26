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

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"
#include "../common/pzrMouseCameraController.h"

int main()
{
    std::shared_ptr<SDK> sdk;
    std::shared_ptr<VegaFemSceneObject> femObject;
    std::shared_ptr<StaticSceneObject> staticObject;
    std::shared_ptr<VegaFemSimulator> femSimulator;
    std::shared_ptr<DefaultSimulator> staticSimulator;
    std::shared_ptr<PlaneCollisionModel> plane;
    std::shared_ptr<ViewerBase> viewer;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Light> light;
    std::shared_ptr<Camera> sceneCamera;
    std::shared_ptr<mstk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<mstk::Examples::Common::KeyPressSDKShutdown> keyShutdown;
    std::shared_ptr<mstk::Examples::Common::pzrMouseCameraController> pzrCamCtl;
    //std::shared_ptr<mstk::Examples::Common::hapticController> hapticCtl;
    Matrix33d mat;

    //-------------------------------------------------------
    // 1. Create an instance of the SoFMIS framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    sdk = SDK::createStandardSDK();

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a FEM simulator
    femSimulator = std::make_shared<VegaFemSimulator>(sdk->getErrorLog());

    // create a Vega based FEM object and attach it to the fem simulator
    femObject = std::make_shared<VegaFemSceneObject>(
        sdk->getErrorLog(),
        "asianDragon/asianDragon.config");

    auto femObjRenderDetail = std::make_shared<RenderDetail>(//SIMMEDTK_RENDER_WIREFRAME
                                                             //| SIMMEDTK_RENDER_VERTICES
                                                             SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_NORMALS
                                                              );
    femObjRenderDetail->setPointSize(4.0);
    Color maroon(165.0f / 255, 42.0f / 255, 42.0f / 255, 1.0);
    femObjRenderDetail->setVertexColor(maroon);
    femObjRenderDetail->setNormalLength(0.02);

    femObject->getVolumetricMesh()->getRenderingMesh()->setRenderDetail(femObjRenderDetail);

    sdk->addSceneActor(femObject, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane + dummy simulator
    //-------------------------------------------------------
    // Create dummy simulator
    staticSimulator = std::make_shared<DefaultSimulator>(sdk->getErrorLog());

    // create a static plane scene object of given normal and position
    staticObject = std::make_shared<StaticSceneObject>();

    plane = std::make_shared<PlaneCollisionModel>(core::Vec3d(0.0, -3.0, 0.0),
                                                  core::Vec3d(0.0, 1.0, 0.0));

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

    meshModel->setMesh(femObject->getVolumetricMesh()->getAttachedMesh(0));

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
    viewer = sdk->getViewerInstance();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail |
                                SIMMEDTK_VIEWERRENDER_FADEBACKGROUND |
                                SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS;

    viewer->setGlobalAxisLength(0.8);

    // Get Scene
    scene = sdk->getScene(0);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "Collision pipeline demo");

    // Setup Scene lighting
    light = Light::getDefaultLighting();
    scene->addLight(light);

    // Camera setup
    sceneCamera = Camera::getDefaultCamera();
    sceneCamera->setPos(12, 12, 24);
    sceneCamera->setFocus(0, 0, 0);
    scene->addCamera(sceneCamera);

    // Create the camera controller
    camCtl = std::make_shared<mstk::Examples::Common::wasdCameraController>();
    camCtl->setCamera(sceneCamera);

    keyShutdown = std::make_shared<mstk::Examples::Common::KeyPressSDKShutdown>();

    pzrCamCtl = std::make_shared<mstk::Examples::Common::pzrMouseCameraController>();
    pzrCamCtl->setCamera(sceneCamera);

    // Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(core::EventType::Keyboard, camCtl);
    viewer->attachEvent(core::EventType::Keyboard, keyShutdown);
    viewer->attachEvent(core::EventType::MouseMove, pzrCamCtl);
    viewer->attachEvent(core::EventType::MouseButton, pzrCamCtl);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}
