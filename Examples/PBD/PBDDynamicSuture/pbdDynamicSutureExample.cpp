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

using namespace imstk;

// Create tissue object to stitch
std::shared_ptr<PbdObject>
createTissue(std::shared_ptr<PbdModel> model)
{
    // Load a tetrahedral mesh
    std::shared_ptr<TetrahedralMesh> tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "Tissues/tissue_hole.vtk");
    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    std::vector<int> fixedNodes;
    const int        numVerts = tetMesh->getNumVertices();
    for (int i = 0; i < numVerts; i++)
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
    auto pbdObject = std::make_shared<PbdObject>("meshHole");
    pbdObject->setVisualGeometry(surfMesh);
    pbdObject->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);;
    pbdObject->setPhysicsGeometry(tetMesh);
    pbdObject->setCollidingGeometry(surfMesh);
    pbdObject->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    pbdObject->setDynamicalModel(model);
    pbdObject->getPbdBody()->uniformMassValue = 0.2 / numVerts;
    std::cout<<"Tissue nodal mass = "<< 0.2 / numVerts<<"\n";
    // Fix the borders
    pbdObject->getPbdBody()->fixedNodeIds = fixedNodes;
    model->getConfig()->setBodyDamping(pbdObject->getPbdBody()->bodyHandle, 0.3);

    pbdObject->addComponent<Puncturable>();

    return pbdObject;
}

static std::shared_ptr<SceneObject>
makeClampObj(std::string name)
{
    auto surfMesh =
        MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Clamps/Gregory Suture Clamp/gregory_suture_clamp.obj");

    surfMesh->scale(5.0, Geometry::TransformType::ApplyToData);

    auto toolObj = std::make_shared<SceneObject>(name);
    toolObj->setVisualGeometry(surfMesh);
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(Color::LightGray);
    renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
    renderMaterial->setRoughness(0.5);
    renderMaterial->setMetalness(1.0);
    toolObj->getVisualModel(0)->setRenderMaterial(renderMaterial);

    return toolObj;
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
    auto stringObj = std::make_shared<PbdObject>(name);
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(model);
    stringObj->getPbdBody()->fixedNodeIds     = { 0, 1 };
    stringObj->getPbdBody()->uniformMassValue = 0.1 / numVerts; // 0.002 / numVerts; // grams
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 50000.0, stringObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableBendConstraint(0.2, 1, true, stringObj->getPbdBody()->bodyHandle);
    model->getConfig()->setBodyDamping(stringObj->getPbdBody()->bodyHandle, 0.3);

    return stringObj;
}

static std::shared_ptr<PbdObject>
makeToolObj(std::shared_ptr<PbdModel> model)
{
    auto needleObj      = std::make_shared<PbdObject>();
    auto sutureMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
    auto sutureLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));

    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureLineMesh->transform(rot, Geometry::TransformType::ApplyToData);

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
    needleObj->getPbdBody()->setRigid(Vec3d(0, 0, 0.1), 0.0007, Quatd::Identity(), Mat3d::Identity() * 10000.0);

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

    scene->getActiveCamera()->setPosition(0.0, 0.04, 0.09);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.02, 0.05);
    scene->getActiveCamera()->setViewUp(0.001, 1.0, -0.4);

    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Setup the Model
    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 5.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 20.0);
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 3;
    pbdModel->configure(pbdParams);

    // Mesh with hole for suturing
    std::shared_ptr<PbdObject> tissueHole = createTissue(pbdModel);
    scene->addSceneObject(tissueHole);

    // Create arced needle
    std::shared_ptr<PbdObject> needleObj = makeToolObj(pbdModel);
    scene->addSceneObject(needleObj);

    // Create the suture pbd-based string
    const double               stringLength      = 0.08;
    const int                  stringVertexCount = 47;
    std::shared_ptr<PbdObject> sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength, pbdModel);
    scene->addSceneObject(sutureThreadObj);

    // Add needle constraining behavior between the tissue & arc needle/thread
    auto sutureInteraction = std::make_shared<NeedleInteraction>(tissueHole, needleObj, sutureThreadObj);
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
        driver->setDesiredDt(0.005);

        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        auto hapController = std::make_shared<PbdObjectController>();
        hapController->setControlledObject(needleObj);
        hapController->setDevice(deviceClient);
        hapController->setTranslationScaling(0.5);
        hapController->setLinearKs(10.0);
        hapController->setAngularKs(100000000.0);
        hapController->setUseCritDamping(true);
        hapController->setForceScaling(10.0);
        hapController->setSmoothingKernelSize(10);
        hapController->setUseForceSmoothening(true);
        scene->addControl(hapController);

        // Add extra component to tool for the ghost
        auto controllerGhost = needleObj->addComponent<ObjectControllerGhost>();
        controllerGhost->setController(hapController);
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