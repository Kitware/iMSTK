/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#endif

using namespace imstk;

///
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const double       width,
              const double       height,
              const int          rowCount,
              const int          colCount)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount));

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 5000.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 5000.0);
    pbdParams->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 2;
    pbdParams->m_linearDampingCoeff = 0.0;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>(name);
    pbdObject->addVisualModel(visualModel);
    pbdObject->setPhysicsGeometry(clothMesh);
    pbdObject->setCollidingGeometry(clothMesh);
    pbdObject->setDynamicalModel(pbdModel);
    pbdObject->getPbdBody()->uniformMassValue = width * height / (rowCount * colCount);
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || x == rowCount - 1 || y == colCount - 1)
            {
                pbdObject->getPbdBody()->fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }

    return pbdObject;
}

///
/// \brief This example demonstrates collision interaction with a 2d pbd
/// simulated tissue/membrane/cloth
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdThinTissueContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", 10.0, 10.0, 16, 16);
    scene->addSceneObject(tissueObj);

    // Setup the tool to press the tissue
    auto                    toolGeom = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 2.0, 0.0) };
    VecDataArray<int, 2>    cells    = { Vec2i(0, 1) };
    toolGeom->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(cells));

    auto toolObj = std::make_shared<CollidingObject>("Tool");
    toolObj->setVisualGeometry(toolGeom);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRecomputeVertexNormals(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    scene->addSceneObject(toolObj);

    // Add a collision interaction between the tools
    scene->addInteraction(std::make_shared<PbdObjectCollision>(tissueObj, toolObj));

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

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->setDesiredDt(0.005);
        driver->addModule(viewer);
        driver->addModule(sceneManager);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        Mat3d rotationalOffset = Mat3d::Identity();
        connect<Event>(sceneManager, SceneManager::preUpdate, [&](Event*)
            {
                const Quatd deviceOrientation = (Quatd(rotationalOffset) * deviceClient->getOrientation()).normalized();
                const Vec3d devicePosition    = (rotationalOffset * deviceClient->getPosition()) * 50.0 + Vec3d(0.0, 0.0, 0.0);
                toolGeom->setRotation(deviceOrientation);
                toolGeom->setTranslation(devicePosition);
                toolGeom->postModified();
            });
#else
        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 10.0 + Vec3d(0.5, 2.0, 0.5);

                toolGeom->setTranslation(worldPos);
                toolGeom->postModified();
            });
#endif

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
