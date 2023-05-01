/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkControllerForceText.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdRigidObjectGrasping.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkMeshIO.h"

#include "imstkImageData.h"

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
    tissueMesh->scale(1.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { -0.0, 0.0, 0.0 };
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

    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));

    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue =  0.6 / tissueMesh->getNumVertices();

    model->getConfig()->m_femParams->m_YoungModulus = 108000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean);
    model->getConfig()->setBodyDamping(tissueObj->getPbdBody()->bodyHandle, 0.01);

    // tissueObj->getPbdBody()->fixedNodeIds = { 57, 131, 132 }; // { 72, , 131, 132 };

    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = tissueMesh->getVertexPositions();
    for (int i = 0; i < tissueMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] >= 0.016)
        {
            tissueObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    tissueObj->initialize();

    return tissueObj;
}

///
/// \brief Creates pbd simulated cube mesh for testing
///
static std::shared_ptr<PbdObject>
makePbdObjCube(
    const std::string&        name,
    std::shared_ptr<PbdModel> model,
    const Vec3d&              size,
    const Vec3i&              dim,
    const Vec3d&              center)
{
    auto prismObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh  = prismMesh->extractSurfaceMesh();

    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setCollidingGeometry(surfMesh);
    prismObj->setVisualGeometry(surfMesh);
    prismObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    prismObj->setDynamicalModel(model);
    // prismObj->getPbdBody()->uniformMassValue = 0.05;
    prismObj->getPbdBody()->uniformMassValue = 0.003; // 0.06 / surfMesh->getNumVertices();

    prismObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(prismMesh, surfMesh));

    //model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance,
    //    1e4,
    //    prismObj->getPbdBody()->bodyHandle);
    //model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral,
    //    0.1,
    //    prismObj->getPbdBody()->bodyHandle);

    //prismObj->initialize();
    model->getConfig()->m_femParams->m_YoungModulus = 6000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean);
    model->getConfig()->setBodyDamping(prismObj->getPbdBody()->bodyHandle, 0.001);

    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMesh->getCells();
    const VecDataArray<int, 3>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
    const VecDataArray<double, 3>&           verts       = *verticesPtr;

    double avgArea = 0.0;
    for (int i = 0; i < surfMesh->getNumCells(); i++)
    {
        const Vec3i& cell = indices[i];
        const Vec3d& p0   = verts[cell[0]];
        const Vec3d& p1   = verts[cell[1]];
        const Vec3d& p2   = verts[cell[2]];
        avgArea += 0.5 * (p1 - p0).cross(p2 - p0).norm();
    }

    avgArea /= surfMesh->getNumCells();
    LOG(INFO) << "Average Cell Area = " << avgArea;
    LOG(INFO) << "Cell Characteristic Length = " << sqrt(avgArea);
    LOG(INFO) << "Per node mass = " << prismObj->getPbdBody()->uniformMassValue;

    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = prismMesh->getVertexPositions();
    for (int i = 0; i < prismMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            prismObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    return prismObj;
}

///
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string&        name,
              std::shared_ptr<PbdModel> model,
              const double              width,
              const double              height,
              const int                 rowCount,
              const int                 colCount)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> surfMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount));

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>(name);

    pbdObject->setVisualGeometry(surfMesh);
    pbdObject->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    pbdObject->setPhysicsGeometry(surfMesh);
    pbdObject->setCollidingGeometry(surfMesh);
    pbdObject->setDynamicalModel(model);
    pbdObject->getPbdBody()->uniformMassValue = 0.003; // 0.06 / surfMesh->getNumVertices();
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || y == colCount - 1) //|| x == rowCount - 1
            {
                pbdObject->getPbdBody()->fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }

    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr  = surfMesh->getCells();
    const VecDataArray<int, 3>&              indices     = *indicesPtr;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;

    double avgArea = 0.0;
    for (int i = 0; i < surfMesh->getNumCells(); i++)
    {
        const Vec3i& cell = indices[i];
        const Vec3d& p0   = vertices[cell[0]];
        const Vec3d& p1   = vertices[cell[1]];
        const Vec3d& p2   = vertices[cell[2]];
        avgArea += 0.5 * (p1 - p0).cross(p2 - p0).norm();
    }

    avgArea /= surfMesh->getNumCells();
    LOG(INFO) << "Average Cell Area = " << avgArea;
    LOG(INFO) << "Cell Characteristic Length = " << sqrt(avgArea);
    LOG(INFO) << "Per node mass = " << pbdObject->getPbdBody()->uniformMassValue;

    return pbdObject;
}

///
/// \brief Creates capsule to use as a tool
///
static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model)
{
    double radius = 0.005;
    double length = 0.2;
    double mass   = 0.02;

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
    controller->setTranslationScaling(1.0);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(1.0);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

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
    scene->getActiveCamera()->setPosition(0.00610397, 0.131126, 0.281497);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.00251247, 0.90946, -0.415783);

    auto                            pbdModel  = std::make_shared<PbdModel>();
    std::shared_ptr<PbdModelConfig> pbdParams = pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.002;
    pbdParams->m_iterations = 2;
    pbdParams->m_linearDampingCoeff = 0.03;
    /* pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
     pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e1);*/

    // Setup a gallbladder
    //std::shared_ptr<PbdObject> pbdObj = makeGallBladder("Gallbladder", pbdModel);
    //scene->addSceneObject(pbdObj);

    Vec3d                      size   = Vec3d(0.10, 0.08, 0.10);
    Vec3i                      dim    = Vec3i(18, 4, 18);
    Vec3d                      center = Vec3d(0.0, -0.05, 0.0);
    std::shared_ptr<PbdObject> pbdObj = makePbdObjCube("Cube", pbdModel, size, dim, center);
    scene->addSceneObject(pbdObj);

    // int resolution = 35;
    //std::shared_ptr<PbdObject> pbdObj = makeTissueObj("Plane", pbdModel, 0.15, 0.15, resolution, resolution);
    //scene->addSceneObject(pbdObj);

    // Setup a tool to grasp with
    std::shared_ptr<PbdObject> toolObj = makeCapsuleToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // Add collision
    auto pbdToolCollision = std::make_shared<PbdObjectCollision>(pbdObj, toolObj);
    pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    pbdToolCollision->setUseCorrectVelocity(true);
    scene->addInteraction(pbdToolCollision);

    // Create new picking with constraints
    auto toolPicking = std::make_shared<PbdObjectGrasping>(pbdObj, toolObj);
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
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.002);

        auto controller = toolObj->getComponent<PbdObjectController>();
        controller->setPosition(Vec3d(0.0, 0.0, 0.0));

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
                        toolPicking->beginVertexGrasp(dilatedCapsule);
                        pbdToolCollision->setEnabled(false);
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        toolPicking->endGrasp();
                        pbdToolCollision->setEnabled(true);
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

        // Alternative grasping by keyboard (in case device doesn't have a button)
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    auto capsule = std::dynamic_pointer_cast<Capsule>(toolObj->getCollidingGeometry());
                    auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                    dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                    toolPicking->beginVertexGrasp(dilatedCapsule);
                    pbdToolCollision->setEnabled(false);
                }
            });
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyRelease,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    toolPicking->endGrasp();
                    pbdToolCollision->setEnabled(true);
                }
            });
        controller->setDevice(deviceClient);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);

        // Add something to display controller force
        auto controllerForceTxt = mouseAndKeyControls->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
        controllerForceTxt->setCollision(pbdToolCollision);

        scene->addSceneObject(mouseAndKeyControls);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Simulate in real time
                pbdModel->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}