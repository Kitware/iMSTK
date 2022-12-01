/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkMeshIO.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPlane.h"
#include "imstkPortHoleInteraction.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneControlText.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKOpenVRViewer.h"
#include "imstkVTKViewer.h"
#include "VRCameraControl.h"

using namespace imstk;

///
/// \brief Create a laprascopic tool object
///
std::shared_ptr<PbdObject>
makeLapToolObj(const std::string&        name,
               std::shared_ptr<PbdModel> model)
{
    auto lapTool = std::make_shared<PbdObject>(name);

    const double capsuleLength = 0.5;
    auto         toolGeom      = std::make_shared<Capsule>(
        Vec3d(0.0, 0.0, capsuleLength * 0.5 - 0.005), // Position
        0.002,                                        // Radius
        capsuleLength,                                // Length
        Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));     // Orientation

    const double lapToolHeadLength = 0.01;
    auto         graspCapsule      = std::make_shared<Capsule>(
        Vec3d(0.0, 0.0, lapToolHeadLength * 0.5),                           // Position
        0.004,                                                              // Radius
        lapToolHeadLength,                                                  // Length
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0))); // Orientation

    auto lapToolVisualGeom = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/LapTool/laptool_all_in_one.obj");

    lapTool->setDynamicalModel(model);
    lapTool->setPhysicsGeometry(toolGeom);
    lapTool->addComponent<Collider>()->setGeometry(toolGeom);
    lapTool->setVisualGeometry(lapToolVisualGeom);
    lapTool->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, lapToolVisualGeom));

    // Add grasp capsule for visualization
    auto graspVisualModel = std::make_shared<VisualModel>();
    graspVisualModel->setGeometry(graspCapsule);
    graspVisualModel->getRenderMaterial()->setIsDynamicMesh(false);
    graspVisualModel->setIsVisible(false);
    lapTool->addVisualModel(graspVisualModel);

    std::shared_ptr<RenderMaterial> material = lapTool->getVisualModel(0)->getRenderMaterial();
    material->setIsDynamicMesh(false);
    material->setMetalness(1.0);
    material->setRoughness(0.2);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    lapTool->getPbdBody()->setRigid(
        Vec3d(0.0, 0.0, capsuleLength * 0.5) + Vec3d(0.0, 0.1, -1.0),
        5.0,
        Quatd::Identity(),
        Mat3d::Identity() * 0.08);

    // \todo: The grasp capsule and its map can't be placed as components yet.
    // For now grasp capsule is placed as a VisualModel and the map updated in this lambda
    auto graspCapsuleMap    = std::make_shared<IsometricMap>(toolGeom, graspCapsule);
    auto graspCapsuleUpdate = lapTool->addComponent<LambdaBehaviour>("graspCapsuleUpdate");
    graspCapsuleUpdate->setUpdate([ = ](const double&)
        {
            graspCapsuleMap->update();
        });

    return lapTool;
}

///
/// \brief Create a hand to grasp the tool
///
std::shared_ptr<PbdObject>
makeHandObj(const std::string&        name,
            std::shared_ptr<PbdModel> model)
{
    auto handSphereObj = std::make_shared<PbdObject>(name);

    auto sphere = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.02);

    handSphereObj->setDynamicalModel(model);
    handSphereObj->setPhysicsGeometry(sphere);
    handSphereObj->addComponent<Collider>()->setGeometry(sphere);
    handSphereObj->setVisualGeometry(sphere);

    std::shared_ptr<RenderMaterial> material = handSphereObj->getVisualModel(0)->getRenderMaterial();
    material->setIsDynamicMesh(false);
    material->setMetalness(0.0);
    material->setRoughness(1.0);
    material->setColor(Color::Green);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    handSphereObj->getPbdBody()->setRigid(
        Vec3d(0.0, 0.1, -1.0),
        5.0,
        Quatd::Identity(),
        Mat3d::Identity() * 0.08);

    auto controller = handSphereObj->addComponent<PbdObjectController>();
    controller->setControlledObject(handSphereObj);
    controller->setLinearKs(10000.0);
    controller->setAngularKs(10.0);
    controller->setForceScaling(0.01);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    return handSphereObj;
}

