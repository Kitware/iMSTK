/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
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
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectController.h"
#include "imstkPointwiseMap.h"
#include "imstkPuncturable.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleInteraction.h"

using namespace imstk;

// Create tissue object to stitch
std::shared_ptr<Entity>
createTissue(std::shared_ptr<PbdSystem> model)
{
    // Load a tetrahedral mesh
    std::shared_ptr<TetrahedralMesh> tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "Tissues/tissue_hole.vtk");
    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    std::vector<int> fixedNodes;
    for (int i = 0; i < tetMesh->getNumVertices(); i++)
    {
        const Vec3d& position = tetMesh->getVertexPosition(i);
        if (std::fabs(1.40984 - std::fabs(position[1])) <= 1E-4)
        {
            fixedNodes.push_back(i);
        }
    }

    tetMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    surfMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    surfMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    tetMesh->scale(0.018, Geometry::TransformType::ApplyToData); // 0.015
    surfMesh->scale(0.018, Geometry::TransformType::ApplyToData);

    surfMesh->computeVertexNormals();
    surfMesh->computeTrianglesNormals();

    // Setup the Object
    auto pbdObject = SceneUtils::makePbdEntity("meshHole", surfMesh, surfMesh, tetMesh, model);
    pbdObject->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);;
    auto method = pbdObject->getComponent<PbdMethod>();
    method->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    method->setUniformMass(0.01);
    // Fix the borders
    method->setFixedNodes(fixedNodes);
    model->getConfig()->setBodyDamping(method->getBodyHandle(), 0.3);

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 100.0, method->getBodyHandle());
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0, method->getBodyHandle());

    pbdObject->addComponent<Puncturable>();

    return pbdObject;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<Entity>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength,
    std::shared_ptr<PbdSystem> model)
{
    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(18.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto stringObj = SceneUtils::makePbdEntity(name, stringMesh, model);
    stringObj->getComponent<VisualModel>()->setRenderMaterial(material);
    auto stringMethod = stringObj->getComponent<PbdMethod>();
    stringMethod->setFixedNodes({ 0, 1 });
    stringMethod->setUniformMass(0.0001 / numVerts); // 0.002 / numVerts; // grams
    const int bodyHandle = stringMethod->getBodyHandle();
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 50.0, bodyHandle);
    model->getConfig()->enableBendConstraint(0.2, 1, true, bodyHandle);
    model->getConfig()->setBodyDamping(bodyHandle, 0.03);

    return stringObj;
}

static std::shared_ptr<Entity>
makeToolObj(std::shared_ptr<PbdSystem> pbdSystem)
{
    auto sutureMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
    auto sutureLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));

    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureLineMesh->transform(rot, Geometry::TransformType::ApplyToData);

    auto needleObj = SceneUtils::makePbdEntity("Needle", sutureMesh, sutureLineMesh, sutureLineMesh, pbdSystem);
    needleObj->getComponent<PbdMethod>()->setPhysicsToVisualMap(std::make_shared<IsometricMap>(sutureLineMesh, sutureMesh));
    needleObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 1.0, Quatd::Identity(), Mat3d::Identity() * 1.0);

    auto material = needleObj->getComponent<VisualModel>()->getRenderMaterial();
    material->setColor(Color(0.9, 0.9, 0.9));
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->setRoughness(0.5);
    material->setMetalness(1.0);

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

    // Construct the scene
    auto scene = std::make_shared<Scene>("DynamicSuture");

    scene->getActiveCamera()->setPosition(0.0, 0.04, 0.09);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.02, 0.05);
    scene->getActiveCamera()->setViewUp(0.001, 1.0, -0.4);

    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 6;
    pbdSystem->configure(pbdParams);

    // Mesh with hole for suturing
    auto tissueHole = createTissue(pbdSystem);
    scene->addSceneObject(tissueHole);

    // Create arced needle
    auto needleObj = makeToolObj(pbdSystem);
    scene->addSceneObject(needleObj);

    // Create the suture pbd-based string
    const double stringLength      = 0.12;
    const int    stringVertexCount = 70;
    auto         sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength, pbdSystem);
    scene->addSceneObject(sutureThreadObj);

    // Add needle constraining behaviour between the tissue & arc needle/thread
    auto sutureInteraction = std::make_shared<NeedleInteraction>(tissueHole, needleObj, sutureThreadObj);
    sutureInteraction->setRigidBodyCompliance(0.01); // Helps with smoothness
    scene->addInteraction(sutureInteraction);

    // Add thread CCD
    auto interactionCCDThread = std::make_shared<PbdObjectCollision>(sutureThreadObj, sutureThreadObj);
    // Very important parameter for stability of solver, keep lower than 1.0:
    interactionCCDThread->setDeformableStiffnessA(0.01);
    interactionCCDThread->setDeformableStiffnessB(0.01);
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
        driver->setDesiredDt(0.01);         // 1ms, 1000hz

        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        auto hapController = std::make_shared<PbdObjectController>();
        hapController->setControlledObject(needleObj->getComponent<PbdMethod>(), needleObj->getComponent<VisualModel>());
        hapController->setDevice(deviceClient);
        hapController->setTranslationScaling(0.5);
        hapController->setLinearKs(5000.0);
        hapController->setAngularKs(1000.0);
        hapController->setUseCritDamping(true);
        hapController->setForceScaling(0.001);
        hapController->setSmoothingKernelSize(15);
        hapController->setUseForceSmoothening(true);
        scene->addControl(hapController);

        // Update the needle opbject for real time
        auto sutureThreadSystem = sutureThreadObj->getComponent<PbdMethod>()->getPbdSystem();
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [ = ](Event*)
            {
                pbdSystem->getConfig()->m_dt = sceneManager->getDt();
            });

        // Constrain the first two vertices of the string to the needle
        auto needleMethod       = needleObj->getComponent<PbdMethod>();
        auto sutureThreadMethod = sutureThreadObj->getComponent<PbdMethod>();
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [ = ](Event*)
            {
                auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleMethod->getGeometry());
                auto sutureLineMesh = std::dynamic_pointer_cast<LineMesh>(sutureThreadMethod->getGeometry());
                (*sutureLineMesh->getVertexPositions())[1] = (*needleLineMesh->getVertexPositions())[0];
                (*sutureLineMesh->getVertexPositions())[0] = (*needleLineMesh->getVertexPositions())[1];
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