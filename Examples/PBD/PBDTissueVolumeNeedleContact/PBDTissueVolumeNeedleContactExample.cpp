/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkControllerForceText.h"
#include "imstkDebugGeometryModel.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPointwiseMap.h"
#include "imstkPuncturable.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkStraightNeedle.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleEmbedder.h"
#include "NeedleInteraction.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Given a child mesh, find all the vertices of the parent that
/// are coincident to the child.
///
static std::vector<int>
computeFixedPtsViaMap(std::shared_ptr<PointSet> parent,
                      std::shared_ptr<PointSet> child,
                      const double              tolerance = 0.00001)
{
    std::vector<int> fixedPts;

    auto map = std::make_shared<PointwiseMap>();
    map->setParentGeometry(parent);
    map->setChildGeometry(child);
    map->setTolerance(tolerance);
    map->compute();
    fixedPts.reserve(child->getNumVertices());
    for (int i = 0; i < child->getNumVertices(); i++)
    {
        fixedPts.push_back(map->getParentVertexId(i));
    }
    return fixedPts;
}

///
/// \brief Creates PBD tetrahedral simulated tissue
/// \param name The name of the object
/// \param model The DynamicalModel to use
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string&               name,
              std::shared_ptr<PbdModel>        model,
              std::shared_ptr<TetrahedralMesh> tissueMesh)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->setVisualGeometry(surfMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->addComponent<Collider>()->setGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.04;

    // \todo: iMSTK doesn't support multiple different materials for FEM tet constraints without
    // making the functor yourself
    auto functor = std::make_shared<PbdFemTetConstraintFunctor>();
    functor->setGeometry(tissueMesh);
    functor->setBodyIndex(tissueObj->getPbdBody()->bodyHandle);
    const double youngsModulus    = 100000.0;
    const double poissonRatio     = 0.48;
    auto         constraintConfig = std::make_shared<PbdFemConstraintConfig>(
        youngsModulus / 2.0 / (1.0 + poissonRatio),
        youngsModulus * poissonRatio / ((1.0 + poissonRatio) * (1.0 - 2.0 * poissonRatio)),
        youngsModulus,
        poissonRatio);
    functor->setFemConfig(constraintConfig);
    functor->setMaterialType(PbdFemConstraint::MaterialType::StVK);
    model->getConfig()->addPbdConstraintFunctor(functor);

    tissueObj->addComponent<Puncturable>();

    return tissueObj;
}

static std::shared_ptr<PbdObject>
makeNeedleObj(const std::string&        name,
              std::shared_ptr<PbdModel> model)
{
    auto toolObj = std::make_shared<PbdObject>(name);

    auto toolGeometry = std::make_shared<LineMesh>();
    auto verticesPtr  = std::make_shared<VecDataArray<double, 3>>(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.0, 0.25);
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    auto trocarMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/LapTool/trocar.obj");

    toolObj->setVisualGeometry(trocarMesh);
    toolObj->addComponent<Collider>()->setGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeometry, trocarMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.0, 1.0, 0.0),         // Position
        1.0,                          // Mass
        Quatd::Identity(),            // Orientation
        Mat3d::Identity() * 10000.0); // Inertia

    // Add a component for needle puncturing
    auto needle = toolObj->addComponent<StraightNeedle>();
    needle->setNeedleGeometry(toolGeometry);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setLinearKs(20000.0);
    controller->setAngularKs(8000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.05);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setUseForceFade(true);
    controllerGhost->setController(controller);

    return toolObj;
}

static void
updateDebugGeom(std::shared_ptr<NeedleInteraction>  interaction,
                std::shared_ptr<DebugGeometryModel> debugGeomObj)
{
    auto                      needleEmbedder     = std::dynamic_pointer_cast<NeedleEmbedder>(interaction->getEmbedder());
    const std::vector<Vec3d>& debugEmbeddingPts  = needleEmbedder->m_debugEmbeddingPoints;
    const std::vector<Vec3i>& debugEmbeddingTris = needleEmbedder->m_debugEmbeddedTriangles;
    debugGeomObj->clear();
    for (size_t i = 0; i < debugEmbeddingPts.size(); i++)
    {
        debugGeomObj->addPoint(debugEmbeddingPts[i]);
    }
    std::shared_ptr<PbdObject> tissueObj   = interaction->getEmbedder()->getTissueObject();
    auto                       verticesPtr = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry())->getVertexPositions();
    VecDataArray<double, 3>&   vertices    = *verticesPtr;
    for (size_t i = 0; i < debugEmbeddingTris.size(); i++)
    {
        debugGeomObj->addTriangle(
            vertices[debugEmbeddingTris[i][0]],
            vertices[debugEmbeddingTris[i][1]],
            vertices[debugEmbeddingTris[i][2]]);
    }
}

