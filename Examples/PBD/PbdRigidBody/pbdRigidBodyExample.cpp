/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkPbdHingeJointConstraint.h"

#include <thread>
#include <chrono>

using namespace imstk;

///
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string&        name,
              std::shared_ptr<PbdModel> model,
              const double              width,
              const double              height,
              const int                 rowCount,
              const int                 colCount,
              const double              particleMassValue)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> tissueMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount));

    // Setup the Parameters
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, distStiffness);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, bendStiffness);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tissueMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>(name);
    pbdObject->addVisualModel(visualModel);
    pbdObject->setPhysicsGeometry(tissueMesh);
    pbdObject->setCollidingGeometry(tissueMesh);
    pbdObject->setDynamicalModel(model);

    pbdObject->getPbdBody()->uniformMassValue = particleMassValue;
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
/// \brief Creates tissue object
///
static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 std::shared_ptr<PbdModel> model,
                 const double particleMassValue,
                 const double distStiffness, const double bendStiffness)
{
    // Setup the Geometry
    auto                    tissueMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> vertices   = { Vec3d(-0.1, 0.0, -0.1), Vec3d(0.0, 0.0, 0.1), Vec3d(0.1, 0.0, -0.1) };
    VecDataArray<int, 3>    indices    = { Vec3i(0, 1, 2) };
    tissueMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 3>>(indices));

    // Setup the Parameters
    //model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, distStiffness);
    //model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, bendStiffness);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tissueMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto pbdObject = std::make_shared<PbdObject>(name);
    pbdObject->addVisualModel(visualModel);
    pbdObject->setPhysicsGeometry(tissueMesh);
    pbdObject->setCollidingGeometry(tissueMesh);
    pbdObject->setDynamicalModel(model);

    pbdObject->getPbdBody()->uniformMassValue = particleMassValue;

    return pbdObject;
}

static void
sausageScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRigidBody");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -20.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.005;
    pbdConfig->m_iterations = 3;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.03;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    // Setup a capsule
    auto rigidObj = std::make_shared<PbdObject>("rigidObj");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.5, 2.0);
        rigidObj->setVisualGeometry(rigidGeom);
        rigidObj->setCollidingGeometry(rigidGeom);
        rigidObj->setPhysicsGeometry(rigidGeom);

        // Setup material
        rigidObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        rigidObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        rigidObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        rigidObj->setDynamicalModel(pbdModel);

        // Setup body
        //const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0).normalized());
        rigidObj->getPbdBody()->setRigid(Vec3d(-4.0, 1.0, 0.0), 10.0,
            Quatd::Identity(), Mat3d::Identity() * 100.0);
    }
    scene->addSceneObject(rigidObj);

    //auto collision = std::make_shared<PbdObjectCollision>(rigidObj, tissueObj, "SurfaceMeshToCapsuleCD");
    ////auto collision = std::make_shared<PbdObjectCollision>(tissueObj, capsuleObj, "SurfaceMeshToSphereCD");
    //scene->addSceneObject(collision);

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
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
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

        driver->start();
    }
}

