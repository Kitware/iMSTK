/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "InflatableObject.h"

#ifdef iMSTK_USE_HAPTICS
#define EXAMPLE_USE_HAPTICS
#endif

#ifdef EXAMPLE_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

static std::shared_ptr<RigidObject2>
makeToolObj(const std::string& name)
{
    //auto toolGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 1.0);
    auto                    toolGeom = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 2.0, 0.0) };
    VecDataArray<int, 2>    indices  = { Vec2i(0, 1) };
    toolGeom->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    auto toolObj = std::make_shared<RigidObject2>(name);
    toolObj->setVisualGeometry(toolGeom);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(10.0);

    auto rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 6;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 10.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos  = Vec3d(0.0, 0.8, 0.0);
    toolObj->getRigidBody()->m_isStatic = false;

    auto controller = toolObj->addComponent<RigidObjectController>();
    controller->setControlledObject(toolObj);
    controller->setTranslationScaling(10.0);
    controller->setLinearKs(20000.0);
    controller->setAngularKs(10000000.0);
    controller->setForceScaling(0.0);
    controller->setSmoothingKernelSize(15);
    controller->setUseCritDamping(true);
    controller->setUseForceSmoothening(true);

    return toolObj;
}

static void
inject(std::shared_ptr<InflatableObject> tissueObj, std::shared_ptr<RigidObject2> toolObj,
       Vec3d& toolTip, const double radius, const double rate)
{
    // The LineMesh used for collision with the PBD tissue
    std::shared_ptr<LineMesh> lineMesh = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
    const Vec3d               vertex   = lineMesh->getVertexPosition(0);

    if ((toolTip - vertex).norm() > 0.01)
    {
        toolTip = vertex;
        tissueObj->setUpdateAffectedConstraint();
    }

    tissueObj->inject(toolTip, radius, rate);
}

///
/// \brief This example demonstrates the inflatable distance and volume Constraint
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdInjectExample");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    const Vec3d  tissueSize   = Vec3d(10.0, 3.0, 10.0);
    const Vec3i  tissueDim    = Vec3i(20, 5, 20);
    const Vec3d  tissueCenter = Vec3d(0.1, -1.0, 0.0);
    const double radius       = tissueSize[0] / 5.0;
    auto         tissueObj    = std::make_shared<InflatableObject>("PbdTissue",
        tissueSize, tissueDim, tissueCenter);
    scene->addSceneObject(tissueObj);

    // Setup a tool
    Vec3d                         toolTip = tissueCenter + Vec3d(0.0, tissueSize[1] / 2.0, 0.0);
    std::shared_ptr<RigidObject2> toolObj = makeToolObj("RbdTool");
    scene->addSceneObject(toolObj);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

#ifdef EXAMPLE_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        connect<Event>(sceneManager, SceneManager::postUpdate, [&](Event*)
            {
                if (deviceClient->getButton(0))
                {
                    inject(tissueObj, toolObj, toolTip, radius, 0.001);
                }
                else if (deviceClient->getButton(1))
                {
                    inject(tissueObj, toolObj, toolTip, radius, -0.001);
                }
            });
#else
        auto deviceClient = std::make_shared<DummyClient>();
        // Use keyboard controls
        connect<Event>(sceneManager, SceneManager::preUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.5;

                deviceClient->setPosition(worldPos);
            });
#endif
        auto controller = toolObj->getComponent<RigidObjectController>();
        controller->setDevice(deviceClient);

        // Key controls for injection
        connect<Event>(sceneManager, SceneManager::preUpdate, [&](Event*)
            {
                if (viewer->getKeyboardDevice()->getButton('s') == KEY_PRESS)
                {
                    inject(tissueObj, toolObj, toolTip, radius, 0.001);
                }
                else if (viewer->getKeyboardDevice()->getButton('a') == KEY_PRESS)
                {
                    inject(tissueObj, toolObj, toolTip, radius, -0.001);
                }
            });

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                tissueObj->getPbdModel()->getConfig()->m_dt      = sceneManager->getDt();
            });

        std::cout << "================================================\n";
        std::cout << "Key s : injection \n" << "Key a : deflation \n";
        std::cout << "================================================\n\n";

        driver->start();
    }

    return 0;
}