///
/// \brief This example demonstrates two-way linear tissue needle contact
/// with a tetrahedral mesh. No torques rendered. Constraints are used at
/// the tetrahedrons faces of intersection.
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueVolumeNeedleContact");
    scene->getActiveCamera()->setPosition(0.0, 0.412873, 0.102441);
    scene->getActiveCamera()->setFocalPoint(0.0, -0.0, -0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.242952, -0.969977);
    scene->getConfig()->debugCamBoundingBox = false;
    *scene->getCamera("debug") = *scene->getActiveCamera();

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.001;     // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 1; // Prefer small timestep over iterations
    pbdModel->getConfig()->m_gravity    = Vec3d(0.0, 0.0, 0.0);

    // Setup a tissue with surface collision geometry
    const Vec3i dim = Vec3i(6, 3, 6);
    auto        tetGridMesh = GeometryUtils::toTetGrid(
        Vec3d(0.0, 0.0, 0.0),  // Center
        Vec3d(0.2, 0.01, 0.2), // Size (meters)
        dim);                  // Dimensions
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("PbdTissue1", pbdModel, tetGridMesh);
    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }
    scene->addSceneObject(tissueObj);

    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Kidney/kidney_vol_low_rez.vtk");
    tetMesh->translate(Vec3d(0.0, -0.07, -0.05), Geometry::TransformType::ApplyToData);
    std::shared_ptr<PbdObject> tissueObj2  = makeTissueObj("PbdTissue2", pbdModel, tetMesh);
    auto                       fixedPtMesh = MeshIO::read<PointSet>(iMSTK_DATA_ROOT "/Organs/Kidney/kidney_fixedpts_low_rez.obj");
    fixedPtMesh->translate(Vec3d(0.0, -0.07, -0.05), Geometry::TransformType::ApplyToData);
    tissueObj2->getPbdBody()->fixedNodeIds = computeFixedPtsViaMap(tetMesh, fixedPtMesh, 0.001);
    tissueObj2->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blood);
    scene->addSceneObject(tissueObj2);

    // Setup a tool for the user to move
    std::shared_ptr<PbdObject> toolObj   = makeNeedleObj("PbdNeedle", pbdModel);
    auto                       debugGeom = toolObj->addComponent<DebugGeometryModel>();
    debugGeom->setLineWidth(0.1);
    scene->addSceneObject(toolObj);

    // This adds both contact and puncture functionality
    auto interaction = std::make_shared<NeedleInteraction>(tissueObj, toolObj);
    interaction->setPunctureForceThreshold(3.0);
    interaction->setNeedleCompliance(0.000001);
    interaction->setFriction(0.1);
    scene->addInteraction(interaction);
    // This adds both contact and puncture functionality
    auto interaction2 = std::make_shared<NeedleInteraction>(tissueObj2, toolObj);
    interaction2->setPunctureForceThreshold(3.0);
    interaction2->setNeedleCompliance(0.000001);
    interaction2->setFriction(0.1);
    scene->addInteraction(interaction2);

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
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

        auto controller = toolObj->getComponent<PbdObjectController>();
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            controller->setTranslationOffset(Vec3d(0.125, -0.07, 0.0));
        }
#else
        auto deviceClient = std::make_shared<DummyClient>();

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;
                const Quatd desiredOrientation = Quatd(Rotd(0.0, Vec3d(1.0, 0.0, 0.0)));

                deviceClient->setPosition(desiredPos);
                deviceClient->setOrientation(desiredOrientation);
            });
#endif
        controller->setDevice(deviceClient);

        int counter = 0;
        connect<Event>(viewer, &VTKViewer::preUpdate,
            [&](Event*)
            {
                // Copy constraint faces and points to debug geometry for display
                updateDebugGeom(interaction, debugGeom);
            });
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Keep the tool moving in real time
                pbdModel->getConfig()->m_dt = sceneManager->getDt();
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        // Add something to display controller force
        auto controllerForceTxt = mouseAndKeyControls->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
        controllerForceTxt->setCollision(interaction);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}