/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollisionUtils.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include "imstkPbdObjectCellRemoval.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

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
    tissueMesh->scale(10.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { -0.4, 0.0, 0.0 };
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
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setDynamicalModel(model);
    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue = 60.0 / tissueMesh->getNumVertices();

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
        tissueObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 100.0,
        tissueObj->getPbdBody()->bodyHandle);

    tissueObj->getPbdBody()->fixedNodeIds = { 72, 57, 131, 132 };

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    tissueObj->initialize();

    return tissueObj;
}

static std::shared_ptr<PbdObject>
makeKidney(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Kidney/kidney_vol_low_rez.vtk");
    const Vec3d center     = tissueMesh->getCenter();

    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(10.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 1.0, 0.0), 90.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { 0.4, 0.0, 0.0 };
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tissueMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);

    tissueObj->addVisualModel(visualModel);
    //tissueObj->addVisualModel(labelModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setDynamicalModel(model);
    tissueObj->setCollidingGeometry(surfMesh);

    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue = 60.0 / tissueMesh->getNumVertices();
    tissueObj->getPbdBody()->fixedNodeIds     = { 72, 57, 131, 132 };

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
        tissueObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 500.0,
        tissueObj->getPbdBody()->bodyHandle);

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    return tissueObj;
}

///
/// \brief Creates simulated tool object with cutting plane
///
static std::shared_ptr<PbdObject>
makeToolObj(const std::string& name, std::shared_ptr<PbdModel> model, double shift)
{
    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.25, 0.25), Vec2i(2, 2));
    cutGeom->setTranslation(Vec3d(-1, 0, 0));
    cutGeom->rotate(Vec3d(0.0, 0.0, 1.0), (shift * 90.0) / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    cutGeom->translate(Vec3d(shift * 0.25, 0, 0), Geometry::TransformType::ApplyToData);

    cutGeom->updatePostTransformData();

    auto toolObj = std::make_shared<PbdObject>(name);
    toolObj->setVisualGeometry(cutGeom);
    toolObj->setPhysicsGeometry(cutGeom);
    toolObj->setCollidingGeometry(cutGeom);
    toolObj->setDynamicalModel(model);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);

    // Add a visual model to render the normals of the surface
    auto normalsVisualModel = std::make_shared<VisualModel>();
    normalsVisualModel->setGeometry(cutGeom);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.05);
    toolObj->addVisualModel(normalsVisualModel);

    toolObj->getPbdBody()->setRigid(Vec3d(0.0, 0.0, 0.0), 1.0);

    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setTranslationOffset(Vec3d(0.0, 0.0, 0.0));
    controller->setTranslationScaling(10.0);
    controller->setForceScaling(0.0);
    controller->setLinearKs(2000.0);
    controller->setAngularKs(500.0);
    // Damping doesn't work well here. The force is applied at the start of pbd
    // Because velocities are ulimately computed after the fact from positions
    controller->setUseCritDamping(true);

    return toolObj;
}

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdConnectiveTissue");
    scene->getActiveCamera()->setPosition(0.278448, 0.0904159, 3.43076);
    scene->getActiveCamera()->setFocalPoint(0.0703459, -0.539532, 0.148011);
    scene->getActiveCamera()->setViewUp(-0.0400007, 0.980577, -0.19201);

    // Setup the PBD Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 4;
    pbdModel->getConfig()->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdModel->getConfig()->m_linearDampingCoeff  = 0.001; // Removed from velocity
    pbdModel->getConfig()->m_angularDampingCoeff = 0.01;

    // Setup gallbladder object
    std::shared_ptr<PbdObject> gallbladerObj = makeGallBladder("Gallbladder", pbdModel);
    scene->addSceneObject(gallbladerObj);

    std::shared_ptr<PbdObject> kidneyObj = makeKidney("Kidney", pbdModel);
    scene->addSceneObject(kidneyObj);

    // Create PBD object of connective strands with associated constraints
    double maxDist = 0.6;
    auto   connectiveStrands = makeConnectiveTissue(gallbladerObj, kidneyObj, pbdModel, maxDist, 0.75);

    scene->addSceneObject(connectiveStrands);

    auto cellRemoval = std::make_shared<PbdObjectCellRemoval>(connectiveStrands);
    scene->addInteraction(cellRemoval);

    // Setup the tool with cutting plane
    std::shared_ptr<PbdObject> toolObj = makeToolObj("Tool", pbdModel, 0.0);
    scene->addSceneObject(toolObj);

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
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation pause

        auto driver = std::make_shared<SimulationManager>();
        driver->setDesiredDt(0.005);
        driver->addModule(viewer);
        driver->addModule(sceneManager);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        // Queue haptic button press to be called after scene thread
        queueConnect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
            {
                //  LOG(INFO) << "Button Press";
                // When button 0 is pressed replace the PBD cloth with a cut one
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    auto cutter  = std::dynamic_pointer_cast<SurfaceMesh>(toolObj->getPhysicsGeometry());
                    auto strands = std::dynamic_pointer_cast<LineMesh>(connectiveStrands->getCollidingGeometry());

                    for (int i = 0; i < cutter->getNumCells(); i++)
                    {
                        const Vec3d a = cutter->getVertexPosition(cutter->getCells()->at(i)[0]);
                        const Vec3d b = cutter->getVertexPosition(cutter->getCells()->at(i)[1]);
                        const Vec3d c = cutter->getVertexPosition(cutter->getCells()->at(i)[2]);

                        for (int strandId = 0; strandId < strands->getNumCells(); strandId++)
                        {
                            const Vec3d p = strands->getVertexPosition(strands->getCells()->at(strandId)[0]);
                            const Vec3d q = strands->getVertexPosition(strands->getCells()->at(strandId)[1]);

                            if (CollisionUtils::testSegmentTriangle(p, q, a, b, c))
                            {
                                cellRemoval->removeCellOnApply(strandId);
                            }
                        }
                    }

                    cellRemoval->apply();
                }
            });
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.1) * 0.5;

                deviceClient->setPosition(worldPos);
                deviceClient->setOrientation(Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0),
                    Vec3d(1.0, 0.0, 0.0)));
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    auto cutter  = std::dynamic_pointer_cast<SurfaceMesh>(toolObj->getPhysicsGeometry());
                    auto strands = std::dynamic_pointer_cast<LineMesh>(connectiveStrands->getCollidingGeometry());

                    for (int i = 0; i < cutter->getNumCells(); i++)
                    {
                        const Vec3d a = cutter->getVertexPosition(cutter->getCells()->at(i)[0]);
                        const Vec3d b = cutter->getVertexPosition(cutter->getCells()->at(i)[1]);
                        const Vec3d c = cutter->getVertexPosition(cutter->getCells()->at(i)[2]);

                        for (int strandId = 0; strandId < strands->getNumCells(); strandId++)
                        {
                            const Vec3d p = strands->getVertexPosition(strands->getCells()->at(strandId)[0]);
                            const Vec3d q = strands->getVertexPosition(strands->getCells()->at(strandId)[1]);

                            if (CollisionUtils::testSegmentTriangle(p, q, a, b, c))
                            {
                                cellRemoval->removeCellOnApply(strandId);
                            }
                        }
                    }
                    cellRemoval->apply();
                }
            });
#endif

        auto controller = toolObj->getComponent<PbdObjectController>();
        controller->setDevice(deviceClient);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