static void
planeContactScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRigidBody");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    // Slightly larger gravity to compensate damping
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 10;
    pbdConfig->m_collisionIterations = 10;
    pbdConfig->m_linearDampingCoeff  = 0.001;
    pbdConfig->m_angularDampingCoeff = 0.001;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    auto planeObj  = std::make_shared<CollidingObject>("plane");
    auto planeGeom = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    planeGeom->setWidth(1.0);
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Setup a capsule
    auto rigidPbdObj = std::make_shared<PbdObject>("rigidPbdObj");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto                         rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.05, 0.25);
        std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
        rigidPbdObj->setVisualGeometry(surfMesh);
        rigidPbdObj->setCollidingGeometry(surfMesh);
        rigidPbdObj->setPhysicsGeometry(surfMesh);

        // Setup material
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        rigidPbdObj->setDynamicalModel(pbdModel);

        // Setup body
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 1.0).normalized());
        //const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0).normalized());
        //const Mat3d inertia = Mat3d::Identity() * 100.0;
        rigidPbdObj->getPbdBody()->setRigid(Vec3d(0.0, 0.2, 0.0),
            1.0, orientation, Mat3d::Identity() * 0.01);
    }
    scene->addSceneObject(rigidPbdObj);

    auto collision = std::make_shared<PbdObjectCollision>(rigidPbdObj, planeObj, "PointSetToPlaneCD");
    scene->addSceneObject(collision);

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
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
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

            std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
            const Vec3d                           dx    = scene->getActiveCamera()->getPosition() - scene->getActiveCamera()->getFocalPoint();
            const double                          speed = 10.0;
            connect<KeyEvent>(keyDevice, &KeyboardDeviceClient::keyPress, [&](KeyEvent* e)
                {
                    if (e->m_key == 'f')
                    {
                        if (collision->getFriction() == 0.0)
                        {
                            collision->setFriction(0.5);
                        }
                        else
                        {
                            collision->setFriction(0.0);
                        }
                    }
                });
            connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
                {
                    Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
                    Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
                    // If w down, move forward
                    if (keyDevice->getButton('i') == KEY_PRESS)
                    {
                        extForce += Vec3d(0.0, 0.0, -1.0) * speed;
                    }
                    if (keyDevice->getButton('k') == KEY_PRESS)
                    {
                        extForce += Vec3d(0.0, 0.0, 1.0) * speed;
                    }
                    if (keyDevice->getButton('j') == KEY_PRESS)
                    {
                        extForce += Vec3d(-1.0, 0.0, 0.0) * speed;
                    }
                    if (keyDevice->getButton('l') == KEY_PRESS)
                    {
                        extForce += Vec3d(1.0, 0.0, 0.0) * speed;
                    }
                    // Apply torque around global y
                    if (keyDevice->getButton('u') == KEY_PRESS)
                    {
                        extTorque += Vec3d(0.0, -0.1, 0.0);
                    }
                    if (keyDevice->getButton('o') == KEY_PRESS)
                    {
                        extTorque += Vec3d(0.0, 0.1, 0.0);
                    }
                    rigidPbdObj->getPbdBody()->externalForce  = extForce;
                    rigidPbdObj->getPbdBody()->externalTorque = extTorque;
                });
        }

        driver->start();
    }
}

