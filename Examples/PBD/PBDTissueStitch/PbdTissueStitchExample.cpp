/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectStitching.h"
#include "imstkPointwiseMap.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

// #define USE_THIN_TISSUE

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTetTissueObj(const std::string& name,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEMTet constraints (42k - 85k for tissue, but we want
    // something much more stretchy to wrap)
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4; // 0.48 for tissue
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    /* pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 0.01);
     pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.4);*/
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_linearDampingCoeff = 0.05;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setVisualGeometry(surfMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->addComponent<Collider>()->setGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->uniformMassValue = 0.00001;
    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    return tissueObj;
}

///
/// \brief Creates thin tissue object
///
static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh =
        GeometryUtils::toTriangleGrid(center, size, dim,
        Quatd::Identity(), 1.0);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6);
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_linearDampingCoeff = 0.025;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));

    // Setup the Object
    tissueObj->setVisualGeometry(triMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(triMesh);
    tissueObj->addComponent<Collider>()->setGeometry(triMesh);
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->uniformMassValue = 0.00001;
    // Fix the borders
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++)
        {
            if (x == 0)
            {
                tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * y);
            }
        }
    }

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    auto toolGeom    = std::make_shared<LineMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeom->initialize(verticesPtr, indicesPtr);

    auto toolObj = std::make_shared<RigidObject2>("ToolObj");
    toolObj->setVisualGeometry(toolGeom);
    toolObj->addComponent<Collider>()->setGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    auto rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.3;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    // Add a component for controller via another device
    auto controller = toolObj->addComponent<RigidObjectController>();
    controller->setControlledObject(toolObj);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.0045);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    return toolObj;
}

///
/// \brief This example demonstrates stitching interaction with pbd tissues
///
int
main()
{
    const double capsuleRadius = 0.02;
    const double tissueLength  = 0.15;

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueStitch");
    scene->getActiveCamera()->setPosition(0.0012, 0.0451, 0.1651);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tet tissue
#ifdef USE_THIN_TISSUE
    std::shared_ptr<PbdObject> tissueObj = makeTriTissueObj("Tissue",
        Vec2d(tissueLength, 0.07), Vec2i(15, 5),
        Vec3d(tissueLength * 0.5, -0.01 - capsuleRadius, 0.0));
#else
    std::shared_ptr<PbdObject> tissueObj = makeTetTissueObj("Tissue",
        Vec3d(tissueLength, 0.01, 0.07), Vec3i(15, 2, 5),
        Vec3d(tissueLength * 0.5, -0.01 - capsuleRadius, 0.0));
#endif
    scene->addSceneObject(tissueObj);

    // Setup a capsule to wrap around
    auto cdObj       = std::make_shared<SceneObject>("collisionObject");
    auto capsuleGeom = std::make_shared<Capsule>();
    capsuleGeom->setPosition(0.0, 0.0, 0.0);
    capsuleGeom->setRadius(capsuleRadius);
    capsuleGeom->setLength(0.08);
    capsuleGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    auto cdObjVisualModel = cdObj->addComponent<VisualModel>();
    cdObjVisualModel->setGeometry(capsuleGeom);
    cdObjVisualModel->getRenderMaterial()->setColor(
        Color(246.0 / 255.0, 127.0 / 255.0, 123.0 / 255.0));
    cdObj->addComponent<Collider>()->setGeometry(capsuleGeom);
    scene->addSceneObject(cdObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

    // Setup CD with a cylinder CD object
    auto collisionInteraction = std::make_shared<PbdObjectCollision>(tissueObj, cdObj);
    collisionInteraction->setFriction(0.0);
    collisionInteraction->setDeformableStiffnessA(0.3);
    scene->addInteraction(collisionInteraction);

    auto stitching = std::make_shared<PbdObjectStitching>(tissueObj);
    stitching->setStitchDistance(0.015);
    scene->addInteraction(stitching);

    // Lights
    auto light1 = std::make_shared<DirectionalLight>();
    light1->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light1->setIntensity(0.5);
    scene->addLight("light1", light1);
    auto light2 = std::make_shared<DirectionalLight>();
    light2->setFocalPoint(Vec3d(-5.0, -8.0, -5.0));
    light2->setIntensity(0.5);
    scene->addLight("light2", light2);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.001, 0.001, 0.001);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d& pos = viewer->getMouseDevice()->getPos();
#ifdef USE_THIN_TISSUE
                deviceClient->setPosition(Vec3d(40.0, 40.0, -(pos[1] * 100.0 - 50.0)));
                deviceClient->setOrientation(Quatd(Rotd(-0.6, Vec3d(0.0, 0.0, 1.0))));
#else
                deviceClient->setPosition(Vec3d(37.0, 0.0, -(pos[1] * 100.0 - 50.0)));
                deviceClient->setOrientation(Quatd(Rotd(0.65, Vec3d(0.0, 0.0, 1.0))));
#endif
            });
#endif

        auto controller = toolObj->getComponent<RigidObjectController>();
        controller->setDevice(deviceClient);

#ifdef iMSTK_USE_HAPTICS
        connect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    auto toolGeom   = std::dynamic_pointer_cast<LineMesh>(toolObj->getComponent<Collider>()->getGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginStitch(v1, (v2 - v1).normalized());
                }
            });
#endif

        double t = 0.0;
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                // Toggle gravity
                if (e->m_key == 'g')
                {
                    Vec3d& g = tissueObj->getPbdModel()->getConfig()->m_gravity;
                    g = Vec3d(0.0, -static_cast<double>(!(g.norm() > 0.0)), 0.0);
                }
                // Perform stitch
                else if (e->m_key == 's')
                {
                    auto toolGeom   = std::dynamic_pointer_cast<LineMesh>(toolObj->getComponent<Collider>()->getGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginStitch(v1, (v2 - v1).normalized());
                }
                // Reset
                else if (e->m_key == 'r')
                {
                    t = 0.0;
                }
            });

        // Record the intial positions
        std::vector<Vec3d> initPositions;

        auto pointMesh =
            std::dynamic_pointer_cast<PointSet>(tissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointMesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;
        const std::vector<int>                   fixedNodes  = tissueObj->getPbdBody()->fixedNodeIds;
        for (size_t i = 0; i < fixedNodes.size(); i++)
        {
            initPositions.push_back(vertices[fixedNodes[i]]);
        }
        bool stopped = false;

        // Script the movement of the tissues fixed points
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const double dt = sceneManager->getDt();
                t += dt;
                if (t < 10.5)
                {
                    for (size_t i = 0; i < fixedNodes.size(); i++)
                    {
                        Vec3d initPos = initPositions[i];
                        Vec3d& pos    = vertices[fixedNodes[i]];

                        const double r = (capsuleGeom->getPosition().head<2>() - initPos.head<2>()).norm();
                        pos = Vec3d(-sin(t) * r, -cos(t) * r, initPos[2]);
                    }
                }
                else
                {
                    if (!stopped)
                    {
                        // Clear and reinit all constraints (new resting lengths)
                        stopped = true;
                        tissueObj->getPbdBody()->fixedNodeIds.clear();
                        tissueObj->getPbdModel()->initialize();
                    }
                }
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}