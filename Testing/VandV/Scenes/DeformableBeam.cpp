/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"

#include "imstkScene.h"
#include "imstkSceneManager.h"

#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"

#include "imstkVTKViewer.h"

#include "imstkProgrammableClient.h"
#include "imstkSceneObjectToVTKMB.h"

#include "imstkPbdObject.h"
#include "imstkGeometryUtilities.h"

#include "imstkCollider.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkProgrammableClient.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkVisualModel.h"
#include "imstkOrientedBox.h"
#include "imstkSelectEnclosedPoints.h"

using namespace imstk;

static std::shared_ptr<PbdObject>
makePbdBeam(
    const std::string&        name,
    std::shared_ptr<PbdModel> model,
    const Vec3d&              size,
    const Vec3i&              dim,
    const Vec3d&              center)
{
    auto beam = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh  = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Object
    beam->setPhysicsGeometry(surfMesh);
    beam->addComponent<Collider>()->setGeometry(surfMesh);
    beam->setVisualGeometry(surfMesh);
    beam->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    beam->setDynamicalModel(model);
    beam->getPbdBody()->uniformMassValue = 0.05;
    // Use dihedral+distance constraints, worse results. More performant (can use larger mesh)
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1000.0,
    beam->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
    beam->getPbdBody()->bodyHandle);
    // Set bottom verts to be static
    std::shared_ptr<VecDataArray<double, 3>> vertices = surfMesh->getVertexPositions();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            beam->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    return beam;
}

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    std::vector<std::shared_ptr<ProgrammableClient>> vPC;
    auto                                             scene = std::make_shared<Scene>("PbdProgrammedMovement");

    double                          dt        = 0.002;
    auto                            pbdModel  = std::make_shared<PbdModel>();
    std::shared_ptr<PbdModelConfig> pbdParams = pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = dt;
    pbdParams->m_iterations = 1;
    pbdParams->m_linearDampingCoeff  = 0.005;
    pbdParams->m_angularDampingCoeff = 0.005;
    pbdParams->m_doPartitioning      = false;

    // auto box = makeBoxToolObj(pbdModel);
    auto beam = makePbdBeam("Beam",
      pbdModel,
      Vec3d(3.0, 1.0, 1.0),  // Dimensions
      Vec3i(3, 3, 3),        // Divisions
      Vec3d(0.0, 0.0, 0.0)); // Center
    scene->addSceneObject(beam);

    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(0.002);

    std::vector<Vec3d> vertexPos;
    vertexPos.push_back(Vec3d(1.5, .5, 0));
    vertexPos.push_back(Vec3d(1.5, .5, .5));
    vertexPos.push_back(Vec3d(1.5, .5, -.5));

    std::shared_ptr<TetrahedralMesh> tetMesh  = GeometryUtils::toTetGrid(Vec3d(1.5, 0.0, 0.0), Vec3d(1, 1, 2), Vec3i(3, 3, 3));
    std::shared_ptr<SurfaceMesh>     surfMesh = tetMesh->extractSurfaceMesh();

    auto mesh = std::dynamic_pointer_cast<PointSet>(beam->getPhysicsGeometry());

    auto selector = SelectEnclosedPoints();
    selector.setInputMesh(surfMesh);
    selector.setInputPoints(mesh);
    selector.update();
    auto output = selector.getOutputPoints();

    std::cout << output->getNumVertices() << std::endl;

    std::vector<int> ids;
    for (int i = 0; i < output->getNumVertices(); i++)
    {
        for (int j = 0; j < mesh->getNumVertices(); j++)
        {
            if (output->getInitialVertexPosition(i).isApprox(mesh->getInitialVertexPosition(j)))
            {
                ids.push_back(j);
            }
        }
    }

    std::vector<bool> pin{ true, true, true };

    deviceClient->addLinearVertexMovement(beam, ids,
                                            Vec3d(0, 1, 0.0),
                                            pin,
                                            0, 5);
    deviceClient->addWaitCommand(5, 10);

    auto meshConverter = std::make_shared<SceneObjectToVTKMB>();
    meshConverter->setTimeSeriesFilePath("./paraview_series");
    meshConverter->addEntity(beam);
    meshConverter->setTimeBetweenRecordings(0.1);

    // Camera
    scene->getActiveCamera()->setPosition(0.0, 3.0, 5.0);
    scene->getActiveCamera()->setFocalPoint(0.0, 1.0, 0.0);
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Viewer
    auto viewer = std::make_shared<VTKViewer>();
    viewer->setActiveScene(scene);
    viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);

    // Setup a scene manager to advance the scene
    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene);

    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.002);

    // Add default mouse and keyboard controls to the viewer
    std::shared_ptr<Entity> mouseAndKeyControls =
        SimulationUtils::createDefaultSceneControl(driver);
    scene->addSceneObject(mouseAndKeyControls);

    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            meshConverter->recordObjectState(driver->getDt());
            deviceClient->update();
            // std::cout << std::dynamic_pointer_cast<SurfaceMesh>(beam->getPhysicsGeometry())->getVertexPosition(15) << std::endl;
            if (deviceClient->isFinished())
            {
                driver->requestStatus(ModuleDriverStopped);
            }
    });

    connect<Event>(driver, &SimulationManager::ending, [&](Event*)
        {
            meshConverter->writeTimeSeriesJsonFile();
            meshConverter->writeObjectsToFile("./paraview.vtm");
    });

    driver->start();

    return 0;
}