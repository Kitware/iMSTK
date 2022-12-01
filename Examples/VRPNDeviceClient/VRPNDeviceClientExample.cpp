/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the object
/// using external device.
///
/// You should see a rectangular object that can be controlled with
/// whatever device you configured as the designated tracker
///
/// Expects vrpn_server to be running and listening to the default port
/// and have a tracker to configured as the "Tracker0" device in it `vrpn.cfg`
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("ObjectController");

    /// \ IP address of the server.
    const std::string serverIP = "localhost";

    /// \ the default VRPN port 38833
    const int serverPort = 38833;

    //VRPN Server
    auto server = std::make_shared<VRPNDeviceManager>(serverIP, serverPort);

    auto client = server->makeDeviceClient("Tracker0", VRPNTracker);

    std::shared_ptr<AnalyticalGeometry> geometries[] = {
        std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(1.0, 5.0, 1.0)),
    };

    imstkNew<SceneObject> object("VirtualObject");
    object->addComponent<VisualModel>()->setGeometry(geometries[0]);
    scene->addSceneObject(object);

    imstkNew<SceneObjectController> controller;
    controller->setControlledObject(object);
    controller->setDevice(client);
    controller->setTranslationScaling(0.1);
    scene->addControl(controller);

    // Update Camera position
    std::shared_ptr<Camera> cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0.0, 0.0, 10.0));
    cam->setFocalPoint(geometries[0]->getPosition());

    // Light
    imstkNew<DirectionalLight> light;
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    //Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(server);
        driver->setDesiredDt(0.01);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }
    return 0;
}