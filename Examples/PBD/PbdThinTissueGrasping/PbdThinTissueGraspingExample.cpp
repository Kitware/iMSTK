/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLaparoscopicToolController.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

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
    std::shared_ptr<SurfaceMesh> mesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount),
            Quatd::Identity(), 2.0);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10000.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.1);
    pbdParams->m_gravity    = Vec3d(0.0, -0.01, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 4;
    pbdParams->m_linearDampingCoeff = 0.01;

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

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->setVisualGeometry(mesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(mesh);
    tissueObj->setCollidingGeometry(mesh);
    tissueObj->setDynamicalModel(pbdModel);
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || x == rowCount - 1 || y == colCount - 1)
            {
                tissueObj->getPbdBody()->fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }
    tissueObj->getPbdBody()->uniformMassValue = 1.0;

    return tissueObj;
}

///
/// \brief This example demonstrates Pbd grasping. PbdObjectGrasping allows
/// us to hold onto parts of a tissue or other pbd deformable with a tool
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("PbdThinTissueGrasping");
    scene->getActiveCamera()->setPosition(0.001, 0.05, 0.15);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    auto geomShaft = std::make_shared<Capsule>();
    geomShaft->setLength(1.0);
    geomShaft->setRadius(0.005);
    geomShaft->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    geomShaft->setTranslation(Vec3d(0.0, 0.0, 0.5));
    auto objShaft = std::make_shared<CollidingObject>("ShaftObject");
    objShaft->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/pivot.obj"));
    objShaft->setCollidingGeometry(geomShaft);
    scene->addSceneObject(objShaft);

    auto geomUpperJaw = std::make_shared<Capsule>();
    geomUpperJaw->setLength(0.05);
    geomUpperJaw->setTranslation(Vec3d(0.0, 0.0013, -0.016));
    geomUpperJaw->setRadius(0.004);
    geomUpperJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    auto objUpperJaw = std::make_shared<CollidingObject>("UpperJawObject");
    objUpperJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/upper.obj"));
    objUpperJaw->setCollidingGeometry(geomUpperJaw);
    scene->addSceneObject(objUpperJaw);

    auto geomLowerJaw = std::make_shared<Capsule>();
    geomLowerJaw->setLength(0.05);
    geomLowerJaw->setTranslation(Vec3d(0.0, -0.0013, -0.016));
    geomLowerJaw->setRadius(0.004);
    geomLowerJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    auto objLowerJaw = std::make_shared<CollidingObject>("LowerJawObject");
    objLowerJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/lower.obj"));
    objLowerJaw->setCollidingGeometry(geomLowerJaw);
    scene->addSceneObject(objLowerJaw);

    auto pickGeom = std::make_shared<Capsule>();
    pickGeom->setLength(0.05);
    pickGeom->setTranslation(Vec3d(0.0, 0.0, -0.016));
    pickGeom->setRadius(0.006);
    pickGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));

    // 300mm x 300mm patch of tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", 0.1, 0.1, 16, 16);
    scene->addSceneObject(tissueObj);

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Create and add virtual coupling object controller in the scene
    auto controller = std::make_shared<LaparoscopicToolController>();
    controller->setParts(objShaft, objUpperJaw, objLowerJaw, pickGeom);
    controller->setDevice(deviceClient);
    controller->setJawAngleChange(1.0);
    scene->addControl(controller);

    // Add collision for both jaws of the tool
    auto upperJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objUpperJaw);
    auto lowerJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objLowerJaw);
    scene->addInteraction(upperJawCollision);
    scene->addInteraction(lowerJawCollision);

    // Add picking interaction for both jaws of the tool
    auto jawPicking = std::make_shared<PbdObjectGrasping>(tissueObj);
    scene->addInteraction(jawPicking);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(0.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(hapticManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

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

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                // Simulate the cloth in real time
                tissueObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        connect<Event>(controller, &LaparoscopicToolController::JawClosed,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Closed!";

                upperJawCollision->setEnabled(false);
                lowerJawCollision->setEnabled(false);
                jawPicking->beginCellGrasp(pickGeom, "SurfaceMeshToCapsuleCD");
            });
        connect<Event>(controller, &LaparoscopicToolController::JawOpened,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Opened!";

                upperJawCollision->setEnabled(true);
                lowerJawCollision->setEnabled(true);
                jawPicking->endGrasp();
            });

        driver->start();
    }

    return 0;
}
