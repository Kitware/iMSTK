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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVertexLabelVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string&        name,
              const double              width,
              const double              height,
              const int                 nRows,
              const int                 nCols,
              std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));
    clothMesh->translate(Vec3d(0.0, height * 0.5, width * 0.5),
        Geometry::TransformType::ApplyToData);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    auto vertexLabelModel = std::make_shared<VertexLabelVisualModel>();
    vertexLabelModel->setGeometry(clothMesh);
    vertexLabelModel->setFontSize(20.0);
    vertexLabelModel->setTextColor(Color::Red);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>();
    tissueObj->addVisualModel(visualModel);
    tissueObj->addVisualModel(vertexLabelModel);
    tissueObj->setPhysicsGeometry(clothMesh);
    tissueObj->addComponent<Collider>()->setGeometry(clothMesh);
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->fixedNodeIds     = { 0, nCols - 1 };
    tissueObj->getPbdBody()->uniformMassValue = 0.01;

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance,
        1e4, tissueObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral,
        0.1, tissueObj->getPbdBody()->bodyHandle);

    return tissueObj;
}

static std::shared_ptr<PbdObject>
makeToolObj(std::shared_ptr<PbdModel> model)
{
    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.03, 0.03), Vec2i(2, 2));
    cutGeom->updatePostTransformData();

    auto toolObj = std::make_shared<PbdObject>("CuttingObject");
    toolObj->setVisualGeometry(cutGeom);
    toolObj->addComponent<Collider>()->setGeometry(cutGeom);
    toolObj->setPhysicsGeometry(cutGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);

    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.0, 0.0, 0.0),         // Position
        1.0,                          // Mass
        Quatd::Identity(),            // Orientation
        Mat3d::Identity() * 10000.0); // Inertia

    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setLinearKs(20000.0);
    controller->setAngularKs(8000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.025);
    controller->setSmoothingKernelSize(10);
    controller->setUseForceSmoothening(true);

    return toolObj;
}

///
/// \brief This example demonstrates the concept of PBD cutting
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    auto scene = std::make_shared<Scene>("PbdThinTissueCut");
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.1, 0.3));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.02, 0.05));

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 5;
    //pbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
    pbdModel->getConfig()->m_gravity = Vec3d(0.0, -7.0, 0.0);

    std::shared_ptr<PbdObject> toolObj = makeToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // Create a pbd cloth object in the scene
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        0.1, 0.1, 12, 12,
        pbdModel);
    scene->addSceneObject(tissueObj);

    // Add cutting interaction
    auto cutting = std::make_shared<PbdObjectCutting>(tissueObj, toolObj);
    cutting->setEpsilon(0.001);
    scene->addInteraction(cutting);

    // Add collision interaction (order matters for MeshtoMesh)
    // Requires per element collision
    //scene->addInteraction(std::make_shared<PbdObjectCollision>(toolObj, tissueObj));

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
    auto                           controller    = toolObj->getComponent<PbdObjectController>();
    controller->setDevice(deviceClient);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(hapticManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Queue haptic button press to be called after scene thread
        queueConnect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
            {
                // When button 0 is pressed replace the PBD cloth with a cut one
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    cutting->apply();
                }
            });
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    cutting->apply();
                }
            });

        std::cout << "================================================\n";
        std::cout << "Haptic button 0 or key 'g' to cut the cloth.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
