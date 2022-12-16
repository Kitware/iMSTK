/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdRigidBaryPointToPointConstraint.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

static std::shared_ptr<PbdObject>
makePbdObjSurface(
    const std::string&        name,
    std::shared_ptr<PbdModel> model,
    const Vec3d&              size,
    const Vec3i&              dim,
    const Vec3d&              center)
{
    auto prismObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh  = prismMesh->extractSurfaceMesh();

    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->addComponent<Collider>()->setGeometry(surfMesh);
    prismObj->setVisualGeometry(surfMesh);
    prismObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    prismObj->setDynamicalModel(model);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1000.0,
                prismObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
                prismObj->getPbdBody()->bodyHandle);
    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = surfMesh->getVertexPositions();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            prismObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    return prismObj;
}

static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.5);
    toolGeometry->setLength(1);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->addComponent<Collider>()->setGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
                Vec3d(0.0, 5.0, 2.0),
                1.0,
                Quatd::Identity(),
                Mat3d::Identity() * 1.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.9);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setTranslationScaling(50.0);
    controller->setLinearKs(5000.0);
    controller->setAngularKs(1000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.001);
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
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    auto                            pbdModel  = std::make_shared<PbdModel>();
    std::shared_ptr<PbdModelConfig> pbdParams = pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 8;
    pbdParams->m_linearDampingCoeff = 0.003;

    // Setup a tissue to grasp
    std::shared_ptr<PbdObject> pbdObj = makePbdObjSurface("Tissue",
                pbdModel,
                Vec3d(4.0, 4.0, 4.0),  // Dimensions
                Vec3i(5, 5, 5),        // Divisions
                Vec3d(0.0, 0.0, 0.0)); // Center
    scene->addSceneObject(pbdObj);

    // Setup a tool to grasp with
    std::shared_ptr<PbdObject> toolObj = makeCapsuleToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // Add collision
    auto pbdToolCollision = std::make_shared<PbdObjectCollision>(pbdObj, toolObj);
    pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    scene->addInteraction(pbdToolCollision);

    // Create new picking with constraints
    auto toolPicking = std::make_shared<PbdObjectGrasping>(pbdObj, toolObj);
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

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.002);

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
                        toolPicking->beginVertexGrasp(dilatedCapsule);
                        //pbdToolCollision->setEnabled(false);
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        toolPicking->endGrasp();
                        //pbdToolCollision->setEnabled(true);
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
                //pbdToolCollision->setEnabled(false);
                        });
        connect<Event>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](Event*)
            {
                toolPicking->endGrasp();
                //pbdToolCollision->setEnabled(true);
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
                    //pbdToolCollision->setEnabled(false);
                }
                        });
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyRelease,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    toolPicking->endGrasp();
                    //pbdToolCollision->setEnabled(true);
                }
             });
        controller->setDevice(deviceClient);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
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