/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkCompositeImplicitGeometry.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkOrientedBox.h"
#include "imstkPbdAngularConstraint.h"
#include "imstkPbdSystem.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPbdMethod.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphere.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates tissue object
///
static std::shared_ptr<Entity>
makeTissueObj(const std::string& name,
              std::shared_ptr<PbdSystem> system,
              const double width,
              const double height,
              const int rowCount,
              const int colCount,
              const double particleMassValue,
              const double distStiffness, const double bendStiffness)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> tissueMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(rowCount, colCount));

    // Setup the Parameters
    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, distStiffness);
    system->getConfig()->enableConstraint(PbdSystemConfig::ConstraintGenType::Dihedral, bendStiffness);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    // Setup the Object
    auto pbdObject = SceneUtils::makePbdEntity(name, tissueMesh, system);
    pbdObject->getComponent<VisualModel>()->setRenderMaterial(material);
    auto method = pbdObject->getComponent<PbdMethod>();
    method->setUniformMass(particleMassValue);
    std::vector<int> fixedNodeIds;
    for (int x = 0; x < rowCount; x++)
    {
        for (int y = 0; y < colCount; y++)
        {
            if (x == 0 || y == 0 || x == rowCount - 1 || y == colCount - 1)
            {
                fixedNodeIds.push_back(x * colCount + y);
            }
        }
    }
    method->setFixedNodes(fixedNodeIds);

    return pbdObject;
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

    auto pbdSystem = std::make_shared<PbdSystem>();
    auto pbdConfig = std::make_shared<PbdSystemConfig>();
    // Slightly larger gravity to compensate damping
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 10;
    pbdConfig->m_linearDampingCoeff  = 0.001;
    pbdConfig->m_angularDampingCoeff = 0.001;
    pbdConfig->m_doPartitioning      = false;
    pbdSystem->configure(pbdConfig);

    auto planeObj  = std::make_shared<SceneObject>("plane");
    auto planeGeom = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    planeGeom->setWidth(1.0);
    planeObj->addComponent<VisualModel>()->setGeometry(planeGeom);
    planeObj->addComponent<Collider>()->setGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // Setup a capsule
    std::shared_ptr<Entity> rigidCapsule;
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto                         rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.05, 0.25);
        std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
        rigidCapsule = SceneUtils::makePbdEntity("rigidCapsule", surfMesh, pbdSystem);
        auto renderMaterial = rigidCapsule->getComponent<VisualModel>()->getRenderMaterial();

        // Setup material
        renderMaterial->setColor(Color(0.9, 0.0, 0.0));
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        renderMaterial->setRoughness(0.5);
        renderMaterial->setMetalness(1.0);
        renderMaterial->setIsDynamicMesh(false);

        // Setup body
        auto        method      = rigidCapsule->getComponent<PbdMethod>();
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 1.0).normalized());
        method->setRigid(Vec3d(0.0, 0.2, 0.0),
            1.0, orientation, Mat3d::Identity() * 0.01);
    }
    scene->addSceneObject(rigidCapsule);

    auto collision = std::make_shared<PbdObjectCollision>(rigidCapsule, planeObj);
    collision->setRigidBodyCompliance(0.000001);
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

        {
            // Add default mouse and keyboard controls to the viewer
            std::shared_ptr<Entity> mouseAndKeyControls =
                SimulationUtils::createDefaultSceneControl(driver);
            scene->addSceneObject(mouseAndKeyControls);

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
                    rigidCapsule->getComponent<PbdMethod>()->getPbdBody()->externalForce  = extForce;
                    rigidCapsule->getComponent<PbdMethod>()->getPbdBody()->externalTorque = extTorque;
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

    auto                    scene = std::make_shared<Scene>("PbdSDFCollision");
    std::shared_ptr<Entity> cubeObj;
    {
        // This model is shared among interacting rigid bodies
        auto pbdSystem = std::make_shared<PbdSystem>();
        auto pbdConfig = std::make_shared<PbdSystemConfig>();
        // Slightly larger gravity to compensate damping
        pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
        pbdConfig->m_dt         = 0.001;
        pbdConfig->m_iterations = 10;
        pbdConfig->m_linearDampingCoeff  = 0.001;
        pbdConfig->m_angularDampingCoeff = 0.001;
        pbdConfig->m_doPartitioning      = false;
        pbdSystem->configure(pbdConfig);

        // Create the first pbd, plane floor
        auto planeObj = std::make_shared<SceneObject>("Plane");
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
            planeObj->addComponent<VisualModel>()->setGeometry(toSurfMesh.getOutputMesh());
            planeObj->addComponent<Collider>()->setGeometry(compGeom);

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
            auto material = std::make_shared<RenderMaterial>();
            material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
            material->setLineWidth(2.0);
            material->setColor(Color::Orange);

            // Create the cube rigid object
            cubeObj = SceneUtils::makePbdEntity("cube", subdivide.getOutputMesh(), pbdSystem);
            cubeObj->getComponent<VisualModel>()->setRenderMaterial(material);
            auto cubeObjMethod = cubeObj->addComponent<PbdMethod>();
            cubeObjMethod->setGeometry(subdivide.getOutputMesh());
            cubeObjMethod->setRigid(Vec3d(0.0, 0.2, 0.0), 1.0,
                Quatd(Rotd(0.4, Vec3d(1.0, 0.0, 0.0))), Mat3d::Identity() * 0.01);

            scene->addSceneObject(cubeObj);
        }

        auto pbdInteraction = std::make_shared<PbdObjectCollision>(cubeObj, planeObj, "ImplicitGeometryToPointSetCD");
        pbdInteraction->setRigidBodyCompliance(0.00001);
        pbdInteraction->setFriction(0.5);
        //pbdInteraction->setUseCorrectVelocity(false);
        scene->addInteraction(pbdInteraction);

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

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

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
                auto cubeObjMethod = cubeObj->addComponent<PbdMethod>();
                cubeObjMethod->getPbdBody()->externalForce  = extForce;
                cubeObjMethod->getPbdBody()->externalTorque = extTorque;
                scene->getActiveCamera()->setFocalPoint((*cubeObjMethod->getPbdBody()->vertices)[0]);
                scene->getActiveCamera()->setPosition((*cubeObjMethod->getPbdBody()->vertices)[0] + dx);
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

    auto pbdSystem = std::make_shared<PbdSystem>();
    auto pbdConfig = std::make_shared<PbdSystemConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.0;
    pbdConfig->m_angularDampingCoeff = 0.0;
    pbdConfig->m_doPartitioning      = false;
    pbdSystem->configure(pbdConfig);

    // Setup a tissue
    auto tissueObj = makeTissueObj("Tissue", pbdSystem,
        0.1, 0.1,
        5, 5,
        0.1,       // Per Particle Mass
        1.0, 0.2); // Distance & Bend Stiffness
    scene->addSceneObject(tissueObj);
    pbdConfig->setBodyDamping(tissueObj->getComponent<PbdMethod>()->getBodyHandle(), 0.1);

    // Setup capsule to drop on tissue
    std::shared_ptr<Entity> capsuleObj;
    {
        // Setup line geometry
        auto rigidGeom = std::make_shared<Capsule>(Vec3d(-0.005, 0.0, 0.0), 0.005, 0.015);
        capsuleObj = SceneUtils::makePbdEntity("capsule0", rigidGeom, pbdSystem);

        // Setup material
        auto renderMaterial = capsuleObj->getComponent<VisualModel>()->getRenderMaterial();
        renderMaterial->setColor(Color(0.9, 0.0, 0.0));
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        renderMaterial->setRoughness(0.5);
        renderMaterial->setMetalness(1.0);
        renderMaterial->setIsDynamicMesh(false);

        auto capsuleObjMethod = capsuleObj->getComponent<PbdMethod>();
        pbdConfig->setBodyDamping(capsuleObjMethod->getBodyHandle(), 0.04, 0.01);

        // Setup body
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 0.0).normalized());
        capsuleObjMethod->setRigid(Vec3d(0.0, 0.05, 0.0), 1.0,
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
                pbdSystem->getConfig()->m_dt = sceneManager->getDt();
            });

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

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

    auto pbdSystem = std::make_shared<PbdSystem>();
    auto pbdConfig = std::make_shared<PbdSystemConfig>();
    pbdConfig->m_gravity    = Vec3d(0.0, 0.0, 0.0); // Slightly larger gravity to compensate viscosity
    pbdConfig->m_dt         = 0.001;
    pbdConfig->m_iterations = 5;
    pbdConfig->m_linearDampingCoeff  = 0.003;
    pbdConfig->m_angularDampingCoeff = 0.003;
    pbdConfig->m_doPartitioning      = false;
    pbdSystem->configure(pbdConfig);

    // Setup a capsule
    std::shared_ptr<Entity> rigidCapsule;
    {
        // Setup line geometry
        //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
        auto                         rigidGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.5, 2.0);
        std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
        rigidCapsule = SceneUtils::makePbdEntity("rigidCapsule", surfMesh, pbdSystem);

        // Setup material
        auto renderMaterial = rigidCapsule->getComponent<VisualModel>()->getRenderMaterial();
        renderMaterial->setColor(Color(0.9, 0.0, 0.0));
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        renderMaterial->setRoughness(0.5);
        renderMaterial->setMetalness(1.0);
        renderMaterial->setIsDynamicMesh(false);

        // Setup body such that z is now pointing in -x
        const Quatd orientation = Quatd::FromTwoVectors(Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 0.0, 1.0).normalized());
        //const Quatd orientation = Quatd::Identity();
        const Mat3d inertia = Vec3d(1.0, 1.0, 100.0).asDiagonal(); // Resistance on z
        auto        rigidCapsuleMethod = rigidCapsule->getComponent<PbdMethod>();
        rigidCapsuleMethod->setRigid(Vec3d(0.0, 0.0, 0.0),
            1.0, orientation, inertia);

        // Custom constaint addition
        pbdSystem->getConfig()->addPbdConstraintFunctor([&](PbdConstraintContainer& container)
            {
                auto hingeConstraint = std::make_shared<PbdAngularHingeConstraint>();
                hingeConstraint->initConstraint({ rigidCapsuleMethod->getBodyHandle(), 0 }, Vec3d(1.0, 0.0, 0.0), 0.1);
                container.addConstraint(hingeConstraint);
            });
    }
    scene->addSceneObject(rigidCapsule);

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

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }
}

int
main()
{
    //tissueCapsuleDrop();
    planeContactScene();
    //hingeScene();
    //bowlScene();

    return 0;
}