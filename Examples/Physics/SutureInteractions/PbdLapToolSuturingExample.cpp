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
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkMeshIO.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdMethod.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPbdSystem.h"
#include "imstkPlane.h"
#include "imstkPortHoleInteraction.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneControlText.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

//#define USE_TWO_HAPTIC_DEVICES

#ifndef USE_TWO_HAPTIC_DEVICES
#include "imstkDummyClient.h"
#include "imstkMouseDeviceClient.h"
#else
#include "imstkDeviceClient.h"
#endif

///
/// \brief Create a laprascopic tool object
///
std::shared_ptr<Entity>
makeLapToolObj(const std::string&         name,
               std::shared_ptr<PbdSystem> model)
{
    const double capsuleLength = 0.3;
    auto         toolGeom      = std::make_shared<Capsule>(
        Vec3d(0.0, 0.0, capsuleLength * 0.5 - 0.005), // Position
        0.002,                                        // Radius
        capsuleLength,                                // Length
        Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));     // Orientation

    const double lapToolHeadLength = 0.01;
    auto         graspCapsule      = std::make_shared<Capsule>(
        Vec3d(0.0, 0.0, lapToolHeadLength * 0.5),                           // Position
        0.004,                                                              // Radius
        lapToolHeadLength,                                                  // Length
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0))); // Orientation

    auto lapToolVisualGeom = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/LapTool/laptool_all_in_one.obj");

    auto lapTool       = SceneUtils::makePbdEntity(name, lapToolVisualGeom, toolGeom, toolGeom, model);
    auto lapToolVisual = lapTool->getComponent<VisualModel>();
    auto lapToolMethod = lapTool->getComponent<PbdMethod>();
    lapToolMethod->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, lapToolVisualGeom));

    std::shared_ptr<RenderMaterial> material = lapToolVisual->getRenderMaterial();
    material->setIsDynamicMesh(false);
    material->setMetalness(1.0);
    material->setRoughness(0.2);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    // Add grasp capsule for visualization
    auto graspVisualModel = std::make_shared<VisualModel>();
    graspVisualModel->setGeometry(graspCapsule);
    graspVisualModel->getRenderMaterial()->setIsDynamicMesh(false);
    graspVisualModel->setIsVisible(false);
    lapTool->addComponent(graspVisualModel);

    lapToolMethod->setRigid(
        Vec3d(0.0, 0.0, capsuleLength * 0.5) + Vec3d(0.0, 0.1, -1.0),
        5.0,
        Quatd::Identity(),
        Mat3d::Identity() * 0.08);

    auto controller = lapTool->addComponent<PbdObjectController>();
    controller->setControlledObject(lapToolMethod, lapToolVisual);
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

    // \todo: The grasp capsule and its map can't be placed as components yet.
    // For now grasp capsule is placed as a VisualModel and the map updated in this lambda
    auto graspCapsuleMap    = std::make_shared<IsometricMap>(toolGeom, graspCapsule);
    auto graspCapsuleUpdate = lapTool->addComponent<LambdaBehaviour>("graspCapsuleUpdate");
    graspCapsuleUpdate->setUpdate([ = ](const double&)
        {
            graspCapsuleMap->update();
        });

    return lapTool;
}

