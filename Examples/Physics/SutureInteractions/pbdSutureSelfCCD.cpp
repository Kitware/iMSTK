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
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
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
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(const std::string& name, const std::string& filename)
{
    // Setup the Geometry
    auto stringMesh = MeshIO::read<LineMesh>(filename);

    const int numVerts = stringMesh->getNumVertices();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.0005;
    pbdParams->m_iterations = 1;
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(4.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto stringObj = std::make_shared<PbdObject>(name);
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->addComponent<Collider>()->setGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);

    stringObj->getPbdBody()->uniformMassValue = 0.0001 / numVerts; // grams
    stringObj->getPbdBody()->fixedNodeIds     = { 0, 1,
                                                  stringMesh->getNumVertices() - 2, stringMesh->getNumVertices() - 1 };
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 200.0);
    pbdParams->enableBendConstraint(0.01, 1);
    //pbdParams->enableBendConstraint(.5, 2);

    return stringObj;
}

static std::shared_ptr<PbdObject>
makeNeedleObj()
{
    auto needleObj = std::make_shared<PbdObject>();

    auto sutureMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));
    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);

    needleObj->setVisualGeometry(sutureMesh);
    needleObj->addComponent<Collider>()->setGeometry(sutureMesh);
    needleObj->setPhysicsGeometry(sutureMesh);
    needleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    needleObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    needleObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    needleObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);

    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_gravity    = Vec3d::Zero();
    pbdModel->getConfig()->m_iterations = 5;
    needleObj->setDynamicalModel(pbdModel);

    /*
    needleObj->getPbdBody()->m_mass = 1.0;
    needleObj->getPbdBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    needleObj->getPbdBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);
    */
    needleObj->getPbdBody()->setRigid(Vec3d::Zero(), 1.0, Quatd::Identity(), Mat3d::Identity() * 10000.0);

    auto controller = needleObj->addComponent<PbdObjectController>();
    controller->setControlledObject(needleObj);
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

    std::shared_ptr<PbdObject> threadObj =
        // makePbdString("selfCCDLine", "");
        makePbdString("granny_knot", iMSTK_DATA_ROOT "/LineMesh/granny_knot.obj");
    scene->addSceneObject(threadObj);

    auto interaction = std::make_shared<PbdObjectCollision>(threadObj, threadObj);
    // Important parameter for stability, take multiple smaller steps to resolve multiple contacts
    interaction->setDeformableStiffnessA(0.05);
    interaction->setDeformableStiffnessB(0.05);
    scene->addInteraction(interaction);

    // Create the arc needle
    std::shared_ptr<PbdObject> needleObj = makeNeedleObj();
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
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                auto threadLineMesh = std::dynamic_pointer_cast<LineMesh>(threadObj->getPhysicsGeometry());
                std::shared_ptr<Geometry> geom = needleObj->getPhysicsGeometry();
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
