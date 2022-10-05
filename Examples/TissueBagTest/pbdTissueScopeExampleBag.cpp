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
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdAngularConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkCompositeImplicitGeometry.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkOrientedBox.h"
#include "imstkPbdObjectController.h"
#include "imstkDummyClient.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkSimulationUtils.h"
#include "imstkControllerForceText.h"
#include "imstkTextVisualModel.h"

using namespace imstk;

void
probe2Example()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRbdTissueScope");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 0.004, 0.1);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity = Vec3d(0.0, 0.0, 0.0);
    //pbdConfig->m_gravity = Vec3d(0.0, -9.8, 0.0);
    //pbdConfig->m_gravity = Vec3d(9.8, 0.0, 0.0);
    pbdConfig->m_dt = 0.001;
    pbdConfig->m_iterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.01;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    auto tissueObj = std::make_shared<PbdObject>("tissue");
    {
        auto surfMesh = MeshIO::read<SurfaceMesh>("C:/Users/Andx_/Desktop/vessel_test.obj");

        // Setup the Parameters
        pbdModel->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10000.0);
        pbdModel->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.1);

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        material->setOpacity(0.5);

        // Setup the Object
        tissueObj->setVisualGeometry(surfMesh);
        tissueObj->getVisualModel(0)->setRenderMaterial(material);
        tissueObj->setPhysicsGeometry(surfMesh);
        tissueObj->setCollidingGeometry(surfMesh);
        tissueObj->setDynamicalModel(pbdModel);

        tissueObj->getPbdBody()->uniformMassValue = 1.0;

        // Constrain particles to original location
        /*pbdModel->getConfig()->addPbdConstraintFunctor([=](PbdConstraintContainer& container)
            {
                std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
                for (int i = 0; i < verticesPtr->size(); i++)
                {
                    const PbdParticleId& pid = pbdModel->addVirtualParticle((*verticesPtr)[i], 0.0, Vec3d::Zero(), true);
                    auto distConstraint2 = std::make_shared<PbdDistanceConstraint>();
                    const double t = std::abs((*verticesPtr)[i][0]) / (0.039 * 0.5);
                    distConstraint2->initConstraint(0.0,
                        { tissueObj->getPbdBody()->bodyHandle, i }, pid, 1000.0 * t * t);
                    container.addConstraint(distConstraint2);
                }
            });*/
    }
    scene->addSceneObject(tissueObj);

    auto capsule0Obj = std::make_shared<PbdObject>("capsule0");
    {
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.0018);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.004, 0.01);
        capsule0Obj->setVisualGeometry(rigidGeom);
        capsule0Obj->setCollidingGeometry(rigidGeom);
        capsule0Obj->setPhysicsGeometry(rigidGeom);

        // Setup material
        capsule0Obj->getVisualModel(0)->getRenderMaterial()->setColor(Color(1.0, 0.0, 0.0));
        capsule0Obj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        capsule0Obj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        capsule0Obj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        capsule0Obj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        capsule0Obj->setDynamicalModel(pbdModel);

        // Setup body
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0067, 0.0027, 0.0));
        capsule0Obj->getPbdBody()->setRigid(
            Vec3d(0.0085, 0.0037, 0.0),
            100.0,
            orientation,
            Mat3d::Identity() * 0.005);
    }
    scene->addSceneObject(capsule0Obj);
    auto collision0 = std::make_shared<PbdObjectCollision>(tissueObj, capsule0Obj);
    collision0->setRigidBodyCompliance(0.00001);
    scene->addInteraction(collision0);

    /*auto lapTool1 = std::make_shared<PbdObject>("lapTool1");
    {
    }
    scene->addSceneObject(lapTool1);*/

    auto lapTool2 = std::make_shared<PbdObject>("lapTool2");
    {
        const double capsuleLength = 0.3;
        auto         toolGeom      = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0),
            0.002, capsuleLength, Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0)));

        lapTool2->setDynamicalModel(pbdModel);
        lapTool2->setPhysicsGeometry(toolGeom);
        lapTool2->setCollidingGeometry(toolGeom);
        lapTool2->setVisualGeometry(toolGeom);

        std::shared_ptr<RenderMaterial> material = lapTool2->getVisualModel(0)->getRenderMaterial();
        material->setIsDynamicMesh(false);
        material->setMetalness(1.0);
        material->setRoughness(0.2);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);

        lapTool2->getPbdBody()->setRigid(
            Vec3d(0.0, 0.0, capsuleLength * 0.5), // Position
            6.0,                                  // Mass
            Quatd::Identity(), Mat3d::Identity() * 10000.0);

        auto controller = lapTool2->addComponent<PbdObjectController>();
        controller->setControlledObject(lapTool2);
        controller->setLinearKs(1000000.0);
        controller->setAngularKs(100000000.0);
        controller->setForceScaling(0.003);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);

        // Add something to display controller force
        auto controllerForceTxt = lapTool2->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
    }
    scene->addSceneObject(lapTool2);

    // Add picking interaction for both jaws of the tool
    auto grasping = std::make_shared<PbdObjectGrasping>(tissueObj, lapTool2);
    grasping->setStiffness(0.01);
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

        std::shared_ptr<DeviceManager> hapticManager   = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  hapticDevice    = hapticManager->makeDeviceClient();
        auto                           rightController = lapTool2->getComponent<PbdObjectController>();
        rightController->setDevice(hapticDevice);
        rightController->setTranslationOffset((*lapTool2->getPbdBody()->vertices)[0]);
        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            rightController->setTranslationOffset((*lapTool2->getPbdBody()->vertices)[0] +
                Vec3d(0.1, 0.0, -0.1));
        }
        driver->addModule(hapticManager);

        //auto controller = std::make_shared<PbdObjectController>();
        ////#ifdef iMSTK_USE_OPENHAPTICS
        ////        auto hapticManager = std::make_shared<HapticDeviceManager>();
        ////        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        ////        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        ////        driver->addModule(hapticManager);
        ////
        ////        controller->setTranslationScaling(0.05);
        ////#else
        //auto deviceClient = std::make_shared<DummyClient>();
        //connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        //    {
        //        const Vec2d mousePos = viewer->getMouseDevice()->getPos();
        //        const Vec3d dx = Vec3d(0.0, mousePos[1] - 0.5, mousePos[0] - 0.5).cwiseProduct(Vec3d(0.0, 1.0, 0.01));
        //        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), probeDir.normalized());
        //        const Vec3d worldPos = probeStart + orientation._transformVector(dx);

        //        deviceClient->setPosition(worldPos);
        //        deviceClient->setOrientation(orientation);
        //    });

        ////#endif
        //controller->setControlledObject(probeCapsules[0]);
        //controller->setDevice(deviceClient);
        //controller->setLinearKs(50.0);
        //controller->setAngularKs(1.0);
        //controller->setUseCritDamping(true);
        //controller->setForceScaling(0.0);
        ////controller->setUseForceSmoothening(true);
        //scene->addControl(controller);

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
                        if (pbdModel->getConfig()->m_gravity[0] == 0.0)
                        {
                            pbdModel->getConfig()->m_gravity = Vec3d(0.0, -1.0, 0.0);
                        }
                        else
                        {
                            pbdModel->getConfig()->m_gravity = Vec3d::Zero();
                        }
                    }
                    else if (e->m_key == 'u')
                    {
                        scene->advance(sceneManager->getDt());
                        viewer->update();
                    }
                });
            // Alternative grasping by keyboard (in case device doesn't have a button)
            connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
                [&](KeyEvent* e)
                {
                    if (e->m_key == 'g')
                    {
                        auto capsule = std::dynamic_pointer_cast<Capsule>(lapTool2->getCollidingGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        grasping->beginVertexGrasp(dilatedCapsule);
                        //pbdToolCollision->setEnabled(false);
                    }
                });
            connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyRelease,
                [&](KeyEvent* e)
                {
                    if (e->m_key == 'g')
                    {
                        grasping->endGrasp();
                        //pbdToolCollision->setEnabled(true);
                    }
                });
            connect<ButtonEvent>(hapticDevice, &DeviceClient::buttonStateChanged,
                [&](ButtonEvent* e)
                {
                    if (e->m_button == 1 && e->m_buttonState == BUTTON_PRESSED)
                    {
                        LOG(INFO) << "Grasp!";
                        grasping->beginVertexGrasp(std::dynamic_pointer_cast<AnalyticalGeometry>(lapTool2->getCollidingGeometry()));
                        /*grasping->beginCellGrasp(
                            std::dynamic_pointer_cast<AnalyticalGeometry>(lapTool2->getCollidingGeometry()),
                            "SurfaceMeshToCapsuleCD");*/
                    }
                });
            connect<ButtonEvent>(hapticDevice, &DeviceClient::buttonStateChanged,
                [&](ButtonEvent* e)
                {
                    if (e->m_button == 1 && e->m_buttonState == BUTTON_RELEASED)
                    {
                        LOG(INFO) << "Release!";
                        grasping->endGrasp();
                    }
                });

            // Add default mouse and keyboard controls to the viewer
            std::shared_ptr<Entity> mouseAndKeyControls =
                SimulationUtils::createDefaultSceneControl(driver);
            auto instructText = mouseAndKeyControls->getComponent<TextVisualModel>();
            instructText->setText(instructText->getText() +
                "\nPress G to grasp" +
                "\nPress 1 to toggle gravity");
            scene->addSceneObject(mouseAndKeyControls);
        }

        driver->start();
    }
}

int
main()
{
    //probe1Example();
    probe2Example();
}