///
/// \brief Create pbd string object attached to a needle
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength,
    std::shared_ptr<PbdObject> needleObj)
{
    auto stringObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

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
    stringObj->addComponent<Collider>()->setGeometry(stringMesh);
    std::shared_ptr<PbdModel> model = needleObj->getPbdModel();
    stringObj->setDynamicalModel(model);
    //stringObj->getPbdBody()->fixedNodeIds = { 0, 1, 19, 20 };
    stringObj->getPbdBody()->uniformMassValue = 0.02;

    const int bodyHandle = stringObj->getPbdBody()->bodyHandle;
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1000.0,
        bodyHandle);
    model->getConfig()->enableBendConstraint(0.1, 1, true, bodyHandle);
    //model->getConfig()->enableBendConstraint(0.1, 2, true, bodyHandle);

    // Add a component to update the suture thread to be on the needle
    auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getPhysicsGeometry());

    // Add an attachment constraint for two-way between the string and needle
    // This is important to be able to pull the needle by the string
    model->getConfig()->addPbdConstraintFunctor([ = ](PbdConstraintContainer& container)
        {
            const Vec3d endOfNeedle   = (*needleLineMesh->getVertexPositions())[0];
            auto attachmentConstraint = std::make_shared<PbdBodyToBodyDistanceConstraint>();
            attachmentConstraint->initConstraint(model->getBodies(),
                { needleObj->getPbdBody()->bodyHandle, 0 },
                endOfNeedle,
                { stringObj->getPbdBody()->bodyHandle, 0 }, // Start of string
                0.0,                                        // Rest length
                0.0000001);
            container.addConstraint(attachmentConstraint);
        });

    return stringObj;
}

