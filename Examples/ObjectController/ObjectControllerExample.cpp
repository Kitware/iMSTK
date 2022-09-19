/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollidingObject.h"
#include "imstkCylinder.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling a object using external
/// device.
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    auto scene = std::make_shared<Scene>("ObjectController");

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    std::shared_ptr<AnalyticalGeometry> geometries[] = {
        std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.01, 0.05, 0.01)),
        std::make_shared<Plane>(Vec3d::Zero(), Vec3d(0.0, 0.01, 0.0)),
        std::make_shared<Capsule>(Vec3d::Zero(), 0.005, 0.01),
        std::make_shared<Cylinder>(Vec3d::Zero(), 0.05, 0.01),
        std::make_shared<Sphere>(Vec3d::Zero(), 0.02)
    };

    auto object = std::make_shared<SceneObject>("VirtualObject");
    object->setVisualGeometry(geometries[0]);
    scene->addSceneObject(object);

    auto controller = std::make_shared<SceneObjectController>();
    controller->setControlledObject(object);
    controller->setDevice(deviceClient);
    scene->addControl(controller);

    // Update Camera position
    std::shared_ptr<Camera> cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0.0, 0.0, 0.3));
    cam->setFocalPoint(geometries[0]->getPosition());

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
        driver->setDesiredDt(0.01);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }
    return 0;
}
