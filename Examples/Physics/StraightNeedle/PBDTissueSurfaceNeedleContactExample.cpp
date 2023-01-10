/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNeedle.h"
#include "imstkObjectControllerGhost.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleSurfaceInteraction.h"

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
        const Vec3d vertex = surfMesh->getVertexPosition(i) - center;

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
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 6.0);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use Strain Energy constraints
    pbdParams->m_secParams->m_YoungModulus = 5.0;
    pbdParams->m_secParams->m_PoissonRatio = 0.4;
    pbdParams->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK);
    pbdParams->m_doPartitioning = true;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 9;
    pbdParams->m_linearDampingCoeff = 0.05;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
    material->setNormalStrength(0.3);

    // Add a visual model to render the surface of the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Add a visual model to render the normals of the surface
    /*imstkNew<VisualModel> normalsVisualModel(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    clothObj->addVisualModel(normalsVisualModel);*/

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->addVisualModel(visualModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->addComponent<Collider>()->setGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->uniformMassValue = 0.1;
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

    tissueObj->addComponent<Puncturable>();

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    auto                    toolGeom = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(0.0, -1.0, 0.0), Vec3d(0.0, 1.0, 0.0) };
    VecDataArray<int, 2>    cells    = { Vec2i(0, 1) };
    toolGeom->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(cells));

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->scale(0.4, Geometry::TransformType::ApplyToData);
    syringeMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 4.4, 0.0), Geometry::TransformType::ApplyToData);

    auto toolObj = std::make_shared<RigidObject2>("NeedleRbdTool");
    toolObj->setVisualGeometry(syringeMesh);
    toolObj->addComponent<Collider>()->setGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeom, syringeMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.1;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 2.0, 0.0);

    // Add a component for needle puncturing
    auto needle = toolObj->addComponent<StraightNeedle>();
    needle->setNeedleGeometry(toolGeom);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<RigidObjectController>();
    controller->setControlledObject(toolObj);
    controller->setTranslationScaling(50.0);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.0045);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

///
/// \brief This example demonstrates the tissue needle contact with a 2d surface mesh
/// The needle is constrained at the location on the surface of the 2d mesh.
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueSurfaceNeedleContact");
    scene->getActiveCamera()->setPosition(-0.06, 7.29, 11.69);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(10.0, 3.0, 10.0), Vec3i(7, 3, 6), Vec3d(0.1, -1.0, 0.0));
    scene->addSceneObject(tissueObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

    scene->addInteraction(std::make_shared<NeedleSurfaceInteraction>(tissueObj, toolObj));

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setDirection(Vec3d(5.0, -8.0, -5.0));
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
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.2 + Vec3d(0.0, 0.025, 0.0);
                const Quatd desiredOrientation = Quatd(Rotd(0.0, Vec3d(1.0, 0.0, 0.0)));

                deviceClient->setPosition(desiredPos);
            });
#endif

        auto controller = toolObj->getComponent<RigidObjectController>();
        controller->setDevice(deviceClient);

        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                //tissueObj->getPbdModel()->getParameters()->m_dt = sceneManager->getDt();
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}