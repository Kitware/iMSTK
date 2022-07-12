/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdSolver.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#else
#include "imstkDummyClient.h"
#include "imstkMouseDeviceClient.h"
#endif
#include "../PBDStaticSuture/NeedleObject.h"

using namespace imstk;

/// String geometry for interactive example.
const std::vector<Vec3d> selfCCDStringMesh = {
    // move right
    { 0.00, 0.02, 0 },
    { 0.01, 0.02, 0 },
    { 0.02, 0.02, 0 },
    { 0.03, 0.02, 0 },
    { 0.04, 0.02, 0 },
    { 0.05, 0.02, 0 },
    { 0.06, 0.02, 0 },
    { 0.07, 0.02, 0 },
    { 0.08, 0.02, 0 },
    { 0.09, 0.02, 0 },

    // jump (back / down / left) and then move forward
    { 0.05, 0, -0.04 },
    { 0.05, 0, -0.03 },
    { 0.05, 0, -0.02 },
    { 0.05, 0, -0.01 },
    { 0.05, 0, 0.00 },
    { 0.05, 0, 0.01 },
    { 0.05, 0, 0.02 },
    { 0.05, 0, 0.03 },
    { 0.05, 0, 0.04 }
};

///
/// \brief Create self-ccd string geometry
///
static std::shared_ptr<LineMesh>
makeSelfCCDGeometry()
{
    // Create the geometry
    imstkNew<LineMesh>                stringGeometry;
    size_t                            numVerts = selfCCDStringMesh.size();
    imstkNew<VecDataArray<double, 3>> verticesPtr(static_cast<int>(numVerts));
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    for (size_t i = 0; i < numVerts; i++)
    {
        vertices[i] = selfCCDStringMesh[i];
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 2>> segmentsPtr;
    VecDataArray<int, 2>&          segments = *segmentsPtr.get();
    for (int i = 0; i < numVerts - 1; i++)
    {
        segments.push_back(Vec2i(i, i + 1));
    }

    stringGeometry->initialize(verticesPtr, segmentsPtr);
    return stringGeometry;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(const std::string& name, const std::string& filename)
{
    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh;
    if (!filename.empty())
    {
        stringMesh = MeshIO::read<LineMesh>(filename);
    }
    else
    {
        stringMesh = makeSelfCCDGeometry();
    }

    const int numVerts = stringMesh->getNumVertices();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.0005;
    pbdParams->m_iterations = 1;
    pbdParams->m_linearDampingCoeff  = 0.03;
    pbdParams->m_collisionIterations = 25;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(4.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    imstkNew<PbdObject> stringObj(name);
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);

    if (name == "granny_knot")
    {
        stringObj->getPbdBody()->uniformMassValue = 0.0001 / numVerts; // grams
        stringObj->getPbdBody()->fixedNodeIds     = { 0, 1, stringMesh->getNumVertices() - 2, stringMesh->getNumVertices() - 1 };
        pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 200.0);
        pbdParams->enableBendConstraint(0.01, 1);
        //pbdParams->enableBendConstraint(.5, 2);
    }
    else
    {
        stringObj->getPbdBody()->fixedNodeIds = { 9, 10,
                                                  static_cast<int>(selfCCDStringMesh.size() - 2),
                                                  static_cast<int>(selfCCDStringMesh.size() - 1) };
        pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 200.0);
        pbdParams->enableBendConstraint(0.01, 1);
        //pbdParams->enableBendConstraint(.5, 2);
    }

    return stringObj;
}

///
/// \brief This example is an initial suturing example testbed. It provides the constraint
/// required for an arc shaped needle puncturing vs a static/immovable tissue. What it
/// does not do:
///  - The tissue is not deformable yet, so insertion is a bit stiff
///  - It only constrains the arc to the surface point it punctures not the volume
///  - The suture thread isn't constrained yet
///  - Ability to graps/release the needle, combining into one body
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PbdSutureSelfCCD");

    std::shared_ptr<PbdObject> threadObj =
        // makePbdString("selfCCDLine", "");
        makePbdString("granny_knot", iMSTK_DATA_ROOT "/LineMesh/granny_knot.obj");
    scene->addSceneObject(threadObj);

    auto interaction = std::make_shared<PbdObjectCollision>(threadObj, threadObj, "LineMeshToLineMeshCCD");
    // Important parameter for stability, take multiple smaller steps to resolve multiple contacts
    interaction->setDeformableStiffnessA(0.05);
    interaction->setDeformableStiffnessB(0.05);
    scene->addInteraction(interaction);

    // Create the arc needle
    imstkNew<NeedleObject> needleObj;
    needleObj->setForceThreshold(2.0);
    scene->addSceneObject(needleObj);

    // Adjust the camera
    scene->getActiveCamera()->setFocalPoint(0.022, -0.045, -0.01);
    scene->getActiveCamera()->setPosition(0.02, -0.02, 0.2);
    scene->getActiveCamera()->setViewUp(0, 1, 0);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);
        viewer->setBackgroundColors(Color(202.0 / 255.0, 212.0 / 255.0, 157.0 / 255.0));

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        // Setup a simulation manager to manage renders & scene updates
        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.0005); // 1ms, 1000hz //timestep

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> deviceManager;
        driver->addModule(deviceManager);
        std::shared_ptr<HapticDeviceClient> deviceClient       = deviceManager->makeDeviceClient();
        const double                        translationScaling = 0.001;
        const Vec3d                         offset = Vec3d(-0.02, 0.02, 0.0);
#else
        imstkNew<DummyClient> deviceClient;
        deviceClient->setOrientation(Quatd(Rotd(1.57, Vec3d(0.0, 1.0, 0.0))));
        const double translationScaling = 0.1;
        const Vec3d  offset = Vec3d(-0.02, 0.02, 0.0);

        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseMove,
            [&](MouseEvent* e)
            {
                const Vec2d& mousePos = viewer->getMouseDevice()->getPos();
                const Vec2d pos       = (mousePos - Vec2d(0.5, 0.5));
                deviceClient->setPosition(Vec3d(pos[0], pos[1], 0.0));
            });
#endif

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(needleObj);
        controller->setDevice(deviceClient);
        controller->setTranslationOffset(offset);
        controller->setTranslationScaling(translationScaling);
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0);
        scene->addControl(controller);

        // Update the thread fixed points to the controlled needle
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                auto threadLineMesh = std::dynamic_pointer_cast<LineMesh>(threadObj->getPhysicsGeometry());
                auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getPhysicsGeometry());
                (*threadLineMesh->getVertexPositions())[1] = (*needleLineMesh->getVertexPositions())[0];
                (*threadLineMesh->getVertexPositions())[0] = (*needleLineMesh->getVertexPositions())[1];
            });

        // Add mouse and keyboard controls to the viewer
        auto mouseControl = std::make_shared<MouseSceneControl>();
        mouseControl->setDevice(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        scene->addControl(mouseControl);

        auto keyControl = std::make_shared<KeyboardSceneControl>();
        keyControl->setDevice(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        scene->addControl(keyControl);

        driver->start();
    }

    return 0;
}
