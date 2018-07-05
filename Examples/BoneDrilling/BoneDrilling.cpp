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

#include "imstkMath.h"
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Geometry
#include "imstkSphere.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"

// Devices and controllers
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkSceneObjectController.h"

// Collisions
#include "imstkInteractionPair.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkBoneDrillingCH.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("BoneDrilling");

    // Add virtual coupling object in the scene.
#ifdef iMSTK_USE_OPENHAPTICS

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Device tracker
    auto deviceTracker = std::make_shared<DeviceTracker>(client);

    // Create bone scene object
    // Load the mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return 1;
    }
    auto bone = std::make_shared<CollidingObject>("Bone");
    bone->setCollidingGeometry(tetMesh);
    bone->setVisualGeometry(tetMesh);
    scene->addSceneObject(bone);

    // Create a virtual coupling object: Drill
    auto drillVisualGeom = std::make_shared<Sphere>();
    drillVisualGeom->setRadius(3.);
    auto drillCollidingGeom = std::make_shared<Sphere>();
    drillCollidingGeom->setRadius(3.);
    auto drill = std::make_shared<CollidingObject>("Drill");
    drill->setCollidingGeometry(drillCollidingGeom);
    drill->setVisualGeometry(drillVisualGeom);
    scene->addSceneObject(drill);

    // Create and add virtual coupling object controller in the scene
    auto objController = std::make_shared<SceneObjectController>(drill, deviceTracker);
    scene->addObjectController(objController);

    // Create a collision graph
    auto graph = scene->getCollisionGraph();
    auto pair = graph->addInteractionPair(bone,
        drill,
        CollisionDetection::Type::PointSetToSphere,
        CollisionHandling::Type::BoneDrilling,
        CollisionHandling::Type::None);

#endif

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    auto cam = scene->getCamera();
    cam->setPosition(Vec3d(0, 0, 15));

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::RUNNING);
    return 0;
}
