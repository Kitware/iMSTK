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
#include "imstkKeyboardDeviceClient.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
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
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

int
pbdRigidInDeformableGraspingExample()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRigidInDeformableGrasping");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 0.004, 0.1);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdSystem = std::make_shared<PbdSystem>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.01;
    pbdConfig->m_doPartitioning      = false;
    pbdSystem->configure(pbdConfig);

    EntityPtr tissueObj;
    {
        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Organs/Vessels/vessel_test.obj");

        // Setup the Parameters
        pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10000.0);
        pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.1);

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->setOpacity(0.5);

        // Setup the Object
        SceneUtils::makePbdEntity("tissue", surfMesh, pbdSystem);
        tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);
        tissueObj->getComponent<PbdMethod>()->setUniformMass(1.0);
    }
    scene->addSceneObject(tissueObj);

    // auto capsule0Obj = std::make_shared<Entity>("capsule0");
    EntityPtr capsule0Obj;
    {
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.0018);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.004, 0.01);
        capsule0Obj = SceneUtils::makePbdEntity("capsule0", rigidGeom, pbdSystem);

        // Setup material
        auto material = capsule0Obj->getComponent<VisualModel>()->getRenderMaterial();
        material->setColor(Color(1.0, 0.0, 0.0));
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->setRoughness(0.5);
        material->setMetalness(1.0);
        material->setIsDynamicMesh(false);

        // Setup body
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0067, 0.0027, 0.0));
        capsule0Obj->getComponent<PbdMethod>()->setRigid(
            Vec3d(0.0085, 0.0037, 0.0),
            100.0,
            orientation,
            Mat3d::Identity() * 0.005);
    }
    scene->addSceneObject(capsule0Obj);

    auto collision0 = std::make_shared<PbdObjectCollision>(tissueObj, capsule0Obj);
    collision0->setRigidBodyCompliance(0.00001);
    scene->addInteraction(collision0);

    EntityPtr                  lapTool;
    std::shared_ptr<PbdMethod> lapToolMethod;
    {
        const double capsuleLength = 0.3;
        auto         toolGeom      = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0),
            0.002, capsuleLength, Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0)));

        lapTool = SceneUtils::makePbdEntity("lapTool", toolGeom, pbdSystem);
        auto                            lapToolVisualModel = lapTool->getComponent<VisualModel>();
        std::shared_ptr<RenderMaterial> material = lapToolVisualModel->getRenderMaterial();
        material->setIsDynamicMesh(false);
        material->setMetalness(1.0);
        material->setRoughness(0.2);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);

        lapToolMethod = lapTool->getComponent<PbdMethod>();
        lapToolMethod->setRigid(
            Vec3d(0.0, 0.0, capsuleLength * 0.5), // Position
            6.0,                                  // Mass
            Quatd::Identity(), Mat3d::Identity() * 10000.0);

        auto controller = lapTool->addComponent<PbdObjectController>();
        controller->setControlledObject(lapToolMethod, lapToolVisualModel);
        controller->setLinearKs(1000000.0);
        controller->setAngularKs(100000000.0);
        controller->setForceScaling(0.003);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);

        // Add something to display controller force
        auto controllerForceTxt = lapTool->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
    }
    scene->addSceneObject(lapTool);

    // Add picking interaction for both jaws of the tool
    auto grasping = std::make_shared<PbdObjectGrasping>(tissueObj->getComponent<PbdMethod>(), lapTool->getComponent<PbdMethod>());
    grasping->setStiffness(0.05);
    scene->addInteraction(grasping);

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
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_HAPTICS
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        driver->addModule(hapticManager);
        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            auto rightController = lapTool->getComponent<PbdObjectController>();
            rightController->setTranslationOffset((*lapTool->getComponent<PbdMethod>()->getPbdBody()->vertices)[0] +
                Vec3d(0.1, 0.0, -0.1));
        }

        std::shared_ptr<DeviceClient> deviceClient = hapticManager->makeDeviceClient();

        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 1 && e->m_buttonState == BUTTON_PRESSED)
                {
                    LOG(INFO) << "Grasp!";
                    grasping->beginVertexGrasp(std::dynamic_pointer_cast<AnalyticalGeometry>(lapTool->getComponent<Collider>()->getGeometry()));
                }
            });
        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 1 && e->m_buttonState == BUTTON_RELEASED)
                {
                    LOG(INFO) << "Release!";
                    grasping->endGrasp();
                }
            });
#else
        auto deviceClient = std::make_shared<DummyClient>();
        auto dummyClientMovement = lapTool->addComponent<LambdaBehaviour>("DummyClientMovement");
        dummyClientMovement->setUpdate([ = ](const double&)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;
                deviceClient->setPosition(worldPos);
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    LOG(INFO) << "Grasp!";
                    grasping->beginVertexGrasp(std::dynamic_pointer_cast<AnalyticalGeometry>(lapTool->getComponent<Collider>()->getGeometry()));
                }
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    LOG(INFO) << "Release!";
                    grasping->endGrasp();
                }
            });
#endif
        auto rightController = lapTool->getComponent<PbdObjectController>();
        rightController->setDevice(deviceClient);
        rightController->setTranslationOffset((*lapToolMethod->getPbdBody()->vertices)[0]);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);
            connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress, [&](KeyEvent* e)
                {
                    if (e->m_key == '1')
                    {
                        if (pbdSystem->getConfig()->m_gravity[1] == 0.0)
                        {
                            pbdSystem->getConfig()->m_gravity = Vec3d(0.0, -1.0, 0.0);
                        }
                        else
                        {
                            pbdSystem->getConfig()->m_gravity = Vec3d::Zero();
                        }
                    }
                    else if (e->m_key == 'u')
                    {
                        scene->advance(sceneManager->getDt());
                        viewer->update();
                    }
                });

            // Add default mouse and keyboard controls to the viewer
            std::shared_ptr<Entity> mouseAndKeyControls =
                SimulationUtils::createDefaultSceneControl(driver);
            auto instructText = mouseAndKeyControls->getComponent<TextVisualModel>();
            instructText->setText(instructText->getText() +
                "\nPress Haptic Button or Click to grasp" +
                "\nPress 1 to toggle gravity");
            scene->addSceneObject(mouseAndKeyControls);
        }

        driver->start();
    }
    return 0;
}