/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkObjectControllerGhost.h"
#include "imstkOrientedBox.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPlane.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"

#include "imstkPbdDistanceConstraint.h"
#include "imstkPbdRigidObjectGrasping.h"

#include "imstkCapsule.h"

#ifdef iMSTK_USE_RENDERING_VTK
#include "imstkKeyboardSceneControl.h"
#include "imstkSimulationUtils.h"
#include "imstkVTKViewer.h"
#include "imstkVTKRenderer.h"
#endif

using namespace imstk;


///
/// \brief Creates pbd simulated organ
///
std::shared_ptr<PbdObject>
makeOrgan(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    //NOTE: Replace with path to stomach
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Stomach/stomach.msh"); 
    const Vec3d center = tissueMesh->getCenter();
    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->scale(1.0, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);

    const Vec3d shift = { 0.0, 0.01, 0.0 }; // use this to offset organ posiiton
    tissueMesh->translate(shift, Geometry::TransformType::ApplyToData);

    auto surfMesh = tissueMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->addVisualModel(visualModel);
    //tissueObj->addVisualModel(labelModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setDynamicalModel(model);

    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));

    // Gallblader is about 60g
    // NOTE: Replace with parameters used in hernia
    tissueObj->getPbdBody()->uniformMassValue = 0.6 / tissueMesh->getNumVertices();

    model->getConfig()->m_femParams->m_YoungModulus = 108000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean);
    model->getConfig()->setBodyDamping(tissueObj->getPbdBody()->bodyHandle, 0.01);


    // Define box to set up boundary conditions
    // NOTE: Move this box to constrain point on the stomach
    Vec3d boxPos = { 0.0, 0.0, 0.1 }; // center of box
    Vec3d boxSize = { 0.1, 0.1, 0.15 }; // edge length of box

    // Fix the borders using constraints if point is within the defined box
    std::shared_ptr<VecDataArray<double, 3>> vertices = tissueMesh->getVertexPositions();
    for (int i = 0; i < tissueMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if( pos[0] < boxPos[0] + (boxSize[0] / 2.0) && pos[0] > boxPos[0] - (boxSize[0] / 2.0) &&
            pos[1] < boxPos[1] + (boxSize[1] / 2.0) && pos[1] > boxPos[1] - (boxSize[1] / 2.0) &&
            pos[2] < boxPos[2] + (boxSize[2] / 2.0) && pos[2] > boxPos[2] - (boxSize[2] / 2.0))
        {
            auto newPt = model->addVirtualParticle(pos, 0, Vec3d::Zero(), true);

            PbdParticleId vertex = { tissueObj->getPbdBody()->bodyHandle, i };
            auto constraint = std::make_shared<PbdDistanceConstraint>();
            constraint->initConstraint(0, newPt, vertex, 10.0);

            model->getConstraints()->addConstraint(constraint);
        }
    }

    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    tissueObj->initialize();

    return tissueObj;
}

std::shared_ptr<PbdObject>
makeTool(std::shared_ptr<DeviceClient> deviceClient)
{
    // The visual geometry is the scissor mesh read in from file
    auto rbdObj = std::make_shared<PbdObject>();
    auto model  = std::make_shared<PbdModel>();
    model->getConfig()->m_dt      = 0.001;
    model->getConfig()->m_gravity = Vec3d::Zero();
    rbdObj->setDynamicalModel(model);
    rbdObj->getPbdBody()->setRigid(
        Vec3d(0.0, 0.05, 0.0),            // Position
        7.0,                              // Mass
        Quatd::Identity(),                // Orientation
        Mat3d::Identity() * 100000000.0); // Inertia

    auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scissors/Metzenbaum Scissors/Metz_Scissors.stl");
    rbdObj->setCollidingGeometry(surfMesh);
    rbdObj->setVisualGeometry(surfMesh);
    rbdObj->setPhysicsGeometry(surfMesh);

    std::shared_ptr<RenderMaterial> mat = rbdObj->getVisualModel(0)->getRenderMaterial();
    mat->setShadingModel(RenderMaterial::ShadingModel::PBR);
    mat->setRoughness(0.5);
    mat->setMetalness(1.0);
    mat->setIsDynamicMesh(false);

    // Add a component for controlling via another device
    auto controller = rbdObj->addComponent<PbdObjectController>();
    controller->setControlledObject(rbdObj);
    controller->setDevice(deviceClient);
    controller->setTranslationOffset(Vec3d(0.0, 0.05, 0.0));
    controller->setLinearKs(50000.0);
    controller->setAngularKs(1000000000000.0);
    controller->setTranslationScaling(1.0);
    controller->setForceScaling(0.005);
    controller->setSmoothingKernelSize(10);
    controller->setUseForceSmoothening(true);
    controller->setUseCritDamping(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = rbdObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);
    return rbdObj;
}


