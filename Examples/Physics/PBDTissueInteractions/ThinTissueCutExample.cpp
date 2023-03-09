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
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVertexLabelVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates thinTissue object
///
static std::shared_ptr<Entity>
makeTissueObj(const std::string&         name,
              const double               width,
              const double               height,
              const int                  nRows,
              const int                  nCols,
              std::shared_ptr<PbdSystem> system)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> thinTissueMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));
    thinTissueMesh->translate(Vec3d(0.0, height * 0.5, width * 0.5),
        Geometry::TransformType::ApplyToData);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    auto vertexLabelModel = std::make_shared<VertexLabelVisualModel>();
    vertexLabelModel->setGeometry(thinTissueMesh);
    vertexLabelModel->setFontSize(20.0);
    vertexLabelModel->setTextColor(Color::Red);

    // Setup the Object
    auto tissueObj   = SceneUtils::makePbdEntity(name, thinTissueMesh, system);
    auto visualModel = tissueObj->getComponent<VisualModel>();
    visualModel->setRenderMaterial(material);
    tissueObj->addComponent(vertexLabelModel);
    auto method = tissueObj->getComponent<PbdMethod>();
    method->setFixedNodes({ 0, nCols - 1 });
    method->setUniformMass(0.01);

    system->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance,
        1e4, method->getBodyHandle());
    system->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral,
        0.1, method->getBodyHandle());

    return tissueObj;
}

static std::shared_ptr<Entity>
makeToolObj(std::shared_ptr<PbdSystem> system)
{
    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.03, 0.03), Vec2i(2, 2));
    cutGeom->updatePostTransformData();

    auto toolObj  = SceneUtils::makePbdEntity("CuttingObject", cutGeom, system);
    auto material = toolObj->getComponent<VisualModel>()->getRenderMaterial();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);

    auto toolObjMethod = toolObj->getComponent<PbdMethod>();
    toolObjMethod->setPbdSystem(system);
    toolObjMethod->setRigid(
        Vec3d(0.0, 0.0, 0.0),         // Position
        1.0,                          // Mass
        Quatd::Identity(),            // Orientation
        Mat3d::Identity() * 10000.0); // Inertia

    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObjMethod, toolObj->getComponent<VisualModel>());
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
PBDThinTissueCutExample()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    auto scene = std::make_shared<Scene>("PbdThinTissueCut");
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.1, 0.3));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.02, 0.05));

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_doPartitioning = false;
    pbdSystem->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdSystem->getConfig()->m_iterations = 5;
    //pbdSystem->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
    pbdSystem->getConfig()->m_gravity = Vec3d(0.0, -7.0, 0.0);

    auto toolObj = makeToolObj(pbdSystem);
    scene->addSceneObject(toolObj);

    // Create a pbd thinTissue object in the scene
    auto tissueObj = makeTissueObj("Tissue",
        0.1, 0.1, 12, 12,
        pbdSystem);
    scene->addSceneObject(tissueObj);

    // Add cutting interaction
    auto cutting = std::make_shared<PbdObjectCutting>(tissueObj->getComponent<PbdMethod>(), toolObj->getComponent<Collider>());
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
                // When button 0 is pressed replace the PBD thinTissue with a cut one
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
        std::cout << "Haptic button 0 or key 'g' to cut the thinTissue.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
