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

#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkCube.h"
#include "imstkDummyClient.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the object
/// using a simulated tracker: dummy client
///
int
main()
{
    Logger::startLogger();

    imstkNew<Scene> scene("ObjectControllerDummyClient");

    // Device Client
    imstkNew<DummyClient> client("DummyClient");

    // Object
    imstkNew<Cube> geom;
    geom->setPosition(0.0, 1.0, 0.0);
    geom->setWidth(2.0);

    imstkNew<CollidingObject> object("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    imstkNew<SceneObjectController> controller(object, client);
    controller->setTranslationScaling(0.1);
    scene->addController(controller);

    // Supply translation to dummy client frame
    auto translateFunc =
        [&client](Event*)
        {
            Vec3d p = client->getPosition() + Vec3d(1.0e-4, 0, 0);
            if (p.x() > 50.)
            {
                p = Vec3d(0, 0, 0);
            }
            client->setPosition(p);
        };

    // Update Camera position
    scene->getActiveCamera()->setPosition(0.0, 0.0, 10.0);
    scene->getActiveCamera()->setFocalPoint(geom->getPosition());

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        connect<Event>(sceneManager, EventType::PostUpdate, translateFunc);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        driver->start();
    }

    return 0;
}
