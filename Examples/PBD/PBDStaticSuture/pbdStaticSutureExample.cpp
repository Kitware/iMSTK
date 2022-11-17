/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkArcNeedle.h"
#include "imstkCamera.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkOrientedBox.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPuncturable.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleInteraction.h"

#undef iMSTK_USE_HAPTICS
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
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength)
{
    auto stringObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 100.0);
    pbdParams->enableBendConstraint(100000.0, 1);
    pbdParams->enableBendConstraint(100000.0, 2);
    pbdParams->m_gravity = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt      = 0.0005;                    // Overwritten for real time

    // Requires large amounts of iterations the longer, a different
    // solver would help
    pbdParams->m_iterations = 100;
    pbdParams->m_linearDampingCoeff = 0.01;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Setup the Object
    stringObj->setVisualGeometry(stringMesh);
    stringObj->getVisualModel(0)->setRenderMaterial(material);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);
    stringObj->getPbdBody()->fixedNodeIds     = { 0, 1, 19, 20 };
    stringObj->getPbdBody()->uniformMassValue = 0.002 / numVerts; // grams

    return stringObj;
}

///
/// \brief Generate a static/immovable tissue for static suturing
///
static std::shared_ptr<CollidingObject>
makeTissueObj()
{
    auto tissueObj = std::make_shared<CollidingObject>("tissue");

    auto box1      = std::make_shared<OrientedBox>(Vec3d(0.0, -0.1, -0.1), Vec3d(0.1, 0.025, 0.1));
    auto box1Model = std::make_shared<VisualModel>();
    box1Model->setGeometry(box1);
    box1Model->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Gouraud);
    box1Model->getRenderMaterial()->setColor(Color::LightSkin);
    tissueObj->addVisualModel(box1Model);

    tissueObj->setCollidingGeometry(box1);

    auto box2      = std::make_shared<OrientedBox>(Vec3d(0.0, -0.105, -0.1), Vec3d(0.1001, 0.025, 0.1001));
    auto box2Model = std::make_shared<VisualModel>();
    box2Model->setGeometry(box2);
    box2Model->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Gouraud);
    box2Model->getRenderMaterial()->setColor(Color::darken(Color::Yellow, 0.2));
    tissueObj->addVisualModel(box2Model);

    tissueObj->addComponent<Puncturable>();

    return tissueObj;
}

static std::shared_ptr<SceneObject>
makeToolObj(std::string name)
{
    auto surfMesh =
        MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Clamps/Gregory Suture Clamp/gregory_suture_clamp.obj");

    auto toolObj = std::make_shared<SceneObject>(name);
    toolObj->setVisualGeometry(surfMesh);
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(Color::LightGray);
    renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
    renderMaterial->setRoughness(0.5);
    renderMaterial->setMetalness(1.0);
    toolObj->getVisualModel(0)->setRenderMaterial(renderMaterial);

    return toolObj;
}

static std::shared_ptr<RigidObject2>
makeNeedleObj()
{
    auto needleObj = std::make_shared<RigidObject2>();

    auto sutureMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
    auto sutureLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));
    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureLineMesh->transform(rot, Geometry::TransformType::ApplyToData);

    needleObj->setVisualGeometry(sutureMesh);
    needleObj->setCollidingGeometry(sutureLineMesh);
    needleObj->setPhysicsGeometry(sutureLineMesh);
    needleObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(sutureLineMesh, sutureMesh));
    needleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    needleObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    needleObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    needleObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    needleObj->setDynamicalModel(rbdModel);

    needleObj->getRigidBody()->m_mass = 1.0;
    needleObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    needleObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    // Manually setup an arc aligned with the geometry, some sort of needle+arc generator
    // could be a nice addition to imstk
    Mat3d arcBasis = Mat3d::Identity();
    arcBasis.col(0) = Vec3d(0.0, 0.0, -1.0);
    arcBasis.col(1) = Vec3d(1.0, 0.0, 0.0);
    arcBasis.col(2) = Vec3d(0.0, 1.0, 0.0);
    arcBasis = rot.block<3, 3>(0, 0) * arcBasis;
    const Vec3d  arcCenter = (rot * Vec4d(0.0, -0.005455, 0.008839, 1.0)).head<3>();
    const double arcRadius = 0.010705;

    // Add a component for needles
    auto needle = needleObj->addComponent<ArcNeedle>();
    needle->setArc(arcCenter, arcBasis, arcRadius, 0.558, 2.583);

    // Add a component to control the tool
    auto controller = needleObj->addComponent<RigidObjectController>();
    controller->setControlledObject(needleObj);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.2);
    controller->setSmoothingKernelSize(5);
    controller->setUseForceSmoothening(true);

    return needleObj;
}