static void
bowlScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    auto scene   = std::make_shared<Scene>("RbdSDFCollision");
    auto cubeObj = std::make_shared<PbdObject>("Cube");
    {
        // This model is shared among interacting rigid bodies
        auto pbdModel  = std::make_shared<PbdModel>();
        auto pbdConfig = std::make_shared<PbdModelConfig>();
        // Slightly larger gravity to compensate damping
        pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
        pbdConfig->m_dt         = 0.001;
        pbdConfig->m_iterations = 1;
        pbdConfig->m_collisionIterations = 10;
        pbdConfig->m_linearDampingCoeff  = 0.001;
        pbdConfig->m_angularDampingCoeff = 0.001;
        pbdConfig->m_doPartitioning      = false;
        pbdModel->configure(pbdConfig);

        // Create the first rbd, plane floor
        auto planeObj = std::make_shared<CollidingObject>("Plane");
        {
            // Subtract the sphere from the plane to make a crater
            auto planeGeom = std::make_shared<Plane>();
            planeGeom->setWidth(1.0);
            auto sphereGeom = std::make_shared<Sphere>();
            sphereGeom->setRadius(0.625);
            sphereGeom->setPosition(0.0, 0.4, 0.0);
            auto compGeom = std::make_shared<CompositeImplicitGeometry>();
            compGeom->addImplicitGeometry(planeGeom, CompositeImplicitGeometry::GeometryBoolType::Union);
            compGeom->addImplicitGeometry(sphereGeom, CompositeImplicitGeometry::GeometryBoolType::Difference);

            // Rasterize the SDF into an image
            ImplicitGeometryToImageData toImage;
            toImage.setInputGeometry(compGeom);
            Vec6d bounds;
            bounds[0] = -0.5;
            bounds[1] = 0.5;
            bounds[2] = -0.5;
            bounds[3] = 0.5;
            bounds[4] = -0.5;
            bounds[5] = 0.5;
            toImage.setBounds(bounds);
            toImage.setDimensions(Vec3i(80, 80, 80));
            toImage.update();

            // Extract surface
            SurfaceMeshFlyingEdges toSurfMesh;
            toSurfMesh.setInputImage(toImage.getOutputImage());
            toSurfMesh.update();
            toSurfMesh.getOutputMesh()->flipNormals();

            // Create the object
            planeObj->setVisualGeometry(toSurfMesh.getOutputMesh());
            planeObj->setCollidingGeometry(compGeom);

            scene->addSceneObject(planeObj);
        }

        // Create surface mesh cube (so we can use pointset for point->implicit collision)
        {
            auto                         cubeGeom = std::make_shared<OrientedBox>(Vec3d::Zero(), Vec3d(0.0375, 0.075, 0.025));
            std::shared_ptr<SurfaceMesh> surfMesh = GeometryUtils::toSurfaceMesh(cubeGeom);

            SurfaceMeshSubdivide subdivide;
            subdivide.setInputMesh(surfMesh);
            subdivide.setNumberOfSubdivisions(1);
            subdivide.update();

            // Create the visual model
            auto visualModel = std::make_shared<VisualModel>();
            visualModel->setGeometry(subdivide.getOutputMesh());
            auto material = std::make_shared<RenderMaterial>();
            material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
            material->setLineWidth(2.0);
            material->setColor(Color::Orange);
            visualModel->setRenderMaterial(material);

            // Create the cube rigid object
            cubeObj->setDynamicalModel(pbdModel);
            cubeObj->setPhysicsGeometry(subdivide.getOutputMesh());
            cubeObj->setCollidingGeometry(subdivide.getOutputMesh());
            cubeObj->addVisualModel(visualModel);
            cubeObj->getPbdBody()->setRigid(Vec3d(0.0, 0.2, 0.0), 1.0,
                Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0))), Mat3d::Identity() * 0.01);

            scene->addSceneObject(cubeObj);
        }

        auto rbdInteraction = std::make_shared<PbdObjectCollision>(cubeObj, planeObj, "ImplicitGeometryToPointSetCD");
        rbdInteraction->setRigidBodyCompliance(0.00001);
        //rbdInteraction->setFriction(0.5);
        scene->addInteraction(rbdInteraction);

        // Camera
        scene->getActiveCamera()->setPosition(0.0, 1.0, 1.0);

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setIntensity(1.0);
        scene->addLight("light", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        auto driver = std::make_shared<SimulationManager>();
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

        LOG(INFO) << "Cube Controls:";
        LOG(INFO) << "----------------------------------------------------------------------";
        LOG(INFO) << " | i - forward movement";
        LOG(INFO) << " | j - left movement";
        LOG(INFO) << " | l - right movement";
        LOG(INFO) << " | k - backwards movement";
        LOG(INFO) << " | u - rotate left";
        LOG(INFO) << " | o - rotate right";

        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();

        const Vec3d  dx    = scene->getActiveCamera()->getPosition() - scene->getActiveCamera()->getFocalPoint();
        const double speed = 10.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
                Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
                // If w down, move forward
                if (keyDevice->getButton('i') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, -1.0) * speed;
                }
                if (keyDevice->getButton('k') == KEY_PRESS)
                {
                    extForce += Vec3d(0.0, 0.0, 1.0) * speed;
                }
                if (keyDevice->getButton('j') == KEY_PRESS)
                {
                    extForce += Vec3d(-1.0, 0.0, 0.0) * speed;
                }
                if (keyDevice->getButton('l') == KEY_PRESS)
                {
                    extForce += Vec3d(1.0, 0.0, 0.0) * speed;
                }
                if (keyDevice->getButton('u') == KEY_PRESS)
                {
                    extTorque += Vec3d(0.0, 1.5, 0.0);
                }
                if (keyDevice->getButton('o') == KEY_PRESS)
                {
                    extTorque += Vec3d(0.0, -1.5, 0.0);
                }
                cubeObj->getPbdBody()->externalForce  = extForce;
                cubeObj->getPbdBody()->externalTorque = extTorque;
                scene->getActiveCamera()->setFocalPoint((*cubeObj->getPbdBody()->vertices)[0]);
                scene->getActiveCamera()->setPosition((*cubeObj->getPbdBody()->vertices)[0] + dx);
            });
        /*connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                cubeObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
            });*/

        driver->start();
    }
}

