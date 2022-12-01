/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
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
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkPortHoleInteraction.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

//#define USE_TWO_HAPTIC_DEVICES

std::shared_ptr<PbdObject>
makeLapToolObj(const std::string&        name,
               std::shared_ptr<PbdModel> model)
{
    auto lapTool = std::make_shared<PbdObject>(name);

    const double capsuleLength = 0.3;
    auto         toolGeom      = std::make_shared<Capsule>(Vec3d(0.0, 0.0, capsuleLength * 0.5 - 0.005),
        0.002,
        capsuleLength,
        Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));

    auto lapToolVisualGeom = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/LapTool/laptool_all_in_one.obj");

    lapTool->setDynamicalModel(model);
    lapTool->setPhysicsGeometry(toolGeom);
    lapTool->addComponent<Collider>()->setGeometry(toolGeom);
    lapTool->setVisualGeometry(lapToolVisualGeom);
    lapTool->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, lapToolVisualGeom));

    std::shared_ptr<RenderMaterial> material = lapTool->getVisualModel(0)->getRenderMaterial();
    material->setIsDynamicMesh(false);
    material->setMetalness(1.0);
    material->setRoughness(0.2);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    lapTool->getPbdBody()->setRigid(
        Vec3d(0.0, 0.0, capsuleLength * 0.5) + Vec3d(0.0, 0.1, -1.0),
        10.0,
        Quatd::Identity(),
        Mat3d::Identity() * 0.08);

    auto controller = lapTool->addComponent<PbdObjectController>();
    controller->setControlledObject(lapTool);
    controller->setLinearKs(10000.0);
    controller->setAngularKs(10.0);
    controller->setForceScaling(0.01);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // The center of mass of the object is at the tip this allows most force applied
    // to the tool at the tip upon touch to be translated into linear force. Suitable
    // for 3dof devices.
    //
    // However, the point at which you actually apply force is on the back of the tool,
    // this is important for the inversion of control in lap tools (right movement at the
    // back should move the tip left).
    controller->setHapticOffset(Vec3d(0.0, 0.0, capsuleLength));

    return lapTool;
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

    auto scene = std::make_shared<Scene>("RbdLapToolCollision");

    auto bodyObject = std::make_shared<Entity>("body");
    {
        auto surfMesh  = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/human/full_body/body.obj");
        auto bodyPlane = std::make_shared<Plane>(Vec3d(0.0, 0.09, -1.0), Vec3d(0.0, 1.0, 0.0));
        bodyObject->addComponent<Collider>()->setGeometry(bodyPlane);
        auto visualModel = bodyObject->addComponent<VisualModel>();
        visualModel->setGeometry(surfMesh);
        visualModel->getRenderMaterial()->setShadingModel(
            RenderMaterial::ShadingModel::PBR);
        std::shared_ptr<RenderMaterial> material =
            visualModel->getRenderMaterial();
        material->setRoughness(0.8);
        material->setMetalness(0.1);
        material->setOpacity(0.5);
    }
    scene->addSceneObject(bodyObject);

    auto model = std::make_shared<PbdModel>();
    model->getConfig()->m_gravity = Vec3d::Zero();
    model->getConfig()->m_dt      = 0.001;
    model->getConfig()->m_doPartitioning = false;

    std::shared_ptr<PbdObject> lapTool1 = makeLapToolObj("lapTool1", model);
    scene->addSceneObject(lapTool1);

    std::shared_ptr<PbdObject> lapTool2 = makeLapToolObj("lapTool2", model);
    scene->addSceneObject(lapTool2);

    auto collision = std::make_shared<PbdObjectCollision>(lapTool1, lapTool2);
    collision->setRigidBodyCompliance(0.00001);
    scene->addInteraction(collision);

    // Plane with which to move haptic point of tool on
    auto mousePlane = std::make_shared<Plane>(Vec3d(0.03, 0.1, -0.95), Vec3d(0.1, 0.0, 1.0));
    mousePlane->setWidth(0.3);

    // Camera
    scene->getActiveCamera()->setPosition(-0.039, 0.57, -0.608);
    scene->getActiveCamera()->setFocalPoint(0.001, 0.178, -1.043);
    scene->getActiveCamera()->setViewUp(0.018, 0.742, -0.671);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setIntensity(1.0);
    scene->addLight("light", light);

    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();

#ifdef USE_TWO_HAPTIC_DEVICES
    std::shared_ptr<DeviceClient> leftDeviceClient = hapticManager->makeDeviceClient("Default Device");
    auto                          leftController   = lapTool2->getComponent<PbdObjectController>();
    leftController->setDevice(leftDeviceClient);
    leftController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));

    std::shared_ptr<DeviceClient> rightDeviceClient = hapticManager->makeDeviceClient("Device2");
    auto                          rightController   = lapTool1->getComponent<PbdObjectController>();
    rightController->setDevice(rightDeviceClient);
    rightController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));
#else
    std::shared_ptr<DeviceClient> leftDeviceClient = hapticManager->makeDeviceClient(); // Default device
    auto                          leftController   = lapTool2->getComponent<PbdObjectController>();
    leftController->setDevice(leftDeviceClient);
    leftController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));

    auto rightDeviceClient = std::make_shared<DummyClient>();
    auto rightController   = lapTool1->getComponent<PbdObjectController>();
    rightController->setDevice(rightDeviceClient);
#endif

    // Add port holes
    auto portHoleInteraction = lapTool1->addComponent<PortHoleInteraction>();
    portHoleInteraction->setTool(lapTool1);
    portHoleInteraction->setPortHoleLocation(Vec3d(0.015, 0.092, -1.117));
    auto sphere = std::make_shared<Sphere>(Vec3d(0.015, 0.092, -1.117), 0.01);
    auto rightPortVisuals = lapTool1->addComponent<VisualModel>();
    rightPortVisuals->setGeometry(sphere);
    portHoleInteraction->setToolGeometry(lapTool1->getComponent<Collider>()->getGeometry());
    portHoleInteraction->setCompliance(0.000001);

    auto portHoleInteraction2 = lapTool2->addComponent<PortHoleInteraction>();
    portHoleInteraction2->setTool(lapTool2);
    portHoleInteraction2->setPortHoleLocation(Vec3d(-0.065, 0.078, -1.127));
    auto sphere2 = std::make_shared<Sphere>(Vec3d(-0.065, 0.078, -1.127), 0.01);
    auto leftPortVisuals = lapTool2->addComponent<VisualModel>();
    leftPortVisuals->setGeometry(sphere2);
    portHoleInteraction2->setToolGeometry(lapTool2->getComponent<Collider>()->getGeometry());
    portHoleInteraction2->setCompliance(0.000001);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
        driver->setDesiredDt(0.001);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

#ifndef USE_TWO_HAPTIC_DEVICES
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
                a       = b.cross(mousePlane->getNormal());
                const double width = mousePlane->getWidth();
                rightDeviceClient->setPosition(mousePlane->getPosition() +
                    a * width * (mousePos[1] - 0.5) +
                    b * width * (mousePos[0] - 0.5) +
                    geom->getOrientation().toRotationMatrix().col(1).normalized() *
                    dummyOffset);
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseScroll,
            [&](MouseEvent* e)
            {
                dummyOffset += e->m_scrollDx * 0.01;
            });
#endif
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                model->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}