///
/// \brief This example is an initial suturing example testbed. It provides the constraint
/// required for an arc shaped needle puncturing vs a static/immovable tissue. What it
/// does not do:
///  - The tissue is not deformable yet, so insertion is a bit stiff
///  - It only constrains the arc to the surface point it punctures not the volume
///  - The suture thread isn't constrained yet
///  - Ability to graps/release the needle, combining into one body
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("PbdStaticSuture");

    // Create the arc needle
    std::shared_ptr<RigidObject2> needleObj = makeNeedleObj();
    scene->addSceneObject(needleObj);

    // Create the suture pbd-based string
    const double               stringLength      = 0.2;
    const int                  stringVertexCount = 30;
    std::shared_ptr<PbdObject> sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength);
    scene->addSceneObject(sutureThreadObj);

    // Create a static box for tissue
    std::shared_ptr<CollidingObject> tissueObj = makeTissueObj();
    scene->addSceneObject(tissueObj);

    // Create clamps that follow the needle around
    std::shared_ptr<SceneObject> clampsObj = makeToolObj("Clamps");
    scene->addSceneObject(clampsObj);

    // Create ghost clamps to show real position of hand under virtual coupling
    std::shared_ptr<SceneObject> ghostClampsObj = makeToolObj("GhostClamps");
    ghostClampsObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
    scene->addSceneObject(ghostClampsObj);

    // Add point based collision between the tissue & suture thread
    auto interaction = std::make_shared<PbdObjectCollision>(sutureThreadObj, tissueObj);
    interaction->setFriction(0.0);
    scene->addInteraction(interaction);

    // Add needle constraining behaviour between the tissue & arc needle
    auto needleInteraction = std::make_shared<NeedleInteraction>(tissueObj, needleObj);
    scene->addInteraction(needleInteraction);

    // Adjust the camera
    scene->getActiveCamera()->setFocalPoint(0.00138345, -0.0601133, -0.0261938);
    scene->getActiveCamera()->setPosition(0.00137719, 0.0492882, 0.201508);
    scene->getActiveCamera()->setViewUp(-0.000780726, 0.901361, -0.433067);

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

        // Setup a simulation manager to manage renders & scene updates
        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

        auto controller = needleObj->getComponent<RigidObjectController>();
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        controller->setTranslationOffset(Vec3d(0.05, -0.05, 0.0));
#else
        auto deviceClient = std::make_shared<DummyClient>();
        deviceClient->setOrientation(Quatd(Rotd(1.57, Vec3d(0.0, 1.0, 0.0))));
        controller->setTranslationScaling(0.13);
        controller->setTranslationOffset(Vec3d(-0.05, -0.1, -0.005));

        auto needleMouseMove = needleObj->addComponent<LambdaBehaviour>("NeedleMouseMove");
        needleMouseMove->setUpdate([&](const double&)
            {
                const Vec2d& pos2d = viewer->getMouseDevice()->getPos();
                deviceClient->setPosition(Vec3d(pos2d[0], pos2d[1], 0.0));
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseScroll,
            [&](MouseEvent* e)
            {
                const Quatd delta = Quatd(Rotd(e->m_scrollDx * 0.1, Vec3d(0.0, 0.0, 1.0)));
                deviceClient->setOrientation(deviceClient->getOrientation() * delta);
            });
#endif
        controller->setDevice(deviceClient);

        // Update the timesteps for real time
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                needleObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                // sutureThreadObj->getPbdModel()->getConfig()->m_dt  = sceneManager->getDt();
            });
        // Constrain the first two vertices of the string to the needle
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getPhysicsGeometry());
                auto sutureLineMesh = std::dynamic_pointer_cast<LineMesh>(sutureThreadObj->getPhysicsGeometry());
                (*sutureLineMesh->getVertexPositions())[1] = (*needleLineMesh->getVertexPositions())[0];
                (*sutureLineMesh->getVertexPositions())[0] = (*needleLineMesh->getVertexPositions())[1];
            });
        // Transform the clamps relative to the needle
        const Vec3d clampOffset = Vec3d(-0.009, 0.01, 0.001);
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                clampsObj->getVisualGeometry()->setTransform(
                    needleObj->getVisualGeometry()->getTransform() *
                    mat4dTranslate(clampOffset) *
                    mat4dRotation(Rotd(PI, Vec3d(0.0, 1.0, 0.0))));
                clampsObj->getVisualGeometry()->postModified();
            });
        // Transform the ghost tool clamps to show the real tool location
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                ghostClampsObj->getVisualGeometry()->setTransform(
                    mat4dTranslate(controller->getPosition()) * mat4dRotation(controller->getOrientation()) *
                    mat4dTranslate(clampOffset) *
                    mat4dRotation(Rotd(PI, Vec3d(0.0, 1.0, 0.0))));
                ghostClampsObj->getVisualGeometry()->updatePostTransformData();
                ghostClampsObj->getVisualGeometry()->postModified();
                ghostClampsObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 5.0));
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}