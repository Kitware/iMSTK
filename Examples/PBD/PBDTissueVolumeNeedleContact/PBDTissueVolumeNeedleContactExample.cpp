/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDebugGeometryObject.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdContactConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTextVisualModel.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleEmbeddedCH.h"
#include "NeedleInteraction.h"
#include "NeedleObject.h"

#include <sstream>

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHaplyDeviceManager.h"
#include "imstkHaplyDeviceClient.h"
#include "imstkRigidObjectController.h"
#else
#include "imstkMouseDeviceClient.h"
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              std::shared_ptr<PbdModel> model,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    model->getConfig()->m_femParams->m_YoungModulus = 50000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.48;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->setVisualGeometry(surfMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.04;
    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    return tissueObj;
}

///
/// \brief Creates a text object with text in the top right
///
static std::shared_ptr<SceneObject>
makeTextObj()
{
    auto txtVisualModel = std::make_shared<TextVisualModel>();
    txtVisualModel->setText(
        "Device Force: 0N\n"
        "Device Torque: 0Nm\n"
        "Contact Force: 0N\n"
        "Contact Torque: 0Nm");
    txtVisualModel->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
    auto obj = std::make_shared<SceneObject>();
    obj->addVisualModel(txtVisualModel);
    return obj;
}

static void
updateTxtObj(std::shared_ptr<SceneObject>         txtObj,
             std::shared_ptr<NeedleInteraction>   interaction,
             std::shared_ptr<PbdObjectController> controller)
{
    // Display the contact and device force
    double                             contactForceMag  = 0.0;
    double                             contactTorqueMag = 0.0;
    auto                               pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(interaction->getCollisionHandlingAB());
    const std::vector<PbdConstraint*>& collisionConstraints = pbdCH->getConstraints();
    if (collisionConstraints.size() > 0)
    {
        auto         pbdObj = std::dynamic_pointer_cast<PbdObject>(controller->getControlledObject());
        const double dt     = pbdObj->getPbdModel()->getConfig()->m_dt;

        auto contactConstraint = dynamic_cast<PbdContactConstraint*>(collisionConstraints[0]);

        // Multiply with gradient for direction
        contactForceMag   = std::abs(contactConstraint->getForce(dt));
        contactForceMag  *= controller->getForceScaling(); // Scale to bring in device space
        contactTorqueMag  = std::abs(contactConstraint->getTorque(dt, 0));
        contactTorqueMag *= controller->getForceScaling();
    }

    // Get a desired precision
    std::ostringstream strStream;
    strStream.precision(2);
    strStream <<
        "Device Force: " << controller->getDeviceForce().norm() << "N\n"
        "Device Torque: " << controller->getDeviceTorque().norm() << "Nm\n"
        "Contact Force (scaled): " << contactForceMag << "N\n"
        "Contact Torque (scaled): " << contactTorqueMag << "Nm";

    std::dynamic_pointer_cast<TextVisualModel>(txtObj->getVisualModel(0))->setText(strStream.str());
}

static void
updateDebugGeom(std::shared_ptr<NeedleInteraction>   interaction,
                std::shared_ptr<DebugGeometryObject> debugGeomObj)
{
    auto                      needleEmbeddedCH   = std::dynamic_pointer_cast<NeedleEmbeddedCH>(interaction->getEmbeddingCH());
    const std::vector<Vec3d>& debugEmbeddingPts  = needleEmbeddedCH->m_debugEmbeddingPoints;
    const std::vector<Vec3i>& debugEmbeddingTris = needleEmbeddedCH->m_debugEmbeddedTriangles;
    debugGeomObj->clear();
    for (size_t i = 0; i < debugEmbeddingPts.size(); i++)
    {
        debugGeomObj->addPoint(debugEmbeddingPts[i]);
    }
    auto                     tissueObj   = std::dynamic_pointer_cast<PbdObject>(interaction->getEmbeddingCH()->getInputObjectA());
    auto                     verticesPtr = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry())->getVertexPositions();
    VecDataArray<double, 3>& vertices    = *verticesPtr;
    for (size_t i = 0; i < debugEmbeddingTris.size(); i++)
    {
        debugGeomObj->addTriangle(
            vertices[debugEmbeddingTris[i][0]],
            vertices[debugEmbeddingTris[i][1]],
            vertices[debugEmbeddingTris[i][2]]);
    }
}

