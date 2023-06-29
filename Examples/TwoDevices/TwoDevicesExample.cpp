/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkOrientedBox.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"

#ifdef iMSTK_USE_RENDERING_VTK
#include "imstkKeyboardSceneControl.h"
#include "imstkSimulationUtils.h"
#include "imstkVTKViewer.h"
#include "imstkVTKRenderer.h"
#endif

using namespace imstk;

std::shared_ptr<PbdObject>
makeTool(std::shared_ptr<DeviceClient> deviceClient)
{
    // The visual geometry is the scissor mesh read in from file
    auto rbdObj = std::make_shared<PbdObject>();
    auto model  = std::make_shared<PbdModel>();
    model->getConfig()->m_dt      = 0.001;
    model->getConfig()->m_gravity = Vec3d::Zero();
    rbdObj->setDynamicalModel(model);
    rbdObj->getPbdBody()->setRigid(
        Vec3d(0.0, 0.05, 0.0),            // Position
        7.0,                              // Mass
        Quatd::Identity(),                // Orientation
        Mat3d::Identity() * 100000000.0); // Inertia

    auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scissors/Metzenbaum Scissors/Metz_Scissors.stl");
    rbdObj->setCollidingGeometry(surfMesh);
    rbdObj->setVisualGeometry(surfMesh);
    rbdObj->setPhysicsGeometry(surfMesh);

    std::shared_ptr<RenderMaterial> mat = rbdObj->getVisualModel(0)->getRenderMaterial();
    mat->setShadingModel(RenderMaterial::ShadingModel::PBR);
    mat->setRoughness(0.5);
    mat->setMetalness(1.0);
    mat->setIsDynamicMesh(false);

    // Add a component for controlling via another device
    auto controller = rbdObj->addComponent<PbdObjectController>();
    controller->setControlledObject(rbdObj);
    controller->setDevice(deviceClient);
    controller->setTranslationOffset(Vec3d(0.0, 0.05, 0.0));
    controller->setLinearKs(50000.0);
    controller->setAngularKs(1000000000000.0);
    controller->setTranslationScaling(1.0);
    controller->setForceScaling(0.005);
    controller->setSmoothingKernelSize(10);
    controller->setUseForceSmoothening(true);
    controller->setUseCritDamping(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = rbdObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);
    return rbdObj;
}

///
/// \brief This example demonstrates the concept of virtual coupling
/// for haptic interaction.
/// NOTE: Requires GeoMagic Touch device or Haply Inverse 3
///
int
main(int argc, char* argv[])
{
    int deviceCount = 2;
    if (argc > 1)
    {
        std::string arg(argv[1]);
        std::cout << "Device count: " << arg << std::endl;
        deviceCount = std::min(2, std::max(0, std::stoi(arg)));
    }

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup default haptics manager
    std::shared_ptr<DeviceManager>             hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::vector<std::string>                   deviceName    = { "Right Device", "Left Device" };
    std::vector<std::shared_ptr<DeviceClient>> deviceClients;

    for (int i = 0; i < deviceCount; i++)
    {
        deviceClients.push_back(hapticManager->makeDeviceClient(deviceName[i]));
    }

    // Scene
    auto scene = std::make_shared<Scene>("VirtualCoupling");
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.2, 0.35));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    std::shared_ptr<CollidingObject> obstacleObjs[] =
    {
        std::make_shared<CollidingObject>("Plane"),
        // std::make_shared<CollidingObject>("Cube")
    };

    // Create a plane and cube for collision with scissors
    auto plane = std::make_shared<Plane>();
    plane->setWidth(0.4);
    obstacleObjs[0]->setVisualGeometry(plane);
    obstacleObjs[0]->setCollidingGeometry(plane);

    // 0.1m size cube, slight rotation
    /*auto cube = std::make_shared<OrientedBox>(Vec3d(0.0, 0.0, 0.0),
        Vec3d(0.05, 0.05, 0.05), Quatd(Rotd(1.0, Vec3d(0.0, 1.0, 0.0))));
    obstacleObjs[1]->setVisualGeometry(cube);
    obstacleObjs[1]->setCollidingGeometry(cube);*/

    for (int i = 0; i < 1; i++)
    {
        obstacleObjs[i]->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
        scene->addSceneObject(obstacleObjs[i]);
    }

    std::vector<std::shared_ptr<PbdObject>> toolObjs;

    for (auto client : deviceClients)
    {
        auto tool = makeTool(client);
        scene->addSceneObject(tool);

        toolObjs.push_back(tool);

        for (auto obj : obstacleObjs)
        {
            scene->addInteraction(std::make_shared<PbdObjectCollision>(tool, obj));
        }
    }

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->setPaused(true); // Start paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(hapticManager);
#ifdef iMSTK_USE_RENDERING_VTK
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        driver->addModule(viewer);
#endif
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                for (auto tool : toolObjs)
                {
                    tool->getPbdModel()->getConfig()->m_dt = driver->getDt();
                }
            });

        // Add mouse and keyboard controls to the viewer
#ifdef iMSTK_USE_RENDERING_VTK
        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);
#endif

        driver->start();
    }

    return 0;
}