///
/// \brief Create pbd string object attached to a needle
///
static std::shared_ptr<Entity>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength,
    std::shared_ptr<Entity> needleObj)
{
    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Setup the Object
    auto                       needleMethod = needleObj->getComponent<PbdMethod>();
    std::shared_ptr<PbdSystem> system       = needleMethod->getPbdSystem();
    auto                       stringObj    = SceneUtils::makePbdEntity(name, stringMesh, system);
    stringObj->getComponent<VisualModel>()->setRenderMaterial(material);
    //stringObj->getPbdBody()->fixedNodeIds = { 0, 1, 19, 20 };
    auto stringMethod = stringObj->getComponent<PbdMethod>();
    stringMethod->setUniformMass(0.02);

    const int stringHandle = stringMethod->getBodyHandle();
    system->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1000.0,
        stringHandle);
    // It should have a high bend but without plasticity it's very difficult to use
    //system->getConfig()->enableBendConstraint(100.0, 1, true, stringHandle);
    system->getConfig()->enableBendConstraint(1.0, 1, true, stringHandle);
    //system->getConfig()->enableBendConstraint(0.1, 2, true, stringHandle);

    // Add a component to update the suture thread to be on the needle
    auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleMethod->getGeometry());

    // Add an attachment constraint for two-way between the string and needle
    // This is important to be able to pull the needle by the string
    system->getConfig()->addPbdConstraintFunctor([ = ](PbdConstraintContainer& container)
        {
            const Vec3d endOfNeedle   = (*needleLineMesh->getVertexPositions())[0];
            auto attachmentConstraint = std::make_shared<PbdBodyToBodyDistanceConstraint>();
            attachmentConstraint->initConstraint(system->getBodies(),
                { needleMethod->getBodyHandle(), 0 },
                endOfNeedle,
                { stringHandle, 0 }, // Start of string
                0.0,                 // Rest length
                0.0000001);
            container.addConstraint(attachmentConstraint);
        });

    return stringObj;
}

