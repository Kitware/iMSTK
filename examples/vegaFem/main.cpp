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

// Core SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSDK.h"
#include "smRendering/smViewer.h"
#include "smCore/smSceneObject.h"

// Include required types scene objects
#include "smSimulators/smVegaFemSceneObject.h"
#include "smCore/smStaticSceneObject.h"

// Include required simulators
#include "smSimulators/smVegaFemSimulator.h"
#include "smSimulators/smDummySimulator.h"

#include "smCollision/smCollisionPair.h"
#include "smCollision/smPlaneCollisionModel.h"
#include "smCollision/smMeshCollisionModel.h"
#include "smCollision/smPlaneToMeshCollision.h"

#include "smContactHandling/smPenaltyContactFemToStatic.h"

#include "../common/wasdCameraController.h"
#include "../common/KeyPressSDKShutdown.h"
//#include "../common/hapticController.h"

int main()
{
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smVegaFemSceneObject> femObject;
    std::shared_ptr<smStaticSceneObject> staticObject;
    std::shared_ptr<smVegaFemSimulator> femSimulator;
    std::shared_ptr<smDummySimulator> staticSimulator;
    std::shared_ptr<smPlaneCollisionModel> plane;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smScene> scene;
    std::shared_ptr<smLight> light;
    std::shared_ptr<smCamera> sceneCamera;
    std::shared_ptr<smtk::Examples::Common::wasdCameraController> camCtl;
    std::shared_ptr<smtk::Examples::Common::KeyPressSDKShutdown> keyShutdown;
    //std::shared_ptr<smtk::Examples::Common::hapticController> hapticCtl;
    smMatrix33d mat;

    //-------------------------------------------------------
    // 1. Create an instance of the SoFMIS framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    sdk = smSDK::createStandardSDK();

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a FEM simulator
    femSimulator = std::make_shared<smVegaFemSimulator>(sdk->getErrorLog());

    // create a Vega based FEM object and attach it to the fem simulator
    femObject = std::make_shared<smVegaFemSceneObject>(sdk->getErrorLog(),
                                                        "asianDragon/asianDragon.config");

    auto femObjRenderDetail = std::make_shared<smRenderDetail>(
                                                               SIMMEDTK_RENDER_WIREFRAME
                                                               | SIMMEDTK_RENDER_VERTICES
                                                               //| SIMMEDTK_RENDER_FACES
                                                              );
    femObjRenderDetail->setPointSize(4.0);
    smColor maroon(165.0f / 255, 42.0f / 255, 42.0f / 255, 1.0);
    femObjRenderDetail->setVertexColor(maroon);
    femObjRenderDetail->setNormalLength(0.02);

    femObject->setRenderDetail(femObjRenderDetail);

    /*hapticCtl = std::make_shared<smtk::Examples::Common::hapticController>();
    hapticCtl->setVegaFemSceneObject(femObject);
    femSimulator->attachEvent(smtk::Event::EventType::Haptic, hapticCtl);*/

    sdk->addSceneActor(femObject, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane + dummy simulator
    //-------------------------------------------------------
    // Create dummy simulator
    staticSimulator = std::make_shared<smDummySimulator>(sdk->getErrorLog());

    // create a static plane scene object of given normal and position
    staticObject = std::make_shared<smStaticSceneObject>();

    plane = std::make_shared<smPlaneCollisionModel>(smVec3d(0.0, -3.0, 0.0),
                                                    smVec3d(0.0, 1.0, 0.0));

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
    auto meshModel = std::make_shared<smMeshCollisionModel>();
    
    meshModel->setMesh(femObject->getPrimarySurfaceMesh());

    auto planeMeshCollisionPairs = std::make_shared<smCollisionPair>();

    planeMeshCollisionPairs->setModels(meshModel, plane);

    sdkSimulator->addCollisionPair(planeMeshCollisionPairs);

    auto planeToMeshCollisionDetection = std::make_shared<smPlaneToMeshCollision>();

    sdkSimulator->registerCollisionDetection(planeToMeshCollisionDetection);

    //-------------------------------------------------------
    // Enable contact handling between scene actors 1 and 2
    //-------------------------------------------------------
    auto planeToMeshContact = std::make_shared<smPenaltyContactFemToStatic>(false);

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
    light = smLight::getDefaultLighting();
    scene->addLight(light);

    // Camera setup
    sceneCamera = smCamera::getDefaultCamera();
    sceneCamera->setCameraPos(3, 3, 5);
    sceneCamera->setCameraFocus(0, 0, 0);
    scene->addCamera(sceneCamera);

    // Create the camera controller
    camCtl = std::make_shared<smtk::Examples::Common::wasdCameraController>();
    camCtl->setCamera(sceneCamera);

    keyShutdown = std::make_shared<smtk::Examples::Common::KeyPressSDKShutdown>();

    // Link up the event system between this the camera controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, camCtl);
    viewer->attachEvent(smtk::Event::EventType::Keyboard, keyShutdown);

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    //cleanup
    sdk->releaseScene(scene);

    return 0;
}