///
/// \brief This example demonstrates two-way linear tissue needle contact
/// with a tetrahedral mesh. No torques rendered. Constraints are used at
/// the tetrahedrons faces of intersection.
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueVolumeNeedleContact");
    scene->getActiveCamera()->setPosition(-0.00149496, 0.0562587, 0.168353);
    scene->getActiveCamera()->setFocalPoint(0.00262407, -0.026582, -0.00463737);
    scene->getActiveCamera()->setViewUp(-0.00218222, 0.901896, -0.431947);

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.001; // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 5;
    pbdModel->getConfig()->m_gravity    = Vec3d::Zero();
    pbdModel->getConfig()->m_collisionIterations = 5;

    // Setup a tissue with surface collision geometry
    // 0.1m tissue patch 6x3x6 tet grid
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("PbdTissue", pbdModel,
        Vec3d(0.1, 0.025, 0.1), Vec3i(6, 3, 6), Vec3d(0.0, -0.03, 0.0));
    scene->addSceneObject(tissueObj);

    // Setup a tool for the user to move
    auto toolObj = std::make_shared<NeedleObject>("PbdNeedle");
    toolObj->setDynamicalModel(pbdModel);
    toolObj->getPbdBody()->setRigid(Vec3d(0.0, 1.0, 0.0), 1.0,
        Quatd::Identity(), Mat3d::Identity() * 10000.0);
    scene->addSceneObject(toolObj);

    // Setup a debug ghost tool for virtual coupling
    auto ghostToolObj = std::make_shared<SceneObject>("ghostTool");
    {
        auto toolMesh      = std::dynamic_pointer_cast<SurfaceMesh>(toolObj->getVisualGeometry());
        auto toolGhostMesh = std::make_shared<SurfaceMesh>();
        toolGhostMesh->initialize(
            std::make_shared<VecDataArray<double, 3>>(*toolMesh->getVertexPositions(Geometry::DataType::PreTransform)),
            std::make_shared<VecDataArray<int, 3>>(*toolMesh->getCells()));
        ghostToolObj->setVisualGeometry(toolGhostMesh);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.3);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
    }
    scene->addSceneObject(ghostToolObj);

    // Setup a text to display forces
    std::shared_ptr<SceneObject> txtObj = makeTextObj();
    scene->addSceneObject(txtObj);

    // Setup a debug polygon soup for debug contact points
    auto debugGeomObj = std::make_shared<DebugGeometryObject>();
    debugGeomObj->setLineWidth(0.1);
    scene->addSceneObject(debugGeomObj);

    // This adds both contact and puncture functionality
    auto interaction = std::make_shared<NeedleInteraction>(tissueObj, toolObj);
    interaction->setPunctureForceThreshold(25.0);
    scene->addInteraction(interaction);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

        auto controller = std::make_shared<PbdObjectController>();
#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HaplyDeviceManager> hapticManager;
        //hapticManager->setSleepDelay(0.01);
        std::shared_ptr<HaplyDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(toolObj);
        controller->setDevice(hapticDeviceClient);
        controller->setTranslationScaling(0.2);
        controller->setTranslationOffset(Vec3d(0.05, 0.0, 0.0));
        controller->setLinearKs(5000.0);
        controller->setAngularKs(5000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.1);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(controller->getOrientation());
                toolGhostMesh->setTranslation(controller->getPosition());
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                //ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 15.0));
            });
#else
        controller->setTranslationScaling(1.0);

        auto deviceClient = std::make_shared<DummyClient>();

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;
                const Quatd desiredOrientation = Quatd(Rotd(0.0, Vec3d(1.0, 0.0, 0.0)));

                deviceClient->setPosition(desiredPos);
                deviceClient->setOrientation(desiredOrientation);

                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(desiredOrientation);
                toolGhostMesh->setTranslation(desiredPos);
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                //ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, virtualForce.norm() / 15.0));
            });
#endif
        controller->setControlledObject(toolObj);
        controller->setDevice(deviceClient);
        controller->setLinearKs(20000.0);
        controller->setAngularKs(8000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.05);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);

        int counter = 0;
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();

                // Copy constraint faces and points to debug geometry for display
                updateDebugGeom(interaction, debugGeomObj);

                // Update the force text every 100 frames
                if (counter++ % 100 == 0)
                {
                    updateTxtObj(txtObj, interaction, controller);
                    counter = 0;
                }
            });

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

    return 0;
}