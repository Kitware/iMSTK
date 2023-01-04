/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAxesModel.h"
#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkControllerForceText.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMeshIO.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

//#define USE_TWO_HAPTIC_DEVICES

#ifndef USE_TWO_HAPTIC_DEVICES
#include "imstkDummyClient.h"
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model, bool isLeft)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.005);
    toolGeometry->setLength(0.1);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0)));

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->addComponent<Collider>()->setGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.0, 0.1, 0.0),
        30.0,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.9);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setLinearKs(500000.0);
    controller->setAngularKs(10000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.002);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    auto axesModel = toolObj->addComponent<AxesModel>();
    axesModel->setScale(Vec3d(0.05, 0.05, 0.05));

    auto axesUpdate = toolObj->addComponent<LambdaBehaviour>("AxesModelUpdate");
    axesUpdate->setUpdate([ = ](const double& dt)
        {
            axesModel->setPosition((*toolObj->getPbdBody()->vertices)[0]);
            axesModel->setOrientation((*toolObj->getPbdBody()->orientations)[0]);
        });

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    // Add a component to display controller force
    auto controllerForceTxt = toolObj->addComponent<ControllerForceText>();
    if (isLeft)
    {
        controllerForceTxt->getText()->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
    }
    controllerForceTxt->setController(controller);

    return toolObj;
}

///
/// \brief This example demonstrates grasping interaction with a 3d pbd
/// rigid objects
///
int
PbdRigidBodyGraspingExample()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdRigidBodyGrasping");
    scene->getActiveCamera()->setPosition(0.0, 0.5, 0.5);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto                            pbdModel  = std::make_shared<PbdModel>();
    std::shared_ptr<PbdModelConfig> pbdParams = pbdModel->getConfig();
    pbdParams->m_gravity = Vec3d(0.0, -9.8, 0.0);
    //pbdParams->m_gravity = Vec3d::Zero();
    pbdParams->m_dt = 0.002;
    pbdParams->m_iterations = 8;
    pbdParams->m_linearDampingCoeff  = 0.01;
    pbdParams->m_angularDampingCoeff = 0.01;

    // Make a plane
    auto planeObj = std::make_shared<SceneObject>("PlaneObj");
    auto plane    = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(1.0);
    planeObj->addComponent<Collider>()->setGeometry(plane);
    planeObj->addComponent<VisualModel>()->setGeometry(plane);
    scene->addSceneObject(planeObj);

    // Make a pbd rigid body needle
    auto needleObj = std::make_shared<PbdObject>();
    {
        auto needleMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
        auto needleLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");
        // Transform so center of mass is in center of the needle
        needleMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleLineMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleMesh->scale(2.0, Geometry::TransformType::ApplyToData);
        needleLineMesh->scale(2.0, Geometry::TransformType::ApplyToData);
        needleObj->setVisualGeometry(needleMesh);
        needleObj->addComponent<Collider>()->setGeometry(needleLineMesh);
        needleObj->setPhysicsGeometry(needleLineMesh);
        needleObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(needleLineMesh, needleMesh));
        needleObj->setDynamicalModel(pbdModel);
        needleObj->getPbdBody()->setRigid(
            Vec3d(-0.1, 0.15, 0.0),
            1.0,
            Quatd::Identity(),
            Mat3d::Identity() * 0.01);
        needleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
    }
    scene->addSceneObject(needleObj);

    // Make a pbd rigid body sphere
    auto sphereObj = std::make_shared<PbdObject>();
    {
        auto sphereGeom = std::make_shared<Sphere>(Vec3d::Zero(), 0.01);
        sphereObj->setVisualGeometry(sphereGeom);
        sphereObj->addComponent<Collider>()->setGeometry(sphereGeom);
        sphereObj->setPhysicsGeometry(sphereGeom);
        sphereObj->setDynamicalModel(pbdModel);
        sphereObj->getPbdBody()->setRigid(
            Vec3d(0.1, 0.15, 0.0),
            1.0,
            Quatd::Identity(),
            Mat3d::Identity() * 0.01);
        sphereObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blood);
    }
    scene->addSceneObject(sphereObj);

    // Setup a tool to grasp with
    std::shared_ptr<PbdObject> leftToolObj = makeCapsuleToolObj(pbdModel, true);
    scene->addSceneObject(leftToolObj);
    std::shared_ptr<PbdObject> rightToolObj = makeCapsuleToolObj(pbdModel, false);
    scene->addSceneObject(rightToolObj);

    // Add collision between plane and objects
    auto planeCollision0 = std::make_shared<PbdObjectCollision>(needleObj, planeObj);
    planeCollision0->setRigidBodyCompliance(0.00001);
    scene->addInteraction(planeCollision0);
    auto planeCollision1 = std::make_shared<PbdObjectCollision>(sphereObj, planeObj);
    planeCollision1->setRigidBodyCompliance(0.00001);
    scene->addInteraction(planeCollision1);

    // Collision between needle and sphere
    auto sphereNeedleCollision = std::make_shared<PbdObjectCollision>(sphereObj, needleObj, "PointSetToSphereCD");
    sphereNeedleCollision->setRigidBodyCompliance(0.000001);
    scene->addInteraction(sphereNeedleCollision);

    // Add grasping
    auto leftGrasping0 = std::make_shared<PbdObjectGrasping>(needleObj, leftToolObj);
    leftGrasping0->setCompliance(0.00001);
    scene->addInteraction(leftGrasping0);
    auto leftGrasping1 = std::make_shared<PbdObjectGrasping>(sphereObj, leftToolObj);
    leftGrasping1->setCompliance(0.00001);
    scene->addInteraction(leftGrasping1);

    auto rightGrasping0 = std::make_shared<PbdObjectGrasping>(needleObj, rightToolObj);
    rightGrasping0->setCompliance(0.00001);
    scene->addInteraction(rightGrasping0);
    auto rightGrasping1 = std::make_shared<PbdObjectGrasping>(sphereObj, rightToolObj);
    rightGrasping1->setCompliance(0.00001);
    scene->addInteraction(rightGrasping1);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.05, 0.05, 0.05);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
        driver->setDesiredDt(0.002);

        auto leftController = leftToolObj->getComponent<PbdObjectController>();

        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            leftController->setTranslationOffset(Vec3d(0.1, 0.0, -0.1));
        }
        std::shared_ptr<DeviceClient> leftDeviceClient = hapticManager->makeDeviceClient();
        leftController->setDevice(leftDeviceClient);

        connect<ButtonEvent>(leftDeviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 1)
                {
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        // Use a slightly larger capsule since collision prevents intersection
                        auto capsule = std::dynamic_pointer_cast<Capsule>(leftToolObj->getComponent<Collider>()->getGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        leftGrasping0->beginCellGrasp(dilatedCapsule);
                        leftGrasping1->beginCellGrasp(dilatedCapsule);
                    }
                    else if (e->m_buttonState == BUTTON_RELEASED)
                    {
                        leftGrasping0->endGrasp();
                        leftGrasping1->endGrasp();
                    }
                }
            });
