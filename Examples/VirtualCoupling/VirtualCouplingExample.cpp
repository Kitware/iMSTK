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
#include "imstkVTKViewer.h"
#include "imstkVTKRenderer.h"
#endif

using namespace imstk;

///
/// \brief This example demonstrates the concept of virtual coupling
/// for haptic interaction.
/// NOTE: Requires GeoMagic Touch device or Haply Inverse 3
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Scene
    auto scene = std::make_shared<Scene>("VirtualCoupling");

    std::shared_ptr<CollidingObject> obstacleObjs[] =
    {
        std::make_shared<CollidingObject>("Plane"),
        std::make_shared<CollidingObject>("Cube")
    };

    // Create a plane and cube for collision with scissors
    auto plane = std::make_shared<Plane>();
    plane->setWidth(0.4);
    obstacleObjs[0]->setVisualGeometry(plane);
    obstacleObjs[0]->setCollidingGeometry(plane);

    // 0.1m size cube, slight rotation
    auto cube = std::make_shared<OrientedBox>(Vec3d(0.0, 0.0, 0.0),
        Vec3d(0.05, 0.05, 0.05), Quatd(Rotd(1.0, Vec3d(0.0, 1.0, 0.0))));
    obstacleObjs[1]->setVisualGeometry(cube);
    obstacleObjs[1]->setCollidingGeometry(cube);

    for (int i = 0; i < 2; i++)
    {
        obstacleObjs[i]->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
        scene->addSceneObject(obstacleObjs[i]);
    }

    // The visual geometry is the scissor mesh read in from file
    auto rbdObj = std::make_shared<PbdObject>();
    {
        auto model = std::make_shared<PbdModel>();
        model->getConfig()->m_dt      = 0.001;
        model->getConfig()->m_gravity = Vec3d::Zero();
        rbdObj->setDynamicalModel(model);
        rbdObj->getPbdBody()->setRigid(Vec3d(0.0, 0.05, 0.0),
            7.0, Quatd::Identity(),
            Mat3d::Identity() * 100000000.0);

        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scissors/Metzenbaum Scissors/Metz_Scissors.stl");
        rbdObj->setCollidingGeometry(surfMesh);
        rbdObj->setVisualGeometry(surfMesh);
        rbdObj->setPhysicsGeometry(surfMesh);

        std::shared_ptr<RenderMaterial> mat = rbdObj->getVisualModel(0)->getRenderMaterial();
        mat->setShadingModel(RenderMaterial::ShadingModel::PBR);
        mat->setRoughness(0.5);
        mat->setMetalness(1.0);
        mat->setIsDynamicMesh(false);
    }
    scene->addSceneObject(rbdObj);

    // Setup a ghost tool object to show off virtual coupling
    auto ghostToolObj = std::make_shared<SceneObject>("GhostTool");
    ghostToolObj->setVisualGeometry(rbdObj->getVisualGeometry()->clone());
    auto ghostMaterial = std::make_shared<RenderMaterial>();
    ghostMaterial->setColor(Color::Orange);
    ghostMaterial->setLineWidth(5.0);
    ghostMaterial->setOpacity(0.3);
    ghostMaterial->setIsDynamicMesh(false);
    ghostToolObj->getVisualModel(0)->setRenderMaterial(ghostMaterial);
    scene->addSceneObject(ghostToolObj);

    // Create a virtual coupling controller
    // Balancing ks, device force scaling, and the mass
    auto controller = std::make_shared<PbdObjectController>();
    controller->setControlledObject(rbdObj);
    controller->setDevice(deviceClient);
    controller->setTranslationOffset(Vec3d(0.0, 0.05, 0.0));
    controller->setLinearKs(50000.0);
    controller->setAngularKs(10000000000.0);
    controller->setForceScaling(0.01);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);
    controller->setUseCritDamping(true);
    scene->addControl(controller);

    // Add interaction between the rigid object sphere and static plane
    scene->addInteraction(
        std::make_shared<PbdObjectCollision>(rbdObj, obstacleObjs[0]));
    scene->addInteraction(
        std::make_shared<PbdObjectCollision>(rbdObj, obstacleObjs[1]));

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.2, 0.35));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

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

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Run the rbd model in real time
                rbdObj->getPbdModel()->getConfig()->m_dt = driver->getDt();

                //ghostMaterial->setOpacity(std::min(1.0,controller->getDeviceForce().norm() / 15.0));
                ghostMaterial->setOpacity(1.0);

                // Also apply controller transform to ghost geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setTranslation(controller->getPosition());
                toolGhostMesh->setRotation(controller->getOrientation());
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();
            });

        // Add mouse and keyboard controls to the viewer
#ifdef iMSTK_USE_RENDERING_VTK
        auto mouseControl = std::make_shared<MouseSceneControl>();
        mouseControl->setDevice(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        scene->addControl(mouseControl);

        auto keyControl = std::make_shared<KeyboardSceneControl>();
        keyControl->setDevice(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        scene->addControl(keyControl);
#endif

        driver->start();
    }

    return 0;
}