///
/// \brief Creates capsule to use as a tool
///
static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model, std::shared_ptr<DeviceClient> deviceClient)
{
    double radius = 0.005;
    double length = 0.2;
    double mass = 0.02;

    auto toolGeometry = std::make_shared<Capsule>();
    // auto toolGeometry = std::make_shared<Sphere>();
    toolGeometry->setRadius(radius);
    toolGeometry->setLength(length);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    LOG(INFO) << "Tool Radius  = " << radius;
    LOG(INFO) << "Tool mass = " << mass;

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.04, 0.0, 0.0),
        mass,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(1.0);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    controller->setDevice(deviceClient);
    controller->setHapticOffset(Vec3d(0.0, 0.0, -0.1));
    controller->setTranslationScaling(1.0);
    controller->setLinearKs(1000.0);
    controller->setAngularKs(10000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(1.0);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}
///
/// \brief This example demonstrates the concept of virtual coupling
/// for haptic interaction.
/// NOTE: Requires GeoMagic Touch device or Haply Inverse 3
///
int
main(int argc, char* argv[])
{
    int deviceCount = 1;
    if (argc > 1)
    {
        std::string arg(argv[1]);
        std::cout << "Device count: " << arg << std::endl;
        deviceCount = std::min(2, std::max(0, std::stoi(arg)));
    }

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup default haptics manager
    std::shared_ptr<DeviceManager>             hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::vector<std::string>                   deviceName    = { "Right Device", "Left Device" };
    std::vector<std::shared_ptr<DeviceClient>> deviceClients;

    auto                            pbdModel = std::make_shared<PbdModel>();
    std::shared_ptr<PbdModelConfig> pbdParams = pbdModel->getConfig();
    pbdParams->m_gravity = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt = 0.002;
    pbdParams->m_iterations = 1;
    pbdParams->m_linearDampingCoeff = 0.03;

    for (int i = 0; i < deviceCount; i++)
    {
        deviceClients.push_back(hapticManager->makeDeviceClient(deviceName[i]));
    }

    // Scene
    auto scene = std::make_shared<Scene>("VirtualCoupling");
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.2, 0.35));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, 0.0, 0.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    std::shared_ptr<CollidingObject> obstacleObjs[] =
    {
        std::make_shared<CollidingObject>("Plane"),
        // std::make_shared<CollidingObject>("Cube")
    };

    // Create a plane and cube for collision with scissors
    auto plane = std::make_shared<Plane>();
    plane->setWidth(0.4);
    obstacleObjs[0]->setVisualGeometry(plane);
    obstacleObjs[0]->setCollidingGeometry(plane);

    // Read in organ mesh and set up as PBD deformable
    std::shared_ptr<PbdObject> stomach = makeOrgan("Stomach", pbdModel);
    scene->addSceneObject(stomach);

    for (auto obj : obstacleObjs)
    {
        obj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
        scene->addSceneObject(obj);
    }

    std::vector<std::shared_ptr<PbdObject>> toolObjs;

    // Add collision between the tools and the floor
    for (auto client : deviceClients)
    {
        // auto tool = makeTool(client);
        auto tool = makeCapsuleToolObj(pbdModel, client);
        scene->addSceneObject(tool);

        toolObjs.push_back(tool);

        for (auto obj : obstacleObjs)
        {
            scene->addInteraction(std::make_shared<PbdObjectCollision>(tool, obj));
        }
    }

    // Add collision between tools and organ
    for (auto tool : toolObjs) {
        scene->addInteraction(std::make_shared<PbdObjectCollision>(stomach, tool ));
    }

    // Add collision between stomach and floor
    for (auto obj : obstacleObjs)
    {
        scene->addInteraction(std::make_shared<PbdObjectCollision>(stomach, obj));
    }

    std::vector<std::shared_ptr<PbdObjectGrasping>> grasping;

    for (auto tool : toolObjs) {
        auto grasp = std::make_shared<PbdObjectGrasping>(stomach, tool);
        grasping.push_back(grasp);
        scene->addInteraction(grasp);
    }
    

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->setPaused(true); // Start paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(hapticManager);
#ifdef iMSTK_USE_RENDERING_VTK
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        driver->addModule(viewer);
#endif
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                for (auto tool : toolObjs)
                {
                    tool->getPbdModel()->getConfig()->m_dt = driver->getDt();
                }
            });

        // grasping: NOTE: For some reason I cant set up the ButtonEvents connects in a loop. If you want both devices to grasp then copy paste and set [0] to [1]
        connect<ButtonEvent>(deviceClients[0], &DeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_buttonState == BUTTON_PRESSED)
                {
                    if (e->m_button == 1)
                    {
                        // Use a slightly larger capsule since collision prevents intersection
                        auto capsule = std::dynamic_pointer_cast<Capsule>(toolObjs[0]->getCollidingGeometry());
                        auto dilatedCapsule = std::make_shared<Capsule>(*capsule);
                        dilatedCapsule->setRadius(capsule->getRadius() * 1.1);
                        grasping[0]->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(dilatedCapsule));
                    }
                }
                else if (e->m_buttonState == BUTTON_RELEASED)
                {
                    if (e->m_button == 1)
                    {
                        grasping[0]->endGrasp();
                    }
                }
            });


        
        // Add mouse and keyboard controls to the viewer
#ifdef iMSTK_USE_RENDERING_VTK
        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);
#endif

        driver->start();
    }

    return 0;
}
