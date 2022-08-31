/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNeedle.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleInteraction.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Returns a colliding object tissue plane that uses an implicit geometry for collision
///
static std::shared_ptr<CollidingObject>
createTissueObj()
{
    auto tissueObj = std::make_shared<CollidingObject>("Tissue");

    auto plane = std::make_shared<Plane>();
    plane->setWidth(0.1);

    tissueObj->setVisualGeometry(plane);
    tissueObj->setCollidingGeometry(plane);

    auto material = std::make_shared<RenderMaterial>();
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->setColor(Color::Bone);
    material->setRoughness(0.5);
    material->setMetalness(0.1);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);

    tissueObj->addComponent<Puncturable>();

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
createNeedleObj()
{
    auto                    toolGeom = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 0.0, -0.1) };
    VecDataArray<int, 2>    cells    = { Vec2i(0, 1) };
    toolGeom->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(cells));

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->scale(0.0075, Geometry::TransformType::ApplyToData);
    toolGeom->rotate(Vec3d(0.0, 1.0, 0.0), PI, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 0.0, 0.1), Geometry::TransformType::ApplyToData);

    auto toolObj = std::make_shared<RigidObject2>("NeedleRbdTool");
    toolObj->setVisualGeometry(syringeMesh);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, syringeMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);

    auto lineModel = std::make_shared<VisualModel>();
    lineModel->setGeometry(toolGeom);
    toolObj->addVisualModel(lineModel);
    //toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations       = 20;
    rbdModel->getConfig()->m_angularVelocityDamping = 0.8; // Helps with lack of 6dof
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 1000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.1, 0.0);

    auto needle = toolObj->addComponent<StraightNeedle>();
    needle->setNeedleGeometry(toolGeom);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<RigidObjectController>();
    controller->setControlledObject(toolObj);
    controller->setLinearKs(8000.0);
    controller->setLinearKd(200.0);
    controller->setAngularKs(1000000.0);
    controller->setAngularKd(100000.0);
    controller->setForceScaling(0.02);
    controller->setSmoothingKernelSize(5);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setUseForceFade(true);
    controllerGhost->setController(controller);

    return toolObj;
}

///
/// \brief This examples demonstrates rigid body with pivot constraint
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("RbdSDFNeedle");

    // Create the bone
    std::shared_ptr<CollidingObject> tissueObj = createTissueObj();
    scene->addSceneObject(tissueObj);

    // Create the needle
    std::shared_ptr<RigidObject2> needleObj = createNeedleObj();
    scene->addSceneObject(needleObj);

    // Setup interaction between tissue and needle
    scene->addInteraction(std::make_shared<NeedleInteraction>(tissueObj, needleObj));

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 0.2, 0.2);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setDirection(Vec3d(0.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.005, 0.005, 0.005);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.25;
                const Quatd desiredOrientation = Quatd(Rotd(-1.0, Vec3d(1.0, 0.0, 0.0)));

                deviceClient->setPosition(desiredPos);
                deviceClient->setOrientation(desiredOrientation);
            });
#endif
        auto controller = needleObj->getComponent<RigidObjectController>();
        controller->setDevice(deviceClient);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                needleObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
