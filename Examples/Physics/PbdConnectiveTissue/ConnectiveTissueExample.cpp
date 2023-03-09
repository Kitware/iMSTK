/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBurnable.h"
#include "imstkBurner.h"
#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkCollisionUtils.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkPbdSystem.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTearable.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Creates pbd simulated gallbladder object
///
std::shared_ptr<Entity>
makeGallBladder(const std::string& name, std::shared_ptr<PbdSystem> system)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Gallblader/gallblader.msh");
    const Vec3d center     = tissueMesh->getCenter();
    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(10.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { -0.4, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Setup the Object
    auto tissueObj   = SceneUtils::makePbdEntity(name, surfMesh, system);
    auto visualModel = tissueObj->getComponent<VisualModel>();
    visualModel->setRenderMaterial(material);
    // Gallblader is about 60g
    auto tissueMethod = tissueObj->getComponent<PbdMethod>();
    tissueMethod->setUniformMass(60.0 / tissueMesh->getNumVertices());

    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 700.0,
        tissueMethod->getBodyHandle());
    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Dihedral, 700.0,
        tissueMethod->getBodyHandle());

    tissueMethod->setFixedNodes({ 57, 131, 132 }); // { 72, , 131, 132 };

    LOG(INFO) << "Per particle mass: " << tissueMethod->getPbdBody()->uniformMassValue;

    // tissueObj->initialize();

    return tissueObj;
}

static std::shared_ptr<Entity>
makeKidney(const std::string& name, std::shared_ptr<PbdSystem> system)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Kidney/kidney_vol_low_rez.vtk");
    const Vec3d center     = tissueMesh->getCenter();

    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(10.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 1.0, 0.0), 90.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { 0.4, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, tissueMesh, surfMesh, tissueMesh, system);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);

    // Gallblader is about 60g
    auto method = tissueObj->getComponent<PbdMethod>();
    method->setUniformMass(60.0 / tissueMesh->getNumVertices());
    method->setFixedNodes({ 72, 57, 131, 132 });

    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 500.0,
        method->getBodyHandle());
    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Volume, 500.0,
        method->getBodyHandle());

    LOG(INFO) << "Per particle mass: " << method->getPbdBody()->uniformMassValue;

    return tissueObj;
}

static std::shared_ptr<Entity>
makeCapsuleToolObj(std::shared_ptr<PbdSystem> system)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.03);
    toolGeometry->setLength(0.4);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    auto toolObj    = SceneUtils::makePbdEntity("Tool", toolGeometry, system);
    auto toolVisual = toolObj->getComponent<VisualModel>();
    auto toolMethod = toolObj->getComponent<PbdMethod>();
    // Create the object
    toolMethod->setRigid(
        Vec3d(0.0, 2.0, 2.0),
        0.1,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    toolVisual->getRenderMaterial()->setOpacity(1.0);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolMethod, toolVisual);
    controller->setTranslationScaling(10.0);
    controller->setLinearKs(500.0);
    controller->setAngularKs(200.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.8);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdConnectiveTissue");
    scene->getActiveCamera()->setPosition(0.278448, 0.0904159, 3.43076);
    scene->getActiveCamera()->setFocalPoint(0.0703459, -0.539532, 0.148011);
    scene->getActiveCamera()->setViewUp(-0.0400007, 0.980577, -0.19201);

    // Setup the PBD Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_doPartitioning = false;
    pbdSystem->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdSystem->getConfig()->m_iterations = 5;
    pbdSystem->getConfig()->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdSystem->getConfig()->m_linearDampingCoeff  = 0.005; // Removed from velocity
    pbdSystem->getConfig()->m_angularDampingCoeff = 0.005;

    // Setup gallbladder object
    auto gallbladerObj = makeGallBladder("Gallbladder", pbdSystem);
    scene->addSceneObject(gallbladerObj);

    // Setup kidney
    auto kidneyObj = makeKidney("Kidney", pbdSystem);
    scene->addSceneObject(kidneyObj);

    // Create PBD object of connective strands with associated constraints
    double maxDist = 0.35;
    auto   connectiveStrands       = makeConnectiveTissue(gallbladerObj, kidneyObj, pbdSystem, maxDist, 2.5, 7);
    auto   connectiveStrandsMethod = connectiveStrands->getComponent<PbdMethod>();
    pbdSystem->getConfig()->setBodyDamping(connectiveStrandsMethod->getBodyHandle(), 0.015, 0.0);

    // Add Tearing
    connectiveStrands->addComponent<Tearable>();

    // Add burnable
    auto burnable = std::make_shared<Burnable>();
    connectiveStrands->addComponent(burnable);

    scene->addSceneObject(connectiveStrands);

    // Setup a tool to grasp with
    auto toolObj = makeCapsuleToolObj(pbdSystem);
    scene->addSceneObject(toolObj);

    // add collision
    auto collision = std::make_shared<PbdObjectCollision>(connectiveStrands, toolObj);
    scene->addInteraction(collision);

    // Create new picking with constraints
    auto grasper = std::make_shared<PbdObjectGrasping>(connectiveStrands->getComponent<PbdMethod>(), toolObj->getComponent<PbdMethod>());
    grasper->setStiffness(0.5);
    scene->addInteraction(grasper);

    // Add burner component to tool
    auto burning = std::make_shared<Burner>();
    burning->addObject(connectiveStrandsMethod);

    toolObj->addComponent(burning);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation pause

        auto driver = std::make_shared<SimulationManager>();
        driver->setDesiredDt(0.005);
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        auto controller = toolObj->getComponent<PbdObjectController>();
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            controller->setTranslationOffset(Vec3d(2.0, 0.0, -2.0));
        }
        std::shared_ptr<DeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    if (e->m_button == 1)
                    {
                        // Use a slightly larger capsule since collision prevents intersection
                        auto capsule = std::dynamic_pointer_cast<Capsule>(toolObj->getComponent<Collider>()->getGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        grasper->beginCellGrasp(dilatedCapsule);
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        grasper->endGrasp();
                    }
                }
            });
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;

                deviceClient->setPosition(worldPos);
            });

        // Add click event and side effects
        connect<Event>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](Event*)
            {
                grasper->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getComponent<Collider>()->getGeometry()));
                //pbdToolCollision->setEnabled(false);
            });
        connect<Event>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](Event*)
            {
                grasper->endGrasp();
                //pbdToolCollision->setEnabled(true);
            });
#endif

        // auto controller = toolObj->getComponent<PbdObjectController>();
        controller->setDevice(deviceClient);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Add keyboard controlls for burning and grasping (Note: only for haptic devices without buttons)
        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // If b pressed, burn
                if (keyDevice->getButton('b') == KEY_PRESS)
                {
                    burning->start();
                }
                if (keyDevice->getButton('b') == KEY_RELEASE)
                {
                    burning->stop();
                }
                // If g pressed, grasp
                if (keyDevice->getButton('g') == KEY_PRESS)
                {
                    // Use a slightly larger capsule since collision prevents intersection
                    auto capsule = std::dynamic_pointer_cast<Capsule>(Collider::getCollidingGeometryFromEntity(toolObj.get()));
                    auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                    dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                    grasper->beginCellGrasp(dilatedCapsule);
                }
                if (keyDevice->getButton('g') == KEY_RELEASE)
                {
                    grasper->endGrasp();
                }
            });

        driver->start();
    }

    return 0;
}
