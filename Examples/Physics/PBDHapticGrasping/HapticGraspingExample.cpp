/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkControllerForceText.h"
#include "imstkEntity.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPbdSystem.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkMeshIO.h"

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
makeGallBladder(const std::string& name, std::shared_ptr<PbdSystem> model)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Gallblader/gallblader.msh");
    const Vec3d center     = tissueMesh->getCenter();
    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(1.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { -0.0, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, surfMesh, surfMesh, tissueMesh, model);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);
    auto tissueMethod = tissueObj->getComponent<PbdMethod>();
    tissueMethod->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));

    // Gallblader is about 60g
    tissueMethod->setUniformMass(0.6 / tissueMesh->getNumVertices());

    model->getConfig()->m_secParams->m_YoungModulus = 108000.0;
    model->getConfig()->m_secParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::NeoHookean, tissueMethod->getBodyHandle());
    model->getConfig()->setBodyDamping(tissueMethod->getBodyHandle(), 0.01);

    // tissueObj->getPbdBody()->fixedNodeIds = { 57, 131, 132 }; // { 72, , 131, 132 };

    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = tissueMesh->getVertexPositions();
    for (int i = 0; i < tissueMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] >= 0.016)
        {
            tissueMethod->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    LOG(INFO) << "Per particle mass: " << tissueMethod->getPbdBody()->uniformMassValue;

    tissueMethod->initialize();

    return tissueObj;
}

///
/// \brief Creates pbd simulated cube mesh for testing
///
static std::shared_ptr<Entity>
makePbdObjSurface(
    const std::string&         name,
    std::shared_ptr<PbdSystem> model,
    const Vec3d&               size,
    const Vec3i&               dim,
    const Vec3d&               center)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh  = prismMesh->extractSurfaceMesh();

    // Setup the Object
    auto prismObj = SceneUtils::makePbdEntity(name, surfMesh, surfMesh, prismMesh, model);
    prismObj->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    auto method = prismObj->getComponent<PbdMethod>();
    method->setUniformMass(0.06 / prismMesh->getNumVertices());
    method->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(prismMesh, surfMesh));
    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    model->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Dihedral, 1000.0,
        method->getBodyHandle());
    model->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 500.0,
        method->getBodyHandle());
    model->getConfig()->setBodyDamping(method->getBodyHandle(), 0.01);
    // Fix the borders
    auto             vertices = prismMesh->getVertexPositions();
    std::vector<int> fixedNodeIds;
    for (int i = 0; i < prismMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            fixedNodeIds.push_back(i);
        }
    }
    method->setFixedNodes(fixedNodeIds);

    return prismObj;
}

///
/// \brief Creates capsule to use as a tool
///
static std::shared_ptr<Entity>
makeCapsuleToolObj(std::shared_ptr<PbdSystem> model)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.003);
    toolGeometry->setLength(0.1);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    // Create the object
    auto toolObj = SceneUtils::makePbdEntity("Tool", toolGeometry, model);
    auto method  = toolObj->getComponent<PbdMethod>();
    method->setRigid(
        Vec3d(0.04, 0.0, 0.0),
        0.02,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    auto visualModel = toolObj->getComponent<VisualModel>();
    visualModel->getRenderMaterial()->setOpacity(1.0);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(method, visualModel);
    controller->setTranslationScaling(1.0);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(1.0);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

///
/// \brief This example demonstrates grasping interaction with a 3d pbd
/// simulated tissue
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdHapticGrasping");
    scene->getActiveCamera()->setPosition(0.00610397, 0.131126, 0.281497);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.00251247, 0.90946, -0.415783);

    auto                             pbdSystem = std::make_shared<PbdSystem>();
    std::shared_ptr<PbdSystemConfig> pbdParams = pbdSystem->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 8;
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup a tissue to grasp
    /*
    auto pbdObj = makePbdObjSurface("Tissue",
                pbdSystem,
                Vec3d(4.0, 4.0, 4.0),  // Dimensions
                Vec3i(5, 5, 5),        // Divisions
                Vec3d(0.0, 0.0, 0.0)); // Center
    */
    // Setup a gallbladder
    std::shared_ptr<Entity> pbdObj = makeGallBladder("Gallbladder", pbdSystem);
    scene->addSceneObject(pbdObj);

    // Setup a tool to grasp with
    auto toolObj = makeCapsuleToolObj(pbdSystem);
    scene->addSceneObject(toolObj);

    // Add collision
    auto pbdToolCollision = std::make_shared<PbdObjectCollision>(pbdObj, toolObj);
    pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    pbdToolCollision->setUseCorrectVelocity(true);
    scene->addInteraction(pbdToolCollision);

    // Create new picking with constraints
    auto toolPicking = std::make_shared<PbdObjectGrasping>(pbdObj->getComponent<PbdMethod>(), toolObj->getComponent<PbdMethod>());
    toolPicking->setStiffness(0.3);
    scene->addInteraction(toolPicking);

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
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.002);

        auto controller = toolObj->getComponent<PbdObjectController>();
        controller->setPosition(Vec3d(0.0, 0.0, 0.0));

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
                        toolPicking->beginVertexGrasp(dilatedCapsule);
                        pbdToolCollision->setEnabled(false);
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        toolPicking->endGrasp();
                        pbdToolCollision->setEnabled(true);
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
                toolPicking->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getComponent<Collider>()->getGeometry()));
                pbdToolCollision->setEnabled(false);
            });
        connect<Event>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](Event*)
            {
                toolPicking->endGrasp();
                pbdToolCollision->setEnabled(true);
            });
#endif

        // Alternative grasping by keyboard (in case device doesn't have a button)
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    auto capsule = std::dynamic_pointer_cast<Capsule>(toolObj->getComponent<Collider>()->getGeometry());
                    auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                    dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                    toolPicking->beginVertexGrasp(dilatedCapsule);
                    pbdToolCollision->setEnabled(false);
                }
                        });
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyRelease,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    toolPicking->endGrasp();
                    pbdToolCollision->setEnabled(true);
                }
             });
        controller->setDevice(deviceClient);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);

        // Add something to display controller force
        auto controllerForceTxt = mouseAndKeyControls->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
        controllerForceTxt->setCollision(pbdToolCollision);

        scene->addSceneObject(mouseAndKeyControls);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [ = ](Event*)
            {
                // Simulate in real time
                pbdSystem->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}
