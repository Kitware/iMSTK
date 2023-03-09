/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdSystem.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Create pbd string object
///
static std::shared_ptr<Entity>
makePbdString(const std::string& name, const std::string& filename)
{
    // Setup the Geometry
    auto stringMesh = MeshIO::read<LineMesh>(filename);

    const int numVerts = stringMesh->getNumVertices();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdSystemConfig>();
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.0005;
    pbdParams->m_iterations = 1;
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(4.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Setup the Object
    auto stringObj = SceneUtils::makePbdEntity(name, stringMesh, pbdSystem);
    stringObj->getComponent<VisualModel>()->setRenderMaterial(material);

    stringObj->getComponent<PbdMethod>()->setUniformMass(0.0001 / numVerts); // grams
    stringObj->getComponent<PbdMethod>()->setFixedNodes({ 0, 1,
                                                          stringMesh->getNumVertices() - 2, stringMesh->getNumVertices() - 1 });
    pbdParams->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 200.0);
    pbdParams->enableBendConstraint(0.01, 1);
    //pbdParams->enableBendConstraint(.5, 2);

    return stringObj;
}

static std::shared_ptr<Entity>
makeNeedleObj()
{
    auto sutureMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));
    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);

    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_gravity    = Vec3d::Zero();
    pbdSystem->getConfig()->m_iterations = 5;

    auto needleObj    = SceneUtils::makePbdEntity("Needle", sutureMesh, pbdSystem);
    auto needleVisual = needleObj->getComponent<VisualModel>();
    auto material     = needleVisual->getRenderMaterial();
    material->setColor(Color(0.9, 0.9, 0.9));
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->setRoughness(0.5);
    material->setMetalness(1.0);

    /*
    needleObj->getPbdBody()->m_mass = 1.0;
    needleObj->getPbdBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    needleObj->getPbdBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);
    */
    auto needleMethod = needleObj->getComponent<PbdMethod>();
    needleMethod->setRigid(Vec3d::Zero(), 1.0, Quatd::Identity(), Mat3d::Identity() * 10000.0);

    auto controller = needleObj->addComponent<PbdObjectController>();
    controller->setControlledObject(needleMethod, needleVisual);
    controller->setTranslationOffset(Vec3d(-0.02, 0.02, 0.0));
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.0);

    return needleObj;
}

///
/// \brief This example demonstrates suture on suture collision via CCD
int
SutureSelfCCD()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("PbdSutureSelfCCD");

    auto threadObj =
        // makePbdString("selfCCDLine", "");
        makePbdString("granny_knot", iMSTK_DATA_ROOT "/LineMesh/granny_knot.obj");
    scene->addSceneObject(threadObj);

    auto interaction = std::make_shared<PbdObjectCollision>(threadObj, threadObj);
    // Important parameter for stability, take multiple smaller steps to resolve multiple contacts
    interaction->setDeformableStiffnessA(0.05);
    interaction->setDeformableStiffnessB(0.05);
    scene->addInteraction(interaction);

    // Create the arc needle
    auto needleObj = makeNeedleObj();
    scene->addSceneObject(needleObj);

    // Adjust the camera
    scene->getActiveCamera()->setFocalPoint(0.022, -0.045, -0.01);
    scene->getActiveCamera()->setPosition(0.02, -0.02, 0.2);
    scene->getActiveCamera()->setViewUp(0, 1, 0);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);
        viewer->setBackgroundColors(Color(202.0 / 255.0, 212.0 / 255.0, 157.0 / 255.0));

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        // Setup a simulation manager to manage renders & scene updates
        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.0005); // 1ms, 1000hz //timestep

        auto controller = needleObj->getComponent<PbdObjectController>();
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        deviceClient->setOrientation(Quatd(Rotd(1.57, Vec3d(0.0, 1.0, 0.0))));

        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseMove,
            [&](MouseEvent* e)
            {
                const Vec2d& mousePos = viewer->getMouseDevice()->getPos();
                const Vec2d pos       = (mousePos - Vec2d(0.5, 0.5)) * 0.1;
                deviceClient->setPosition(Vec3d(pos[0], pos[1], 0.0));
            });
#endif
        controller->setDevice(deviceClient);

        // Update the thread fixed points to the controlled needle
        auto threadMethod = threadObj->getComponent<PbdMethod>();
        auto needleMethod = needleObj->getComponent<PbdMethod>();
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [ = ](Event*)
            {
                auto threadLineMesh = std::dynamic_pointer_cast<LineMesh>(threadMethod->getGeometry());
                std::shared_ptr<Geometry> geom = needleMethod->getGeometry();
                const Vec3d pos = geom->getTranslation();
                const Mat3d rot = geom->getRotation();
                (*threadLineMesh->getVertexPositions())[1] = pos;
                (*threadLineMesh->getVertexPositions())[0] = pos + rot * Vec3d(0.0, 0.002, 0.0);
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
