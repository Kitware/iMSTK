/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
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
#include "imstkRigidObject2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

// If two-way coupling is used haptic forces can be felt when the tool
// hits the tissue
#define TWOWAY_COUPLING

// Whether to use FEM or volume+distance constraints
#define USE_FEM

#ifdef TWOWAY_COUPLING
#include "imstkPbdRigidObjectCollision.h"
#else
#include "imstkPbdObjectCollision.h"
#endif

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
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 4.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
#ifdef USE_FEM
    // Use FEMTet constraints
    pbdParams->m_femParams->m_YoungModulus = 5.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4;
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
#else
    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdConstraint::Type::Volume, 0.9);
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 0.95);
#endif
    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.1;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);
    clothObj->addVisualModel(visualModel);

    // Add a visual model to render the normals of the surface
    imstkNew<VisualModel> normalsVisualModel;
    normalsVisualModel->setGeometry(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    clothObj->addVisualModel(normalsVisualModel);

    // Setup the Object
    clothObj->setPhysicsGeometry(tissueMesh);
    clothObj->setCollidingGeometry(surfMesh);
    clothObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, 0.0, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 2.0, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    imstkNew<RigidObject2> toolObj("Tool");
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(10.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    // Due to the lack of a compliance model between PBD-RBD a complete solve is performed
    // resulting in the tool moving completely out of the PBD tissue. Meaning it is non-deformable
    // or nearly.
    // This can be cheated by not completely solving the RBD by lowering step size & iterations
    rbdModel->getConfig()->m_maxNumIterations       = 7;
    rbdModel->getConfig()->m_velocityDamping        = 0.95;
    rbdModel->getConfig()->m_angularVelocityDamping = 1.0;
    rbdModel->getConfig()->m_maxNumConstraints      = 40;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.2;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
#ifdef iMSTK_USE_OPENHAPTICS
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.8, 0.0);
#else
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);
#endif

    return toolObj;
}

///
/// \brief This example demonstrates collision interaction with a 3d pbd
/// simulated tissue (tetrahedral)
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDTissueContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(8.0, 3.0, 8.0), Vec3i(8, 3, 8), Vec3d(0.0, -1.0, 0.0));
    scene->addSceneObject(tissueObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

#ifdef TWOWAY_COUPLING
    // With PbdRigidObjectCollision we have Pbd-Rigid coupling
    // The toolObj responds to the tissue (the tool is pushed partly out of the way of the tissue whilst the
    // tissue deforms)
    auto interaction = std::make_shared<PbdRigidObjectCollision>(tissueObj, toolObj, "ClosedSurfaceMeshToMeshCD");
    std::dynamic_pointer_cast<ClosedSurfaceMeshToMeshCD>(interaction->getCollisionDetection())->setGenerateEdgeEdgeContacts(true);
    scene->addInteraction(interaction);
#else
    // With PbdObjectCollision we only have one-way coupling
    // The toolObj does not respond to the tissue (only the tissueObj responds to the tool, moving out of the way)
    auto interaction = std::make_shared<PbdObjectCollision>(tissueObj, toolObj, "ClosedSurfaceMeshToMeshCD");
    std::dynamic_pointer_cast<ClosedSurfaceMeshToMeshCD>(interaction->getCollisionDetection())->setGenerateEdgeEdgeContacts(true);
    scene->addInteraction(interaction);
#endif

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(toolObj);
        controller->setDevice(hapticDeviceClient);
        controller->setTranslationScaling(0.05);
        controller->setLinearKs(5000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0025);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);
#else
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 10.0;

                const Vec3d fS = (worldPos - toolObj->getRigidBody()->getPosition()) * 1000.0; // Spring force
                const Vec3d fD = -toolObj->getRigidBody()->getVelocity() * 100.0;              // Spring damping

                (*toolObj->getRigidBody()->m_force) += (fS + fD);
            });
#endif

        // Toggle collision interaction in scene with key y
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'y')
                {
                    if (scene->getSceneObject(interaction->getName()) != nullptr)
                    {
                        scene->removeSceneObject(interaction);
                    }
                    else
                    {
                        scene->addInteraction(interaction);
                    }
                    scene->buildTaskGraph();
                    scene->initTaskGraph();
                }
            });

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
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