///
/// \brief This example demonstrates needle and thread grasping with proper
/// lap tool control in VR.
/// Spheres are used for the users hands which may grasp the lap tools at
/// any point.
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene = std::make_shared<Scene>("VRLapToolControl");

    auto model = std::make_shared<PbdModel>();
    model->getConfig()->m_gravity = Vec3d::Zero();
    model->getConfig()->m_dt      = 0.001;
    model->getConfig()->m_doPartitioning = false;

    auto bodyObject = std::make_shared<Entity>("body");
    {
        auto surfMesh  = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/human/full_body/body.obj");
        auto bodyPlane = std::make_shared<Plane>(Vec3d(0.0, -0.04, -1.0), Vec3d(0.0, 1.0, 0.0));
        bodyObject->addComponent<Collider>()->setGeometry(bodyPlane);
        auto visualModel = bodyObject->addComponent<VisualModel>();
        visualModel->setGeometry(surfMesh);
        visualModel->getRenderMaterial()->setShadingModel(
            RenderMaterial::ShadingModel::PBR);
        std::shared_ptr<RenderMaterial> material =
            visualModel->getRenderMaterial();
        material->setRoughness(0.8);
        material->setMetalness(0.1);
        material->setOpacity(0.5);
    }
    scene->addSceneObject(bodyObject);

    // Add the hands
    std::shared_ptr<PbdObject> leftHandObj = makeHandObj("leftHand", model);
    scene->addSceneObject(leftHandObj);
    std::shared_ptr<PbdObject> rightHandObj = makeHandObj("leftHand", model);
    scene->addSceneObject(rightHandObj);

    // Add the rigid lap tools
    std::shared_ptr<PbdObject> leftToolObj = makeLapToolObj("leftLapTool", model);
    scene->addSceneObject(leftToolObj);
    std::shared_ptr<PbdObject> rightToolObj = makeLapToolObj("rightLapTool", model);
    scene->addSceneObject(rightToolObj);

    // Add a rigid needle
    auto needleObj = std::make_shared<PbdObject>();
    {
        auto needleMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
        auto needleLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");
        // Transform so center of mass is in center of the needle
        needleMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleLineMesh->translate(Vec3d(0.0, -0.0047, -0.0087), Geometry::TransformType::ApplyToData);
        needleObj->setVisualGeometry(needleMesh);
        needleObj->addComponent<Collider>()->setGeometry(needleLineMesh);
        needleObj->setPhysicsGeometry(needleLineMesh);
        needleObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(needleLineMesh, needleMesh));
        needleObj->setDynamicalModel(model);
        needleObj->getPbdBody()->setRigid(
            Vec3d(0.02, 0.0, -1.26),
            1.0,
            Quatd::Identity(),
            Mat3d::Identity() * 0.01);
        needleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
    }
    scene->addSceneObject(needleObj);

    // Add deformable suture thread
    auto sutureThreadObj = makePbdString("sutureThread",
        Vec3d(0.02, 0.0, -1.26), Vec3d(0.0, 0.0, 1.0), 50, 0.2, needleObj);
    scene->addSceneObject(sutureThreadObj);

    // Add tool-on-tool collision
    auto lapToolCollision = std::make_shared<PbdObjectCollision>(leftToolObj, rightToolObj);
    lapToolCollision->setRigidBodyCompliance(0.00001);
    scene->addInteraction(lapToolCollision);

    // Add thread-on-tool collisions
    auto threadCollision0 = std::make_shared<PbdObjectCollision>(leftToolObj, sutureThreadObj);
    threadCollision0->setRigidBodyCompliance(0.0001);
    threadCollision0->setUseCorrectVelocity(false);
    //threadCollision0->setFriction(0.1);
    scene->addInteraction(threadCollision0);
    auto threadCollision1 = std::make_shared<PbdObjectCollision>(rightToolObj, sutureThreadObj);
    threadCollision1->setRigidBodyCompliance(0.0001);
    threadCollision1->setUseCorrectVelocity(false);
    //threadCollision1->setFriction(0.1);
    scene->addInteraction(threadCollision1);

    // Add left grasping
    auto leftToolGrasping = std::make_shared<PbdObjectGrasping>(leftToolObj, leftHandObj);
    leftToolGrasping->setCompliance(0.00001);
    scene->addInteraction(leftToolGrasping);
    auto leftNeedleGrasping = std::make_shared<PbdObjectGrasping>(needleObj, leftToolObj);
    leftNeedleGrasping->setCompliance(0.00001);
    scene->addInteraction(leftNeedleGrasping);
    auto leftThreadGrasping = std::make_shared<PbdObjectGrasping>(sutureThreadObj, leftToolObj);
    leftThreadGrasping->setCompliance(0.00001);
    scene->addInteraction(leftThreadGrasping);

    // Add right grasping
    auto rightToolGrasping = std::make_shared<PbdObjectGrasping>(rightToolObj, rightHandObj);
    rightToolGrasping->setCompliance(0.00001);
    scene->addInteraction(rightToolGrasping);
    auto rightNeedleGrasping = std::make_shared<PbdObjectGrasping>(needleObj, rightToolObj);
    rightNeedleGrasping->setCompliance(0.00001);
    scene->addInteraction(rightNeedleGrasping);
    auto rightThreadGrasping = std::make_shared<PbdObjectGrasping>(sutureThreadObj, rightToolObj);
    rightThreadGrasping->setCompliance(0.00001);
    scene->addInteraction(rightThreadGrasping);

    // Add thread-thread self collision
    auto threadOnThreadCollision = std::make_shared<PbdObjectCollision>(sutureThreadObj, sutureThreadObj);
    threadOnThreadCollision->setDeformableStiffnessA(0.05);
    threadOnThreadCollision->setDeformableStiffnessB(0.05);
    scene->addInteraction(threadOnThreadCollision);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Add port holes
    auto portHoleInteraction = rightToolObj->addComponent<PortHoleInteraction>();
    portHoleInteraction->setTool(rightToolObj);
    portHoleInteraction->setPortHoleLocation(Vec3d(0.015, 0.092, -1.117));
    auto sphere = std::make_shared<Sphere>(Vec3d(0.015, 0.092, -1.117), 0.01);
    auto rightPortVisuals = rightToolObj->addComponent<VisualModel>();
    rightPortVisuals->setGeometry(sphere);
    portHoleInteraction->setToolGeometry(rightToolObj->getComponent<Collider>()->getGeometry());
    portHoleInteraction->setCompliance(0.000001);

    auto portHoleInteraction2 = leftToolObj->addComponent<PortHoleInteraction>();
    portHoleInteraction2->setTool(leftToolObj);
    portHoleInteraction2->setPortHoleLocation(Vec3d(-0.065, 0.078, -1.127));
    auto sphere2 = std::make_shared<Sphere>(Vec3d(-0.065, 0.078, -1.127), 0.01);
    auto leftPortVisuals = leftToolObj->addComponent<VisualModel>();
    leftPortVisuals->setGeometry(sphere2);
    portHoleInteraction2->setToolGeometry(leftToolObj->getComponent<Collider>()->getGeometry());
    portHoleInteraction2->setCompliance(0.000001);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKOpenVRViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        auto                                leftController   = leftHandObj->getComponent<PbdObjectController>();
        std::shared_ptr<OpenVRDeviceClient> leftDeviceClient = viewer->getVRDeviceClient(OPENVR_LEFT_CONTROLLER);
        leftController->setDevice(leftDeviceClient);

        auto                                rightController   = rightHandObj->getComponent<PbdObjectController>();
        std::shared_ptr<OpenVRDeviceClient> rightDeviceClient = viewer->getVRDeviceClient(OPENVR_RIGHT_CONTROLLER);
        rightController->setDevice(rightDeviceClient);

        connect<ButtonEvent>(rightDeviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                // Right trigger
                if (e->m_button == 7)
                {
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        if (rightToolGrasping->hasConstraints())
                        {
                            rightToolGrasping->endGrasp();
                        }
                        else
                        {
                            rightToolGrasping->beginCellGrasp(
                                std::dynamic_pointer_cast<Sphere>(rightHandObj->getComponent<Collider>()->getGeometry()));
                        }
                    }
                }
                // Right grip
                else if (e->m_button == 5)
                {
                    viewer->setRenderingMode(Renderer::Mode::Debug);
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        // Use a slighty larger capsule at the tip
                        auto graspCapsule = std::dynamic_pointer_cast<Capsule>(rightToolObj->getVisualModel(1)->getGeometry());
                        rightNeedleGrasping->beginCellGrasp(graspCapsule);
                        rightThreadGrasping->beginCellGrasp(graspCapsule);
                    }
                    else if (e->m_buttonState == BUTTON_RELEASED)
                    {
                        rightNeedleGrasping->endGrasp();
                        rightThreadGrasping->endGrasp();
                    }
                }
            });
        connect<ButtonEvent>(leftDeviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                // Left trigger
                if (e->m_button == 6)
                {
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        if (leftToolGrasping->hasConstraints())
                        {
                            leftToolGrasping->endGrasp();
                        }
                        else
                        {
                            leftToolGrasping->beginCellGrasp(
                                std::dynamic_pointer_cast<Sphere>(leftHandObj->getComponent<Collider>()->getGeometry()));
                        }
                    }
                }
                // Left grip
                if (e->m_button == 4)
                {
                    if (e->m_buttonState == BUTTON_PRESSED)
                    {
                        // Use a slighty larger capsule at the tip
                        auto graspCapsule = std::dynamic_pointer_cast<Capsule>(leftToolObj->getVisualModel(1)->getGeometry());
                        leftNeedleGrasping->beginCellGrasp(graspCapsule);
                        leftThreadGrasping->beginCellGrasp(graspCapsule);
                    }
                    else if (e->m_buttonState == BUTTON_RELEASED)
                    {
                        leftNeedleGrasping->endGrasp();
                        leftThreadGrasping->endGrasp();
                    }
                }
            });

        // Add default mouse and keyboard controls to the viewer
        auto controls   = std::make_shared<Entity>();
        auto camControl = controls->addComponent<VRCameraControl>();
        camControl->setRotateDevice(viewer->getVRDeviceClient(OPENVR_RIGHT_CONTROLLER));
        camControl->setTranslateDevice(viewer->getVRDeviceClient(OPENVR_LEFT_CONTROLLER));
        camControl->setTranslateSpeedScale(1.0);
        camControl->setRotateSpeedScale(1.0);
        camControl->setCamera(scene->getActiveCamera());
        scene->addSceneObject(controls);

        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                model->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}
