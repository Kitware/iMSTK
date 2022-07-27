/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObjectController.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
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
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e3);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e3);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / static_cast<double>(nRows * nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    imstkNew<VertexLabelVisualModel> vertexLabelModel;
    vertexLabelModel->setGeometry(clothMesh);
    vertexLabelModel->setFontSize(20.0);
    vertexLabelModel->setTextColor(Color::Red);

    // Setup the Object
    imstkNew<PbdObject> tissueObj(name);
    tissueObj->addVisualModel(visualModel);
    tissueObj->addVisualModel(vertexLabelModel);
    tissueObj->setPhysicsGeometry(clothMesh);
    tissueObj->setCollidingGeometry(clothMesh);
    tissueObj->setDynamicalModel(pbdModel);

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
    imstkNew<Scene> scene("PbdThinTissueCut");

    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(40.0, 40.0), Vec2i(2, 2));
    cutGeom->setTranslation(Vec3d(-10, -20, 0));
    cutGeom->updatePostTransformData();
    imstkNew<CollidingObject> cutObj("CuttingObject");
    cutObj->setVisualGeometry(cutGeom);
    cutObj->setCollidingGeometry(cutGeom);
    cutObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cutObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(cutObj);

    // Create a pbd cloth object in the scene
    std::shared_ptr<PbdObject> clothObj = makeTissueObj("Tissue", 50.0, 50.0, 12, 12);
    scene->addSceneObject(clothObj);

    // Add interaction pair for pbd cutting
    imstkNew<PbdObjectCutting> cuttingInteraction(clothObj, cutObj);

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient();

    // Create the virtual coupling object controller
    imstkNew<SceneObjectController> controller;
    controller->setControlledObject(cutObj);
    controller->setDevice(client);
    scene->addControl(controller);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(1.0, 1.0, 1.0) * 100.0);
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, -50, 0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(server);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

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
        queueConnect<ButtonEvent>(client, &HapticDeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
            {
                // When button 0 is pressed replace the PBD cloth with a cut one
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    cuttingInteraction->apply();
                }
            });

        std::cout << "================================================\n";
        std::cout << "Haptic button 0 to cut the cloth.\n";
        std::cout << "================================================\n\n";

        driver->start();
    }
    return 0;
}
