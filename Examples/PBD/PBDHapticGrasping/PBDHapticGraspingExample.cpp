/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdRigidBaryPointToPointConstraint.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
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
    const std::string& name,
    const Vec3d&       size,
    const Vec3i&       dim,
    const Vec3d&       center,
    const int          numIter)
{
    auto prismObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh  = prismMesh->extractSurfaceMesh();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 100.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 100.0);

    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = numIter;
    pbdParams->m_linearDampingCoeff  = 0.003;
    pbdParams->m_collisionIterations = 5;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Add a visual model to render the surface of the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);
    prismObj->addVisualModel(visualModel);

    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->setCollidingGeometry(surfMesh);
    prismObj->setDynamicalModel(pbdModel);
    prismObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {
        auto position = surfMesh->getVertexPosition(vert_id);

        if (position(1) == -2.0)
        {
            prismObj->getPbdBody()->fixedNodeIds.push_back(vert_id);
        }
    }

    return prismObj;
}

static std::shared_ptr<RigidObject2>
makeCapsuleToolObj()
{
    // Set up rigid body model
    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations       = 8;
    rbdModel->getConfig()->m_velocityDamping        = 1.0;
    rbdModel->getConfig()->m_angularVelocityDamping = 1.0;
    rbdModel->getConfig()->m_maxNumConstraints      = 40;

    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.5);
    toolGeometry->setLength(1);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));

    std::shared_ptr<RigidObject2> toolObj = std::make_shared<RigidObject2>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setDynamicalModel(rbdModel);
    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 1.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 5.0, 2.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.9);

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

    // Setup a tissue
    std::shared_ptr<PbdObject> pbdObj = makePbdObjSurface("Tissue",
        Vec3d(4.0, 4.0, 4.0), Vec3i(5, 5, 5), Vec3d(0.0, 0.0, 0.0), 8);
    scene->addSceneObject(pbdObj);

    std::shared_ptr<RigidObject2> toolObj = makeCapsuleToolObj();
    scene->addSceneObject(toolObj);

    auto rbdGhost     = std::make_shared<SceneObject>("ghost");
    auto ghostCapsule = std::make_shared<Capsule>();
    ghostCapsule->setRadius(0.5);
    ghostCapsule->setLength(1);
    ghostCapsule->setPosition(Vec3d(0.0, 0.0, 0.0));
    ghostCapsule->setOrientation(Quatd(0.707, 0.0, 0.0, 0.707));
    rbdGhost->setVisualGeometry(ghostCapsule);

    std::shared_ptr<RenderMaterial> ghostMat =
        std::make_shared<RenderMaterial>(*toolObj->getVisualModel(0)->getRenderMaterial());
    ghostMat->setOpacity(0.4);
    ghostMat->setColor(Color::Red);
    rbdGhost->getVisualModel(0)->setRenderMaterial(ghostMat);
    scene->addSceneObject(rbdGhost);

    // Add collision
    /*auto pbdToolCollision = std::make_shared<PbdObjectCollision>(PbdObj, toolObj, "SurfaceMeshToCapsuleCD");
    scene->addInteraction(pbdToolCollision);*/

    // Create new picking with constraints
    auto toolPicking = std::make_shared<PbdRigidObjectGrasping>(pbdObj, toolObj);
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
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.002);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    if (e->m_button == 1)
                    {
                        toolPicking->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry()));
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
                toolPicking->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry()));
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
                    toolPicking->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry()));
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

        auto controller = std::make_shared<RigidObjectController>();
        controller->setControlledObject(toolObj);
        controller->setDevice(deviceClient);
        controller->setTranslationScaling(50.0);
        controller->setLinearKs(5000.0);
        controller->setAngularKs(1000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.001);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                ghostMat->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 15.0));

                // Also apply controller transform to ghost geometry
                ghostCapsule->setTranslation(controller->getPosition());
                ghostCapsule->setRotation(controller->getOrientation());
                ghostCapsule->updatePostTransformData();
                ghostCapsule->postModified();
            });

        // Add mouse and keyboard controls to the viewer
        auto mouseControl = std::make_shared<MouseSceneControl>();
        mouseControl->setDevice(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        scene->addControl(mouseControl);

        auto keyControl = std::make_shared<KeyboardSceneControl>();
        keyControl->setDevice(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        scene->addControl(keyControl);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Simulate in real time
                pbdObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}