/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkControllerForceText.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdSystem.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneUtils.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
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

    auto                    uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(surfMesh->getNumVertices());
    VecDataArray<float, 2>& uvCoords    = *uvCoordsPtr.get();
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
/// \brief Creates pbd simulated tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
/// \param system dynamical model the tissue should use
///
static std::shared_ptr<Entity>
makeTissueObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center,
              std::shared_ptr<PbdSystem> system)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 4.0);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Add a visual model to render the normals of the surface
    auto normalsVisualModel = std::make_shared<VisualModel>();
    normalsVisualModel->setGeometry(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, surfMesh, surfMesh, tissueMesh, system);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);
    tissueObj->addComponent(normalsVisualModel);
    auto tissueMethod = tissueObj->getComponent<PbdMethod>();
    tissueMethod->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueMethod->setUniformMass(0.05);
    // Fix the borders
    std::vector<int> fixedNodeIds;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }
    tissueMethod->setFixedNodes(fixedNodeIds);

    system->getConfig()->m_secParams->m_YoungModulus = 50.0;
    system->getConfig()->m_secParams->m_PoissonRatio = 0.4;
    system->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::NeoHookean);
    system->getConfig()->setBodyDamping(tissueMethod->getBodyHandle(), 0.001);

    return tissueObj;
}

///
/// \brief Create pbd simulated rigid tool object
/// \param system dynamical model the tool should use
///
static std::shared_ptr<Entity>
makeToolObj(std::shared_ptr<PbdSystem> system)
{
    auto                    toolGeometry = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices     = { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 2.0, 0.0) };
    VecDataArray<int, 2>    indices      = { Vec2i(0, 1) };
    toolGeometry->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    auto toolObj    = SceneUtils::makePbdEntity("Tool", toolGeometry, system);
    auto toolVisual = toolObj->getComponent<VisualModel>();
    toolVisual->getRenderMaterial()->setColor(Color::Blue);
    toolVisual->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolVisual->getRenderMaterial()->setBackFaceCulling(false);
    toolVisual->getRenderMaterial()->setLineWidth(10.0);

    auto toolMethod = toolObj->getComponent<PbdMethod>();
    system->getConfig()->setBodyDamping(toolMethod->getBodyHandle(), 0.05, 0.0);

    toolMethod->setRigid(Vec3d(0.0, 0.8, 0.0), // Position
        0.2,                                   // Mass
        Quatd::Identity(),                     // Orientation
        Mat3d::Identity() * 10.0);             // Inertia

    // Add a component for controlling via a device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolMethod, toolVisual);
    controller->setLinearKs(5000.0);
    controller->setAngularKs(10000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.0025);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

///
/// \brief This example demonstrates two-way collision interaction with a 3d
/// xpbd simulated tool and tissue (tetrahedral). With proper compliance.
///
/// If imstk is built with haptics a force is rendered. Otherwise mouse controls
/// are used. This example is currently not too scale, a force scaling is used.
///
int
PBDTissueContactExample()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup the Model/System
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_doPartitioning = false;
    pbdSystem->getConfig()->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdSystem->getConfig()->m_dt         = 0.05;
    pbdSystem->getConfig()->m_iterations = 5;

    // Setup a tissue
    auto tissueObj = makeTissueObj("Tissue",
        Vec3d(8.0, 2.0, 8.0), Vec3i(6, 5, 6), Vec3d(0.0, -1.0, 0.0), pbdSystem);
    scene->addSceneObject(tissueObj);

    // Setup a tool
    auto toolObj = makeToolObj(pbdSystem);
    scene->addSceneObject(toolObj);

    // Setup a collision
    auto collision = std::make_shared<PbdObjectCollision>(tissueObj, toolObj);
    //std::dynamic_pointer_cast<ClosedSurfaceMeshToMeshCD>(collision->getCollisionDetection())->setGenerateEdgeEdgeContacts(true);
    scene->addInteraction(collision);

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

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        auto controller = toolObj->getComponent<PbdObjectController>();
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        controller->setTranslationScaling(50.0);
        if (hapticManager->getTypeName() == "HaplyDeviceManager")
        {
            controller->setTranslationOffset(Vec3d(5.0, -5.0, 0.0));
        }
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 10.0;

                deviceClient->setPosition(worldPos);
            });

        controller->setTranslationScaling(1.0);
#endif
        controller->setDevice(deviceClient);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                pbdSystem->getConfig()->m_dt = sceneManager->getDt();
        });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        // Add something to display controller force
        auto controllerForceTxt = mouseAndKeyControls->addComponent<ControllerForceText>();
        controllerForceTxt->setController(controller);
        controllerForceTxt->setCollision(collision);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}