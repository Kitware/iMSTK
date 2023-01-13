/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBurnable.h"
#include "imstkBurner.h"
#include "imstkCamera.h"
#include "imstkCapsule.h"
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
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
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

#include <iostream>

using namespace imstk;

/*
This example simulates connective tissue by connecting a gallbladder to a kidney.
The gallbladder is deformable with strain energy constraints and the kidney is treated as
rigid by setting all of the nodes to be fixed.

The units for this example are centimeters, kilograms, and seconds
*/

///
/// \brief Creates pbd simulated gallbladder object
///
std::shared_ptr<PbdObject>
makeGallBladder(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Gallblader/gallblader.msh");
    const Vec3d center     = tissueMesh->getCenter();
    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(100.0, Geometry::TransformType::ApplyToData); // scale from meters (input mesh) to cm
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { -4.0, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->addVisualModel(visualModel);
    //tissueObj->addVisualModel(labelModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setDynamicalModel(model);

    // Create map for collisions
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));

    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue = 0.06 / tissueMesh->getNumVertices();

    model->getConfig()->m_femParams->m_YoungModulus = 100.0; // in kg/(cm*s^2)
    model->getConfig()->m_femParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    model->getConfig()->setBodyDamping(tissueObj->getPbdBody()->bodyHandle, 0.01);

    // Fix the top of the gallbladder
    std::shared_ptr<VecDataArray<double, 3>> vertices = tissueMesh->getVertexPositions();
    for (int i = 0; i < tissueMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] >= 1.7)
        {
            tissueObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    tissueObj->initialize();

    return tissueObj;
}

///
/// \brief Creates pbd simulated rigid kidney object
///
static std::shared_ptr<PbdObject>
makeKidney(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Kidney/kidney_vol_low_rez.vtk");
    const Vec3d center     = tissueMesh->getCenter();

    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(100.0, Geometry::TransformType::ApplyToData); // scale from meters (input mesh) to cm
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 1.0, 0.0), 90.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { 4.0, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);

    tissueObj->addVisualModel(visualModel);
    //tissueObj->addVisualModel(labelModel);
    tissueObj->setPhysicsGeometry(surfMesh);
    tissueObj->setDynamicalModel(model);
    tissueObj->setCollidingGeometry(surfMesh);

    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue = 0.06 / surfMesh->getNumVertices();

    // Fix the kidney in position
    std::shared_ptr<VecDataArray<double, 3>> vertices = surfMesh->getVertexPositions();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        tissueObj->getPbdBody()->fixedNodeIds.push_back(i);
    }

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    return tissueObj;
}

///
/// \brief Creates pbd simulated capsule to use as a tool
///
static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.4);
    toolGeometry->setLength(4.0);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.0, 2.0, 2.0),
        0.01,
        Quatd::Identity(),
        Mat3d::Identity() * 100000.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(1.0);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setTranslationScaling(100.0); // this convertes from meters to cm
    controller->setLinearKs(1000.0);          // in N/cm
    controller->setAngularKs(1000000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.01); // 1 N = 1 kg/(m*s^2) = 0.01 kg/(cm*s^2)
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
    scene->getActiveCamera()->setPosition(0.944275, 8.47551, 21.4164);
    scene->getActiveCamera()->setFocalPoint(-0.450427, 0.519797, 0.817356);
    scene->getActiveCamera()->setViewUp(-0.0370536, 0.933044, -0.357851);

    // Setup the PBD Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.001;
    pbdModel->getConfig()->m_iterations = 6;
    pbdModel->getConfig()->m_gravity    = Vec3d(0.0, -981.0, 0.0); //in cm/s^2
    pbdModel->getConfig()->m_linearDampingCoeff  = 0.005;          // Removed from velocity
    pbdModel->getConfig()->m_angularDampingCoeff = 0.005;

    // Setup gallbladder object
    std::shared_ptr<PbdObject> gallbladerObj = makeGallBladder("Gallbladder", pbdModel);
    scene->addSceneObject(gallbladerObj);

    // Setup kidney
    std::shared_ptr<PbdObject> kidneyObj = makeKidney("Kidney", pbdModel);
    scene->addSceneObject(kidneyObj);

    // Create PBD object of connective strands with associated constraints
    double                     maxDist = 3.5;
    std::shared_ptr<PbdObject> connectiveStrands = makeConnectiveTissue(gallbladerObj, kidneyObj, pbdModel, maxDist, 2.5, 7);
    pbdModel->getConfig()->setBodyDamping(connectiveStrands->getPbdBody()->bodyHandle, 0.015, 0.0);

    // Add Tearing
    connectiveStrands->addComponent<Tearable>();

    // Add burnable
    auto burnable = std::make_shared<Burnable>();
    connectiveStrands->addComponent(burnable);

    // Add strands to scene
    scene->addSceneObject(connectiveStrands);

    // Setup a tool to grasp with
    std::shared_ptr<PbdObject> toolObj = makeCapsuleToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // add collisions
    auto strandCollision = std::make_shared<PbdObjectCollision>(connectiveStrands, toolObj);
    scene->addInteraction(strandCollision);

    auto gallCollision = std::make_shared<PbdObjectCollision>(gallbladerObj, toolObj);
    scene->addInteraction(gallCollision);

    // Create new picking with constraints
    auto grasper = std::make_shared<PbdObjectGrasping>(connectiveStrands, toolObj);
    grasper->setStiffness(0.5);
    scene->addInteraction(grasper);

    auto grasper_gall = std::make_shared<PbdObjectGrasping>(gallbladerObj, toolObj);
    grasper_gall->setStiffness(0.5);
    scene->addInteraction(grasper_gall);

    // Add burner component to tool
    auto burning = std::make_shared<Burner>();
    burning->addObject(connectiveStrands);

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
        viewer->setDebugAxesLength(1.0, 1.0, 1.0);

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
                        auto capsule = std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        grasper->beginCellGrasp(dilatedCapsule);
                        grasper_gall->beginCellGrasp(dilatedCapsule);

                        gallCollision->setEnabled(false);

                        std::cout << "Grasping! \n";
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        grasper->endGrasp();
                        grasper_gall->endGrasp();
                        gallCollision->setEnabled(true);

                        std::cout << "Released! \n";
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
                grasper->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry()));
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
                if (keyDevice->getButton('g') == KEY_PRESS && grasper->getGraspState() == false && grasper_gall->getGraspState() == false)
                {
                    // Use a slightly larger capsule since collision prevents intersection
                    auto capsule = std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry());
                    auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                    dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                    grasper->beginCellGrasp(dilatedCapsule);
                    grasper_gall->beginCellGrasp(dilatedCapsule);

                    std::cout << "Grasping! \n";
                }
                if (keyDevice->getButton('g') == KEY_RELEASE && grasper->getGraspState() == true && grasper_gall->getGraspState() == true)
                {
                    grasper->endGrasp();
                    grasper_gall->endGrasp();

                    std::cout << "Released! \n";
                }
            });

        driver->start();
    }

    return 0;
}
