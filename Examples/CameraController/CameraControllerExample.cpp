/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCameraController.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSpotLight.h"
#include "imstkSurfaceMesh.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the camera using external
/// device. Attached is a spotlight.
/// Alternatively use a Pbd or RbdObjectController with virtual coupling
/// for smoothness
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    auto scene = std::make_shared<Scene>("CameraController");

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Load Mesh
    auto mesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    mesh->scale(0.01, Geometry::TransformType::ApplyToData);
    auto meshObj = std::make_shared<SceneObject>("MeshObj");
    meshObj->setVisualGeometry(mesh);
    scene->addSceneObject(meshObj);

    auto planeObj = std::make_shared<SceneObject>("Plane");
    auto plane    = std::make_shared<Plane>(Vec3d(0.0, -0.05, 0.0));
    plane->setWidth(0.5);
    planeObj->setVisualGeometry(plane);
    scene->addSceneObject(planeObj);

    // Update Camera position
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 1.0));

    // Light
    auto light = std::make_shared<SpotLight>();
    light->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    light->setPosition(Vec3d(0.0, 10.0, 0.0));
    light->setIntensity(1.0);
    light->setSpotAngle(10.0);
    //light->setAttenuationValues(0.0, 0.0, 1.0); // Constant
    //light->setAttenuationValues(0.0, 0.5, 0.0); // Linear falloff
    light->setAttenuationValues(50.0, 0.0, 0.0); // Quadratic
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(hapticManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Attach the camera controller to the viewer
        auto camController = std::make_shared<CameraController>();
        camController->setCamera(scene->getActiveCamera());
        camController->setDevice(deviceClient);
        scene->addControl(camController);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Change the spot angle when haptic button is pressed
        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    if (e->m_button == 0)
                    {
                        light->setSpotAngle(light->getSpotAngle() + 5.0);
                    }
                    else if (e->m_button == 1)
                    {
                        light->setSpotAngle(light->getSpotAngle() - 5.0);
                    }
                }
            });

        // Manually make the light follow the camera controller
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec3d pos = camController->getPosition();
                const Quatd orientation = camController->getOrientation();

                light->setPosition(pos);
                light->setFocalPoint(pos - orientation.toRotationMatrix().col(2));
            });

        driver->start();
    }

    return 0;
}
