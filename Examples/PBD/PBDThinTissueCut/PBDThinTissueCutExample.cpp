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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkVertexLabelVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const double       width,
              const double       height,
              const int          nRows,
              const int          nCols)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e3);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e3);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

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
    tissueObj->setCollidingGeometry(clothMesh);
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->fixedNodeIds     = { 0, nCols - 1 };
    tissueObj->getPbdBody()->uniformMassValue = width * height / static_cast<double>(nRows * nCols);

    return tissueObj;
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

    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(40.0, 40.0), Vec2i(2, 2));
    cutGeom->setTranslation(Vec3d(-10, -20, 0));
    cutGeom->updatePostTransformData();
    auto cutObj = std::make_shared<CollidingObject>("CuttingObject");
    cutObj->setVisualGeometry(cutGeom);
    cutObj->setCollidingGeometry(cutGeom);
    cutObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cutObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(cutObj);

    // Create a pbd cloth object in the scene
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", 50.0, 50.0, 12, 12);
    scene->addSceneObject(tissueObj);

    // Add interaction pair for pbd cutting
    auto cutting = std::make_shared<PbdObjectCutting>(tissueObj, cutObj);
    scene->addInteraction(cutting);

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Create the virtual coupling object controller
    auto controller = std::make_shared<SceneObjectController>();
    controller->setControlledObject(cutObj);
    controller->setDevice(deviceClient);
    controller->setTranslationScaling(1000.0);
    controller->setTranslationOffset(Vec3d(0.0, 0.0, -100.0));
    scene->addControl(controller);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(1.0, 1.0, 1.0) * 100.0);
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, -50, 0));

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
        driver->setDesiredDt(0.002);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

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
        std::cout << "Haptic button 0 to cut the cloth.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
