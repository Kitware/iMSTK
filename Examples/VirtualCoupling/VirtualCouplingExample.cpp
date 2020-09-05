/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkSimulationManager.h"
    << << << < HEAD
#include "imstkCollidingObject.h"
#include "imstkObjectInteractionFactory.h"
== == == =
    >> >> >> > 486fd64 4 ... temp commit: all examples working, bug fixes
#include "imstkVirtualCouplingCH.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkHDAPIDeviceClient.h"
#include "imstkSceneObjectController.h"
    << << << < HEAD
#include "imstkCDObjectFactory.h"
#include "imstkDeviceTracker.h"
#include "imstkCollisionGraph.h"
#include "imstkCollisionPair.h"
#include "imstkCollisionData.h"
#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
== == == =
#include "imstkDeviceTracker.h"
#include "imstkCollisionGraph.h"
    >> >> >> > 486fd64 4 ... temp commit: all examples working, bug fixes
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkScene.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates the concept of virtual coupling
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
#ifndef iMSTK_USE_OPENHAPTICS
    std::cout << "LaparoscopicToolController example needs haptic device to be enabled at build time" << std::endl;
    return 1;
#else if
    // simManager and Scene
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("VirtualCoupling");

    // Create a plane in the scene
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(400);
    planeGeom->setPosition(0.0, -50, 0.0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Create the virtual coupling object controller

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    simManager->addModule(server);

    // Device tracker
    auto deviceTracker = std::make_shared<DeviceTracker>(client);

    // Create a virtual coupling object
    auto visualGeom = std::make_shared<Sphere>();
    visualGeom->setRadius(20);
    auto collidingGeom = std::make_shared<Sphere>();
    collidingGeom->setRadius(20);
    auto obj = std::make_shared<CollidingObject>("VirtualCouplingObject");
    obj->setCollidingGeometry(collidingGeom);

    auto material    = std::make_shared<RenderMaterial>();
    auto visualModel = std::make_shared<VisualModel>(visualGeom);
    visualModel->setRenderMaterial(material);
    obj->addVisualModel(visualModel);

    // Add virtual coupling object (with visual, colliding, and physics geometry) in the scene.
    scene->addSceneObject(obj);

    // Create and add virtual coupling object controller in the scene
    auto objController = std::make_shared<SceneObjectController>(obj, deviceTracker);
    scene->addObjectController(objController);

    {
        // Setup CD, and collision data
        auto colData = std::make_shared<CollisionData>();

        std::shared_ptr<CollisionDetection> colDetect = makeCollisionDetectionObject(CollisionDetection::Type::UnidirectionalPlaneToSphere,
            planeObj->getCollidingGeometry(), obj->getCollidingGeometry(), colData);

        // Setup the handler
        auto colHandler = std::make_shared<VirtualCouplingCH>(CollisionHandling::Side::B, colData, obj);
        colHandler->setStiffness(5e-01);
        colHandler->setDamping(0.005);

        auto pair = std::make_shared<CollisionPair>(planeObj, obj, colDetect, nullptr, colHandler);
        scene->getCollisionGraph()->addInteraction(pair);
    }

    // Camera
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(200, 200, 200));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Running);

    return 0;
#endif
}
