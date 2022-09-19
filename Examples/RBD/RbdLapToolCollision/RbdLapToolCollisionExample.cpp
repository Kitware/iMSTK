/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkDummyClient.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief creates a tool rigid object under the provided rigid body model
///
std::shared_ptr<RigidObject2>
createToolObject(std::shared_ptr<RigidBodyModel2> model,
                 std::string                      name)
{
    auto toolObject = std::make_shared<RigidObject2>(name);

    const double capsuleLength = 0.3;
    auto         toolGeom      = std::make_shared<Capsule>(Vec3d(0.0, 0.0, capsuleLength * 0.5 - 0.005),
        0.002,
        capsuleLength,
        Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));

    auto lapToolVisualGeom = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/LapTool/laptool_body.obj");

    toolObject->setDynamicalModel(model);
    toolObject->setPhysicsGeometry(toolGeom);
    toolObject->setCollidingGeometry(toolGeom);
    toolObject->setVisualGeometry(lapToolVisualGeom);
    toolObject->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, lapToolVisualGeom));

    std::shared_ptr<RenderMaterial> material = toolObject->getVisualModel(0)->getRenderMaterial();
    material->setIsDynamicMesh(false);
    material->setMetalness(1.0);
    material->setRoughness(0.2);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    toolObject->getRigidBody()->m_mass = 0.1;
    toolObject->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObject->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, -1.0);

    return toolObject;
}

///
/// \brief This example demonstrates rigid body collisions with two capsule
/// lap tools. Left tool configured to haptic device, right to the mouse
/// on a plane
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("Rigid Body Dynamics");

    auto bodyObject = std::make_shared<CollidingObject>("body");
    {
        auto surfMesh  = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/human/full_body/body.obj");
        auto bodyPlane = std::make_shared<Plane>(Vec3d(0.0, 0.09, -1.0), Vec3d(0.0, 1.0, 0.0));
        bodyObject->setCollidingGeometry(bodyPlane);
        bodyObject->setVisualGeometry(surfMesh);
        bodyObject->getVisualModel(0)->getRenderMaterial()->setShadingModel(
            RenderMaterial::ShadingModel::PBR);
        std::shared_ptr<RenderMaterial> material =
            bodyObject->getVisualModel(0)->getRenderMaterial();
        material->setRoughness(0.8);
        material->setMetalness(0.1);
        material->setOpacity(0.5);
    }
    scene->addSceneObject(bodyObject);

    auto rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_dt      = 0.001;

    std::shared_ptr<RigidObject2> lapTool1 = createToolObject(rbdModel, "lapTool1");
    scene->addSceneObject(lapTool1);

    std::shared_ptr<RigidObject2> lapTool2 = createToolObject(rbdModel, "lapTool2");
    scene->addSceneObject(lapTool2);

    auto rbdInteraction = std::make_shared<RigidObjectCollision>(lapTool1, lapTool2);
    rbdInteraction->setFriction(0.0); // Don't use friction
    rbdInteraction->setBaumgarteStabilization(0.01);
    scene->addInteraction(rbdInteraction);

    auto mousePlane = std::make_shared<Plane>(Vec3d(0.03, 0.0, -1.23), Vec3d(0.1, 0.0, 1.0));
    mousePlane->setWidth(0.1);

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 0.4, -1.1);
    scene->getActiveCamera()->setFocalPoint(0.02, 0.03, -1.2);
    scene->getActiveCamera()->setViewUp(0.020056, 0.299489, -0.953889);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager   = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient    = hapticManager->makeDeviceClient();
    auto                           rightController = std::make_shared<RigidObjectController>();
    {
        rightController->setDevice(deviceClient);
        rightController->setControlledObject(lapTool1);
        rightController->setTranslationOffset(Vec3d(0.0, 0.0, -1.2));
        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            rightController->setTranslationOffset(Vec3d(0.2, 0.0, -1.2));
        }
        rightController->setLinearKs(5000.0);
        rightController->setAngularKs(100000000.0);
        rightController->setForceScaling(0.1);
        rightController->setSmoothingKernelSize(15);
        rightController->setUseForceSmoothening(true);
    }
    scene->addControl(rightController);

    auto       dummyClient = std::make_shared<DummyClient>();
    const Rotd rotX = Rotd(1.3, Vec3d(0.0, 0.0, 1.0));
    const Rotd rotY = Rotd(1.0, Vec3d(0.0, 1.0, 0.0));
    dummyClient->setOrientation(Quatd(rotX.matrix() * rotY.matrix()));
    auto leftController = std::make_shared<RigidObjectController>();
    {
        leftController->setDevice(dummyClient);
        leftController->setControlledObject(lapTool2);
        leftController->setLinearKs(5000.0);
        leftController->setAngularKs(100000000.0);
        leftController->setForceScaling(0.1);
        leftController->setSmoothingKernelSize(15);
        leftController->setUseForceSmoothening(true);
    }
    scene->addControl(leftController);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
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

        // Process Mouse tool movement & presses
        double dummyOffset = 0.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                std::shared_ptr<MouseDeviceClient> mouseDeviceClient = viewer->getMouseDevice();
                const Vec2d& mousePos = mouseDeviceClient->getPos();

                auto geom =
                    std::dynamic_pointer_cast<AnalyticalGeometry>(lapTool2->getPhysicsGeometry());

                // Use plane definition for dummy movement
                Vec3d a = Vec3d(0.0, 1.0, 0.0);
                Vec3d b = a.cross(mousePlane->getNormal()).normalized();
                const double width = mousePlane->getWidth();
                dummyClient->setPosition(mousePlane->getPosition() + a * width * (mousePos[1] - 0.5) +
                    b * width * (mousePos[0] - 0.5) +
                    geom->getOrientation().toRotationMatrix().col(1).normalized() *
                    dummyOffset);
            });
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                //printf("dt: %f\n", sceneManager->getDt());
                rbdModel->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}
