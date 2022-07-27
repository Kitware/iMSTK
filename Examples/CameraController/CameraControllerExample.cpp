/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCameraController.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSpotLight.h"
#include "imstkSurfaceMesh.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates controlling the camera using external
/// device. Attached is a spotlight. One could use RigidBodyController with
/// virtual coupling if they wanted damping/smoothness
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create Scene
    imstkNew<Scene> scene("CameraController");

    // Device Server
    imstkNew<HapticDeviceManager> server;
    server->setSleepDelay(1.0);
    std::shared_ptr<HapticDeviceClient> deviceClient = server->makeDeviceClient();

    // Load Mesh
    auto                  mesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    imstkNew<SceneObject> meshObject("meshObject");
    meshObject->setVisualGeometry(mesh);
    scene->addSceneObject(meshObject);

    imstkNew<SceneObject> planeObject("Plane");
    imstkNew<Plane>       plane(Vec3d(0.0, -2.0, 0.0));
    plane->setWidth(1000.0);
    planeObject->setVisualGeometry(plane);
    scene->addSceneObject(planeObject);

    // Update Camera position
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 10.0));

    imstkNew<CameraController> camController;
    camController->setCamera(scene->getActiveCamera());
    camController->setDevice(deviceClient);
    camController->setTranslationScaling(0.5);
    scene->addControl(camController);

    // Light
    imstkNew<SpotLight> light;
    light->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    light->setPosition(Vec3d(0.0, 10.0, 0.0));
    light->setIntensity(1.0);
    light->setSpotAngle(10.0);
    //light->setAttenuationValues(0.0, 0.0, 1.0); // Constant
    //light->setAttenuationValues(0.0, 0.5, 0.0); // Linear falloff
    light->setAttenuationValues(0.01, 0.0, 0.0); // Quadratic
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        // attach the camera controller to the viewer
        scene->addControl(camController);

        imstkNew<SimulationManager> driver;
        driver->addModule(server);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        // Change the spot angle when haptic button is pressed
        connect<ButtonEvent>(deviceClient, &HapticDeviceClient::buttonStateChanged,
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
