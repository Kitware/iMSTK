/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNeedle.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPointwiseMap.h"
#include "imstkPuncturable.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkNeedleInteraction.h"
#include "imstkPbdObjectController.h"
#include "imstkLoggerSynchronous.h"

#include "imstkObjectControllerGhost.h"
#include "imstkCapsule.h"
#include "imstkPbdRigidObjectGrasping.h"

#include "imstkPbdDistanceConstraint.h"

using namespace imstk;

#include <iostream>

// Create tissue object to stitch
std::shared_ptr<PbdObject>
createTissue(std::shared_ptr<PbdModel> model)
{
    // Load a tetrahedral mesh
    std::shared_ptr<TetrahedralMesh> tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "Tissues/tissue_hole.vtk");
    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    const int numVerts = tetMesh->getNumVertices();

    tetMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    surfMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    surfMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    tetMesh->scale(0.02, Geometry::TransformType::ApplyToData); // 0.02
    surfMesh->scale(0.02, Geometry::TransformType::ApplyToData);

    double squish = 0.5;
    tetMesh->scale(Vec3d(1.0, squish, 1.0), Geometry::TransformType::ApplyToData);
    surfMesh->scale(Vec3d(1.0, squish, 1.0), Geometry::TransformType::ApplyToData);

    surfMesh->computeVertexNormals();
    surfMesh->computeTrianglesNormals();

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>("meshHole");
    pbdObject->setVisualGeometry(surfMesh);
    pbdObject->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);;
    pbdObject->setPhysicsGeometry(tetMesh);
    pbdObject->setCollidingGeometry(surfMesh);
    pbdObject->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    pbdObject->setDynamicalModel(model);
    double mass = 2.5;
    pbdObject->getPbdBody()->uniformMassValue = mass / numVerts; // 0.025
    std::cout << "Tissue nodal mass = " << mass / numVerts << "\n";
    pbdObject->addComponent<Puncturable>();
    pbdObject->initialize();

    std::cout << "Tissue body index = " << pbdObject->getPbdBody()->bodyHandle << "\n";

    model->getConfig()->m_femParams->m_YoungModulus = 8000.0;                                                                 // 8000
    model->getConfig()->m_femParams->m_PoissonRatio = 0.2;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean, pbdObject->getPbdBody()->bodyHandle); // StVK
    model->getConfig()->setBodyDamping(pbdObject->getPbdBody()->bodyHandle, 0.2);

    // Fix the borders (boundary conditions
    std::vector<int> fixedNodes;
    std::cout << "Num Verts = " << numVerts << "\n";
    for (int i = 0; i < numVerts; i++)
    {
        const Vec3d& position = tetMesh->getVertexPosition(i);
        if (std::fabs(0.0281968 - std::fabs(position[0])) <= 1E-4)
        {
            fixedNodes.push_back(i);
        }
    }

    // Fix nodes in place using constraints
    const int bodyId = pbdObject->getPbdBody()->bodyHandle;
    for (int nodeId = 0; nodeId < fixedNodes.size(); nodeId++)
    {
        const auto& position = tetMesh->getVertexPositions()->at(fixedNodes[nodeId]);

        const PbdParticleId& p0 = model->addVirtualParticle(position, 0.0, Vec3d::Zero(), true);
        const PbdParticleId  p1 = { bodyId, fixedNodes[nodeId] };

        auto   boundConstraint = std::make_shared<PbdDistanceConstraint>();
        double restLength      = 0.0;
        double stiffness       = 10.0;
        boundConstraint->initConstraint(restLength, p0, p1, stiffness);
        model->getConstraints()->addConstraint(boundConstraint);
    }

    return pbdObject;
}

