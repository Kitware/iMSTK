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
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPointwiseMap.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleEmbeddedCH.h"
#include "NeedleInteraction.h"
#include "NeedleObject.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkRigidObjectController.h"
#else
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

///
/// \brief Spherically project the texture coordinates
///
static void
setSphereTexCoords(std::shared_ptr<SurfaceMesh> surfMesh, const double uvScale)
{
    Vec3d min, max;
    surfMesh->computeBoundingBox(min, max);
    const Vec3d size   = max - min;
    const Vec3d center = (max + min) * 0.5;

    const double radius = (size * 0.5).norm();

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(surfMesh->getNumVertices());
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        Vec3d vertex = surfMesh->getVertexPosition(i) - center;

        // Compute phi and theta on the sphere
        const double theta = asin(vertex[0] / radius);
        const double phi   = atan2(vertex[1], vertex[2]);
        uvCoords[i] = Vec2f(phi / (PI * 2.0) + 0.5, theta / (PI * 2.0) + 0.5) * uvScale;
    }
    surfMesh->setVertexTCoords("tcoords", uvCoordsPtr);
}

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
    setSphereTexCoords(surfMesh, 6.0);

    model->getConfig()->m_femParams->m_YoungModulus = 420000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.48;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    imstkNew<PbdObject> tissueObj(name);
    tissueObj->addVisualModel(visualModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 100.0;
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

    // Add a visual model to render the normals of the surface
    /*imstkNew<VisualModel> normalsVisualModel(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    clothObj->addVisualModel(normalsVisualModel);*/

    return tissueObj;
}

static std::shared_ptr<NeedleObject>
makeToolObj()
{
    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 4.4, 0.0), Geometry::TransformType::ApplyToData);
    syringeMesh->scale(0.0055, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 0.1, 0.0));

    imstkNew<NeedleObject> toolObj("NeedleRbdTool");
    toolObj->setVisualGeometry(syringeMesh);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeometry, syringeMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.1, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates two-way tissue needle contact with a tetrahedral mesh.
/// Constraints are used at the tetrahedrons faces of intersection
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDTissueVolumeNeedleContact");
    scene->getActiveCamera()->setPosition(-0.00149496, 0.0562587, 0.168353);
    scene->getActiveCamera()->setFocalPoint(0.00262407, -0.026582, -0.00463737);
    scene->getActiveCamera()->setViewUp(-0.00218222, 0.901896, -0.431947);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->m_doPartitioning = false;
    pbdParams->m_dt = 0.001; // realtime used in update calls later in main
    pbdParams->m_iterations = 5;
    pbdParams->m_collisionIterations = 1;
    pbdParams->m_gravity = Vec3d::Zero();
    pbdParams->m_linearDampingCoeff  = 0.08; // Removed from velocity
    pbdParams->m_angularDampingCoeff = 0.08;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    //pbdModel->setModelGeometry(toolObj->getCollidingGeometry());
    pbdModel->configure(pbdParams);

    // Setup a tissue with surface collision geometry
    // 0.1m tissue patch 6x3x6 tet grid
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("PBDTissue", pbdModel,
        Vec3d(0.1, 0.025, 0.1), Vec3i(6, 3, 6), Vec3d(0.0, -0.03, 0.0));
    scene->addSceneObject(tissueObj);

    // Setup a tool for the user to move
    std::shared_ptr<NeedleObject> toolObj = makeToolObj();
    toolObj->setForceThreshold(15.0);
    scene->addSceneObject(toolObj);

    // Setup a debug ghost tool for virtual coupling
    auto ghostToolObj = std::make_shared<SceneObject>("ghostTool");
    {
        auto                  toolMesh = std::dynamic_pointer_cast<SurfaceMesh>(toolObj->getVisualGeometry());
        imstkNew<SurfaceMesh> toolGhostMesh;
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

    // Setup a debug polygon soup for debug contact points
    imstkNew<DebugGeometryObject> debugGeomObj;
    debugGeomObj->setLineWidth(0.1);
    scene->addSceneObject(debugGeomObj);

    // This adds both contact and puncture functionality
    auto interaction = std::make_shared<NeedleInteraction>(tissueObj, toolObj);
    scene->addInteraction(interaction);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        //hapticManager->setSleepDelay(0.01);
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(toolObj);
        controller->setDevice(hapticDeviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(5000.0);
        controller->setAngularKs(5000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.05);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(controller->getOrientation());
                toolGhostMesh->setTranslation(controller->getPosition());
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 15.0));
            });
#else
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;
                const Quatd desiredOrientation = Quatd(Rotd(0.0, Vec3d(1.0, 0.0, 0.0)));

                Vec3d virtualForce;
                {
                    const Vec3d fS = (desiredPos - toolObj->getRigidBody()->getPosition()) * 1000.0; // Spring force
                    const Vec3d fD = -toolObj->getRigidBody()->getVelocity() * 100.0;                // Spring damping

                    const Quatd dq       = desiredOrientation * toolObj->getRigidBody()->getOrientation().inverse();
                    const Rotd angleAxes = Rotd(dq);
                    const Vec3d tS       = angleAxes.axis() * angleAxes.angle() * 10000000.0;
                    const Vec3d tD       = -toolObj->getRigidBody()->getAngularVelocity() * 1000.0;

                    virtualForce = fS + fD;
                    (*toolObj->getRigidBody()->m_force)  += virtualForce;
                    (*toolObj->getRigidBody()->m_torque) += tS + tD;
                }

                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(desiredOrientation);
                toolGhostMesh->setTranslation(desiredPos);
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, virtualForce.norm() / 15.0));
        });
#endif

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

                // Copy debug geometry
                auto needleEmbeddedCH = std::dynamic_pointer_cast<NeedleEmbeddedCH>(interaction->getEmbeddingCH());
                const std::vector<Vec3d>& debugEmbeddingPts  = needleEmbeddedCH->m_debugEmbeddingPoints;
                const std::vector<Vec3i>& debugEmbeddingTris = needleEmbeddedCH->m_debugEmbeddedTriangles;
                debugGeomObj->clear();
                for (size_t i = 0; i < debugEmbeddingPts.size(); i++)
                {
                    debugGeomObj->addPoint(debugEmbeddingPts[i]);
                }
                auto verticesPtr = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry())->getVertexPositions();
                VecDataArray<double, 3>& vertices = *verticesPtr;
                for (size_t i = 0; i < debugEmbeddingTris.size(); i++)
                {
                    debugGeomObj->addTriangle(
                        vertices[debugEmbeddingTris[i][0]],
                        vertices[debugEmbeddingTris[i][1]],
                        vertices[debugEmbeddingTris[i][2]]);
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