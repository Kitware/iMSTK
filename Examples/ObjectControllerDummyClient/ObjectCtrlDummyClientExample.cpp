/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkDummyClient.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
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

    imstkNew<OrientedBox> geom(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 1.0));

    imstkNew<CollidingObject> object("VirtualObject");
    object->setVisualGeometry(geom);
    object->setCollidingGeometry(geom);
    scene->addSceneObject(object);

    // Update Camera position
    scene->getActiveCamera()->setPosition(0.0, 0.0, 10.0);
    scene->getActiveCamera()->setFocalPoint(geom->getPosition());

    // Device Client
    imstkNew<DummyClient>           client("DummyClient");
    imstkNew<SceneObjectController> controller;
    controller->setControlledObject(object);
    controller->setDevice(client);
    scene->addControl(controller);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        double t = 0.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                t += sceneManager->getDt();
                client->setPosition(Vec3d(cos(t) * 2.0, 0.0, 0.0));
            });

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}