///
/// \brief This example demonstrates needle and thread grasping with proper
/// lap tool control. It is very hard to perform any complex movements
/// without two haptic devices.
///
int
LapToolSuturingExample()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("PbdLapToolSuturing");
    scene->getActiveCamera()->setFocalPoint(0.00100544, 0.0779848, -1.20601);
    scene->getActiveCamera()->setPosition(-0.000866941, 0.0832288, -1.20377);
    scene->getActiveCamera()->setViewUp(0.0601552, 0.409407, -0.910367);

    auto model = std::make_shared<PbdSystem>();
    model->getConfig()->m_gravity = Vec3d::Zero();
    model->getConfig()->m_dt      = 0.001;
    model->getConfig()->m_doPartitioning = false;

    auto bodyObject = std::make_shared<Entity>("body");
    {
        auto surfMesh  = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/human/full_body/body.obj");
        auto bodyPlane = std::make_shared<Plane>(Vec3d(0.0, -0.04, -1.0), Vec3d(0.0, 1.0, 0.0));
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

    auto leftToolObj = makeLapToolObj("leftLapTool", model);
    scene->addSceneObject(leftToolObj);
    auto rightToolObj = makeLapToolObj("rightLapTool", model);
    scene->addSceneObject(rightToolObj);

    // Make a pbd rigid body needle
    std::shared_ptr<Entity> needleObj;
    {
        auto needleMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
        auto needleLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");
        // Transform so center of mass is in center of the needle
        needleMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleLineMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleObj = SceneUtils::makePbdEntity("Needle", needleMesh, needleLineMesh, needleLineMesh, model);
        auto needleMethod = needleObj->getComponent<PbdMethod>();
        needleMethod->setPhysicsToVisualMap(std::make_shared<IsometricMap>(needleLineMesh, needleMesh));
        needleMethod->setRigid(
            Vec3d(0.02, 0.0, -1.26),
            1.0,
            Quatd::Identity(),
            Mat3d::Identity() * 0.01);
        needleObj->getComponent<VisualModel>()->getRenderMaterial()->setColor(Color::Orange);
    }
    scene->addSceneObject(needleObj);

    // Make a pbd simulated suture thread
    auto sutureThreadObj = makePbdString("sutureThread",
        Vec3d(0.02, 0.0, -1.26), Vec3d(0.0, 0.0, 1.0), 50, 0.2, needleObj);
    scene->addSceneObject(sutureThreadObj);

    auto collision = std::make_shared<PbdObjectCollision>(leftToolObj, rightToolObj);
    collision->setRigidBodyCompliance(0.00001);
    scene->addInteraction(collision);
    auto threadCollision0 = std::make_shared<PbdObjectCollision>(leftToolObj, sutureThreadObj);
    threadCollision0->setRigidBodyCompliance(0.0001);
    threadCollision0->setUseCorrectVelocity(false);
    //threadCollision0->setFriction(0.1);
    scene->addInteraction(threadCollision0);
    auto threadCollision1 = std::make_shared<PbdObjectCollision>(rightToolObj, sutureThreadObj);
    threadCollision1->setRigidBodyCompliance(0.0001);
    threadCollision1->setUseCorrectVelocity(false);
    //threadCollision1->setFriction(0.1);
    scene->addInteraction(threadCollision1);

    auto leftNeedleGrasping = std::make_shared<PbdObjectGrasping>(needleObj->getComponent<PbdMethod>(), leftToolObj->getComponent<PbdMethod>());
    leftNeedleGrasping->setCompliance(0.00001);
    scene->addInteraction(leftNeedleGrasping);
    auto leftThreadGrasping = std::make_shared<PbdObjectGrasping>(sutureThreadObj->getComponent<PbdMethod>(), leftToolObj->getComponent<PbdMethod>());
    leftThreadGrasping->setCompliance(0.00001);
    scene->addInteraction(leftThreadGrasping);
    auto rightNeedleGrasping = std::make_shared<PbdObjectGrasping>(needleObj->getComponent<PbdMethod>(), rightToolObj->getComponent<PbdMethod>());
    rightNeedleGrasping->setCompliance(0.00001);
    scene->addInteraction(rightNeedleGrasping);
    auto rightThreadGrasping = std::make_shared<PbdObjectGrasping>(sutureThreadObj->getComponent<PbdMethod>(), rightToolObj->getComponent<PbdMethod>());
    rightThreadGrasping->setCompliance(0.00001);
    scene->addInteraction(rightThreadGrasping);

    // Add thread-thread self collision
    auto threadOnThreadCollision = std::make_shared<PbdObjectCollision>(sutureThreadObj, sutureThreadObj);
    threadOnThreadCollision->setDeformableStiffnessA(0.05);
    threadOnThreadCollision->setDeformableStiffnessB(0.05);
    scene->addInteraction(threadOnThreadCollision);

    // Plane with which to move haptic point of tool on
    auto mousePlane = std::make_shared<Plane>(Vec3d(0.03, 0.1, -0.95), Vec3d(0.1, 0.0, 1.0));
    mousePlane->setWidth(0.3);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setIntensity(1.0);
    scene->addLight("light", light);

    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();

#ifdef USE_TWO_HAPTIC_DEVICES
    std::shared_ptr<DeviceClient> leftDeviceClient = hapticManager->makeDeviceClient("Default Device");
    auto                          leftController   = leftToolObj->getComponent<PbdObjectController>();
    leftController->setDevice(leftDeviceClient);
    leftController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));

    std::shared_ptr<DeviceClient> rightDeviceClient = hapticManager->makeDeviceClient("Device2");
    auto                          rightController   = rightToolObj->getComponent<PbdObjectController>();
    rightController->setDevice(rightDeviceClient);
    rightController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));

    connect<ButtonEvent>(rightDeviceClient, &DeviceClient::buttonStateChanged,
        [&](ButtonEvent* e)
        {
            if (e->m_button == 1)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    // Use a slightly larger capsule since collision prevents intersection
                    auto graspCapsule = std::dynamic_pointer_cast<Capsule>(
                        rightToolObj->getVisualModel(1)->getGeometry());
                    rightNeedleGrasping->beginCellGrasp(graspCapsule);
                    rightThreadGrasping->beginCellGrasp(graspCapsule);
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    rightNeedleGrasping->endGrasp();
                    rightThreadGrasping->endGrasp();
                }
            }
        });