///
/// \brief Creates capsule to use as a tool
///
static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model)
{
    double radius = 0.002; // Harry radius is 0.005
    double length = 0.2;   // Harry length is 1
    double mass   = 0.01;  // 0.1 (kg)

    auto toolGeometry = std::make_shared<Capsule>();
    // auto toolGeometry = std::make_shared<Sphere>();
    toolGeometry->setRadius(radius);
    toolGeometry->setLength(length);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    LOG(INFO) << "Tool Radius  = " << radius;
    LOG(INFO) << "Tool mass = " << mass;

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.04, 0.0, 0.0),
        mass,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(1.0);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setHapticOffset(Vec3d(0.0, 0.0, -0.1));
    controller->setTranslationScaling(1.0);
    controller->setLinearKs(50.0);
    controller->setAngularKs(1000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(1.0);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

static std::shared_ptr<CollidingObject>
makeCollidingCapsule()
{
    double radius = 0.009; // Harry radius is 0.005
    double length = 0.05;  // Harry length is 1

    auto capsuleGeometry = std::make_shared<Capsule>();
    // auto capsuleGeometry = std::make_shared<Sphere>();
    capsuleGeometry->setRadius(radius);
    capsuleGeometry->setLength(length);
    capsuleGeometry->setPosition(Vec3d(0.0, 0.0, -0.02));
    capsuleGeometry->setOrientation(Quatd(0.0, 0.0, 0.0, 0.0));

    auto capsuleObject = std::make_shared<CollidingObject>("Esophagus");

    capsuleObject->setCollidingGeometry(capsuleGeometry);
    capsuleObject->setVisualGeometry(capsuleGeometry);

    return capsuleObject;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength,
    std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    double segLength = (stringMesh->getVertexPositions()->at(0) - stringMesh->getVertexPositions()->at(1)).norm();
    std::cout << "Segment length = " << segLength << "\n"; // 0.0016
    std::cout << "Critical length = " << 0.0016 << "\n";

    const Vec3d shift = Vec3d(0.0, 0.05, 0.0);
    stringMesh->translate(shift, Geometry::TransformType::ApplyToData);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(3.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto stringObj = std::make_shared<PbdObject>(name);
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(model);
    //stringObj->getPbdBody()->fixedNodeIds     = { 0, 1 };
    stringObj->getPbdBody()->uniformMassValue = 0.00125;
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1E12, stringObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableBendConstraint(100, 1, true, stringObj->getPbdBody()->bodyHandle);
    model->getConfig()->setBodyDamping(stringObj->getPbdBody()->bodyHandle, 0.05);

    return stringObj;

    // Harrys thread
    // 201 verts
    // total length of 0.75
    // mass = 0.1
    // bend constraints stiffness 2 with stride of 2
}

static std::shared_ptr<PbdObject>
makeNeedleObj(std::shared_ptr<PbdModel> model)
{
    auto needleObj      = std::make_shared<PbdObject>();
    auto sutureMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
    auto sutureLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));

    const Vec3d center = sutureLineMesh->getCenter();
    sutureMesh->translate(-center, Geometry::TransformType::ApplyToData);
    sutureLineMesh->translate(-center, Geometry::TransformType::ApplyToData);

    const Vec3d shift = Vec3d(0.0, 0.05, -0.1);

    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureMesh->translate(shift, Geometry::TransformType::ApplyToData);

    sutureLineMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureLineMesh->translate(shift, Geometry::TransformType::ApplyToData);

    needleObj->setVisualGeometry(sutureMesh);
    // setVisualGeometry(sutureLineMesh);
    needleObj->setCollidingGeometry(sutureLineMesh);
    needleObj->setPhysicsGeometry(sutureLineMesh);
    needleObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(sutureLineMesh, sutureMesh));

    needleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    needleObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    needleObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    needleObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);

    needleObj->setDynamicalModel(model);
    needleObj->getPbdBody()->setRigid(Vec3d(0, 0, 0.1), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.00012);
    // needle mass 0.1
    // inertia is 0.00012

    needleObj->addComponent<Needle>();

    return needleObj;
}