static void
tissueCapsuleDrop()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRigidBody");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 10;
    pbdConfig->m_collisionIterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.0;
    pbdConfig->m_angularDampingCoeff = 0.0;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue", pbdModel,
        0.1, 0.1,
        5, 5,
        0.1,       // Per Particle Mass
        1.0, 0.2); // Distance & Bend Stiffness
    scene->addSceneObject(tissueObj);
    pbdConfig->setBodyDamping(tissueObj->getPbdBody()->bodyHandle, 0.1);

    // Setup capsule to drop on tissue
    auto capsuleObj = std::make_shared<PbdObject>("capsule0");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(-0.005, 0.0, 0.0), 0.005);
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(-0.005, 0.0, 0.0), 0.005, 0.015);
        capsuleObj->setVisualGeometry(rigidGeom);
        capsuleObj->setCollidingGeometry(rigidGeom);
        capsuleObj->setPhysicsGeometry(rigidGeom);

        // Setup material
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        capsuleObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        capsuleObj->setDynamicalModel(pbdModel);
        pbdConfig->setBodyDamping(capsuleObj->getPbdBody()->bodyHandle, 0.04, 0.01);

        // Setup body
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 0.0).normalized());
        capsuleObj->getPbdBody()->setRigid(Vec3d(0.0, 0.05, 0.0), 1.0,
            orientation, Mat3d::Identity() * 0.01);//Vec3d(1.0, 5.0, 1.0).asDiagonal());
    }
    scene->addSceneObject(capsuleObj);

    auto collision = std::make_shared<PbdObjectCollision>(tissueObj, capsuleObj);
    collision->setRigidBodyCompliance(0.00001);
    scene->addSceneObject(collision);

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
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                pbdModel->getConfig()->m_dt = sceneManager->getDt();
            });

        auto driver = std::make_shared<SimulationManager>();
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

        driver->start();
    }
}

static void
hingeScene()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    auto scene = std::make_shared<Scene>("PbdRigidBody");
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(-0.0237419, 0.0368787, 0.338374);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    auto pbdModel  = std::make_shared<PbdModel>();
    auto pbdConfig = std::make_shared<PbdModelConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, 0.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.03;
    pbdConfig->m_angularDampingCoeff = 0.03;
    pbdConfig->m_doPartitioning      = false;
    pbdModel->configure(pbdConfig);

    // Setup a capsule
    auto rigidPbdObj = std::make_shared<PbdObject>("rigidPbdObj");
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto                         rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.5, 2.0);
        std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
        rigidPbdObj->setVisualGeometry(surfMesh);
        rigidPbdObj->setCollidingGeometry(surfMesh);
        rigidPbdObj->setPhysicsGeometry(surfMesh);

        // Setup material
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
        rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

        rigidPbdObj->setDynamicalModel(pbdModel);

        // Setup body such that z is now pointing in -x
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 0.0, 1.0).normalized());
        //const Quatd orientation = Quatd::Identity();
        const Mat3d inertia = Vec3d(1.0, 1.0, 100.0).asDiagonal(); // Resistance on z
        rigidPbdObj->getPbdBody()->setRigid(Vec3d(0.0, 0.0, 0.0),
            1.0, orientation, inertia);

        // Custom constaint addition
        pbdModel->getConfig()->addPbdConstraintFunctor([&](PbdConstraintContainer& container)
            {
                auto hingeConstraint = std::make_shared<PbdHingeJointConstraint>();
                hingeConstraint->initConstraint({ rigidPbdObj->getPbdBody()->bodyHandle, 0 }, Vec3d(1.0, 0.0, 0.0), 30.0);
                container.addConstraint(hingeConstraint);
            });
    }
    scene->addSceneObject(rigidPbdObj);

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
        //viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
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

        driver->start();
    }
}

int
main()
{
    tissueCapsuleDrop();
    //planeContactScene();
    //hingeScene();

    return 0;
}