#else
    std::shared_ptr<DeviceClient> leftDeviceClient = hapticManager->makeDeviceClient(); // Default device
    auto                          leftController   = leftToolObj->getComponent<PbdObjectController>();
    leftController->setDevice(leftDeviceClient);
    leftController->setTranslationOffset(Vec3d(0.0, 0.1, -1.0));

    auto rightDeviceClient = std::make_shared<DummyClient>();
    auto rightController   = rightToolObj->getComponent<PbdObjectController>();
    rightController->setDevice(rightDeviceClient);
#endif
    connect<ButtonEvent>(leftDeviceClient, &DeviceClient::buttonStateChanged,
        [&](ButtonEvent* e)
        {
            if (e->m_button == 1)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    auto graspCapsule = std::dynamic_pointer_cast<Capsule>(
                        leftToolObj->getComponentN<VisualModel>(1)->getGeometry());
                    leftNeedleGrasping->beginCellGrasp(graspCapsule);
                    leftThreadGrasping->beginCellGrasp(graspCapsule);
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    leftNeedleGrasping->endGrasp();
                    leftThreadGrasping->endGrasp();
                }
            }
        });

    // Add port holes
    auto portHoleInteraction = rightToolObj->addComponent<PortHoleInteraction>();
    portHoleInteraction->setTool(rightToolObj->getComponent<PbdMethod>());
    portHoleInteraction->setPortHoleLocation(Vec3d(0.015, 0.092, -1.117));
    auto sphere = std::make_shared<Sphere>(Vec3d(0.015, 0.092, -1.117), 0.01);
    auto rightPortVisuals = rightToolObj->addComponent<VisualModel>();
    rightPortVisuals->setGeometry(sphere);
    portHoleInteraction->setToolGeometry(rightToolObj->getComponent<Collider>()->getGeometry());
    portHoleInteraction->setCompliance(0.000001);

    auto portHoleInteraction2 = leftToolObj->addComponent<PortHoleInteraction>();
    portHoleInteraction2->setTool(leftToolObj->getComponent<PbdMethod>());
    portHoleInteraction2->setPortHoleLocation(Vec3d(-0.065, 0.078, -1.127));
    auto sphere2 = std::make_shared<Sphere>(Vec3d(-0.065, 0.078, -1.127), 0.01);
    auto leftPortVisuals = leftToolObj->addComponent<VisualModel>();
    leftPortVisuals->setGeometry(sphere2);
    portHoleInteraction2->setToolGeometry(leftToolObj->getComponent<Collider>()->getGeometry());
    portHoleInteraction2->setCompliance(0.000001);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
        driver->setDesiredDt(0.001);
        connect<Event>(driver, &SimulationManager::starting,
            [&](Event*)
            {
                sceneManager->setMode(SceneManager::Mode::Debug);
                viewer->setRenderingMode(Renderer::Mode::Debug);
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        auto instructText = mouseAndKeyControls->getComponent<TextVisualModel>();
        instructText->setText(instructText->getText() +
            "\nPress D to Switch to Laprascopic View"
            "\nPress Haptic Device Button to Grasp");
        scene->addSceneObject(mouseAndKeyControls);

#ifndef USE_TWO_HAPTIC_DEVICES
        // Process Mouse tool movement & presses
        double dummyOffset = -0.07;
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                std::shared_ptr<MouseDeviceClient> mouseDeviceClient = viewer->getMouseDevice();
                const Vec2d& mousePos = mouseDeviceClient->getPos();

                auto geom =
                    std::dynamic_pointer_cast<AnalyticalGeometry>(rightToolObj->getComponent<PbdMethod>()->getGeometry());

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
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent*)
            {
                auto graspCapsule = std::dynamic_pointer_cast<Capsule>(
                    rightToolObj->getComponentN<VisualModel>(1)->getGeometry());
                rightNeedleGrasping->beginCellGrasp(graspCapsule);
                rightThreadGrasping->beginCellGrasp(graspCapsule);
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent*)
            {
                rightNeedleGrasping->endGrasp();
                rightThreadGrasping->endGrasp();
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
