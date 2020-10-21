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

#include "imstkLogger.h"
#include "imstkMath.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkVTKViewer.h"

// Objects
#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkLight.h"

// Geometry
#include "imstkMeshIO.h"
#include "imstkSphere.h"
#include "imstkTetrahedralMesh.h"

// Devices and controllers
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkSceneObjectController.h"

// Collisions
#include "imstkCollisionGraph.h"
#include "imstkObjectInteractionFactory.h"

// global variables
const std::string phantomOmni1Name = "Default Device";

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

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient(phantomOmni1Name);

    // Create bone scene object
    // Load the mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(FATAL) << "Could not read mesh from file.";
        return 1;
    }
    imstkNew<CollidingObject> bone("Bone");
    bone->setCollidingGeometry(tetMesh);
    bone->setVisualGeometry(tetMesh);
    scene->addSceneObject(bone);

    // Create a virtual coupling object: Drill
    imstkNew<Sphere>          drillGeom(Vec3d(0.0, 0.0, 0.0), 3.0);
    imstkNew<CollidingObject> drill("Drill");
    drill->setCollidingGeometry(drillGeom);
    drill->setVisualGeometry(drillGeom);
    scene->addSceneObject(drill);

    // Create and add virtual coupling object controller in the scene
    imstkNew<SceneObjectController> controller(drill, client);
    scene->addController(controller);

    // Add interaction
    scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(bone, drill,
        InteractionType::CollidingObjToCollidingObjBoneDrilling, CollisionDetection::Type::PointSetToSphere));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    scene->getActiveCamera()->setPosition(Vec3d(0.0, 3.0, 25.0));

    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer 1");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager 1");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer
        viewer->addChildThread(server);       // Server starts/stops with viewer

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