///
/// \brief This example demonstrates suturing of a hole in a tissue
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();
    // Logger::instance()->setOutput(std::make_shared<StreamOutput>(std::cout));

    // Construct the scene
    auto scene = std::make_shared<Scene>("DynamicSuture");

    scene->getActiveCamera()->setPosition(0.001, 0.1, 0.145);
    scene->getActiveCamera()->setFocalPoint(0.000, 0.025, 0.035);
    scene->getActiveCamera()->setViewUp(0.0, 0.83, -0.55);

    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Setup the Model
    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity        = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt             = 0.001;
    pbdParams->m_iterations     = 5;
    pbdParams->m_doPartitioning = false;
    pbdModel->configure(pbdParams);

    // Mesh with hole for suturing
    std::shared_ptr<PbdObject> tissueHole = createTissue(pbdModel);
    scene->addSceneObject(tissueHole);

    // Capsule for esophagus
    auto esophagus = makeCollidingCapsule();
    scene->addSceneObject(esophagus);

    // Create arced needle
    std::shared_ptr<PbdObject> needleObj = makeNeedleObj(pbdModel);
    scene->addSceneObject(needleObj);

    // Create the thread
    const double               stringLength      = 0.15;
    const int                  stringVertexCount = 65;
    std::shared_ptr<PbdObject> sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength, pbdModel);
    scene->addSceneObject(sutureThreadObj);

    // Setup a tool to grasp with
    std::shared_ptr<PbdObject> toolObj = makeCapsuleToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // Create grasping interactions
    auto needleGrasp = std::make_shared<PbdObjectGrasping>(needleObj, toolObj);
    needleGrasp->setStiffness(0.3);
    scene->addInteraction(needleGrasp);

    auto threadGrasp = std::make_shared<PbdObjectGrasping>(sutureThreadObj, toolObj);
    threadGrasp->setStiffness(0.3);
    scene->addInteraction(threadGrasp);

    // Add tool to tissue collision
    auto pbdToolCollision = std::make_shared<PbdObjectCollision>(tissueHole, toolObj);
    pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    pbdToolCollision->setUseCorrectVelocity(true);
    scene->addInteraction(pbdToolCollision);

    // Add esophagus to tissue, needle, and tool collision
    auto esophagusTissueCollision = std::make_shared<PbdObjectCollision>(tissueHole, esophagus);
    esophagusTissueCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    esophagusTissueCollision->setUseCorrectVelocity(true);
    scene->addInteraction(esophagusTissueCollision);

    auto esophagusToolCollision = std::make_shared<PbdObjectCollision>(toolObj, esophagus);
    esophagusToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    esophagusToolCollision->setUseCorrectVelocity(true);
    scene->addInteraction(esophagusToolCollision);

    // Add needle constraining behavior between the tissue & arc needle/thread
    auto sutureInteraction = std::make_shared<NeedleInteraction>(tissueHole, needleObj, sutureThreadObj);
    scene->addInteraction(sutureInteraction);

    // Add thread CCD
    auto interactionCCDThread = std::make_shared<PbdObjectCollision>(sutureThreadObj, sutureThreadObj);
    // Very important parameter for stability of solver, keep lower than 1.0:
    interactionCCDThread->setDeformableStiffnessA(0.1);
    interactionCCDThread->setDeformableStiffnessB(0.1);
    scene->addInteraction(interactionCCDThread);

    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        // Setup a simulation manager to manage renders & scene updates
        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        auto controller = toolObj->getComponent<PbdObjectController>();
        controller->setDevice(deviceClient);

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
                        needleGrasp->beginCellGrasp(dilatedCapsule);
                        threadGrasp->beginCellGrasp(dilatedCapsule);
                        // pbdToolCollision->setEnabled(false);
                        std::cout << "Grasp\n";
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        needleGrasp->endGrasp();
                        threadGrasp->endGrasp();
                        // pbdToolCollision->setEnabled(true);
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
                pbdToolCollision->setEnabled(false);
            });
        connect<Event>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](Event*)
            {
                toolPicking->endGrasp();
                pbdToolCollision->setEnabled(true);
            });
#endif

        // Update the needle object for real time
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                sutureThreadObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                // Perform stitch
                if (e->m_key == 's')
                {
                    sutureInteraction->stitch();
                }
            });

        driver->start();
    }
    return 0;
}