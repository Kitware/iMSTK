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
#include "imstkScene.h"
#include "imstkNew.h"
#include "imstkLogger.h"
#include "imstkSceneManager.h"
#include "imstkVTKViewer.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"

// Objects
#include "imstkCollidingObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Geometry
#include "imstkSphere.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"

// Devices and controllers
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkSceneObjectController.h"

// Collisions
#include "imstkCollisionGraph.h"
#include "imstkInteractionPair.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkBoneDrillingCH.h"
#include "imstkObjectInteractionFactory.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates bone drilling.
/// NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("BoneDrilling");

    // Add virtual coupling object in the scene.
#ifdef iMSTK_USE_OPENHAPTICS

    // Device Server
    auto server = std::make_shared<HapticDeviceManager>();
    auto client = server->makeDeviceClient(phantomOmni1Name);

    // Create bone scene object
    // Load the mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(FATAL) << "Could not read mesh from file.";
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
    auto objController = std::make_shared<SceneObjectController>(drill, client);
    scene->addController(objController);

    // Add interaction
    scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(bone, drill,
        InteractionType::FemObjToCollidingObjBoneDrilling, CollisionDetection::Type::PointSetToSphere));

#endif

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    //Run
    auto cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0, 0, 15));

    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        viewer->addChildThread(server);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }
    return 0;
}