#ifdef USE_TWO_HAPTIC_DEVICES
        std::shared_ptr<DeviceClient> rightDeviceClient = hapticManager->makeDeviceClient("Device2");
        auto                          rightController   = rightToolObj->getComponent<PbdObjectController>();
        rightController->setDevice(rightDeviceClient);

        connect<ButtonEvent>(rightDeviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 1)
                {
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        // Use a slightly larger capsule since collision prevents intersection
                        auto capsule = std::dynamic_pointer_cast<Capsule>(rightToolObj->getComponent<Collider>()->getGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        rightGrasping0->beginCellGrasp(dilatedCapsule);
                        rightGrasping1->beginCellGrasp(dilatedCapsule);
                    }
                    else if (e->m_buttonState == BUTTON_RELEASED)
                    {
                        rightGrasping0->endGrasp();
                        rightGrasping1->endGrasp();
                    }
                }
            });
#else
        auto rightDeviceClient = std::make_shared<DummyClient>();
        auto rightController   = rightToolObj->getComponent<PbdObjectController>();
        rightController->setDevice(rightDeviceClient);

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;

                rightDeviceClient->setPosition(worldPos);
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
            {
                // Use a slightly larger capsule since collision prevents intersection
                auto capsule = std::dynamic_pointer_cast<Capsule>(rightToolObj->getComponent<Collider>()->getGeometry());
                auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                rightGrasping0->beginCellGrasp(dilatedCapsule);
                rightGrasping1->beginCellGrasp(dilatedCapsule);
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent* e)
            {
                rightGrasping0->endGrasp();
                rightGrasping1->endGrasp();
            });
#endif

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        auto instructText = mouseAndKeyControls->getComponent<TextVisualModel>();
        instructText->setText(instructText->getText() +
            "\nMouse Click/Press Haptic Device Button to grasp");
        scene->addSceneObject(mouseAndKeyControls);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Simulate in real time
                pbdModel->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}