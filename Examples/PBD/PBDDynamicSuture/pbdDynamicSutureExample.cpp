/*=========================================================================

	Library: iMSTK

	Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
	& Imaging in Medicine, Rensselaer Polytechnic Institute.

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0.txt

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

=========================================================================*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"

#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkPointLight.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include "NeedleInteraction.h"
#include "NeedleObject.h"

#include "imstkNew.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkRigidObjectController.h"
#endif

using namespace imstk;

// Create tissue object to stitch
std::shared_ptr<PbdObject>
createTissueHole(std::shared_ptr<TetrahedralMesh> tetMesh)
{
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    std::shared_ptr<SurfaceMesh> tissueHoleVisual = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "Tissues/tissue_hole_surf.obj");

    auto pbdObject = std::make_shared<PbdObject>("meshHole");
    auto pbdParams = std::make_shared<PbdModelConfig>();

    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 5.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 100.0);
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.01;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.3;

    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {
        auto position = tetMesh->getVertexPosition(vert_id);
        if (std::fabs(1.40984 - std::fabs(position(1))) <= 1E-4)
        {
            pbdParams->m_fixedNodeIds.push_back(vert_id);
        }
    }

    tetMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    surfMesh->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    surfMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);

    tetMesh->scale(0.018, Geometry::TransformType::ApplyToData); // 0.015
    surfMesh->scale(0.018, Geometry::TransformType::ApplyToData);

    tissueHoleVisual->rotate(Vec3d(0.0, 0.0, 1.0), -PI_2, Geometry::TransformType::ApplyToData);
    tissueHoleVisual->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2 / 1.0, Geometry::TransformType::ApplyToData);
    tissueHoleVisual->scale(0.018, Geometry::TransformType::ApplyToData);

    surfMesh->computeVertexNormals();
    surfMesh->computeTrianglesNormals();

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    // Add a visual model to render the surface of the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tissueHoleVisual);
    visualModel->setRenderMaterial(material);
    pbdObject->addVisualModel(visualModel);

    // Setup the Object
    pbdObject->setPhysicsGeometry(tetMesh);
    pbdObject->setCollidingGeometry(surfMesh);
    pbdObject->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    auto map = std::make_shared<PointwiseMap>(tetMesh, tissueHoleVisual);
    map->setTolerance(0.01);
    pbdObject->setPhysicsToVisualMap(map);
    pbdObject->setDynamicalModel(pbdModel);

    return pbdObject;
}

static std::shared_ptr<SceneObject>
makeClampObj(std::string name)
{
    auto surfMesh =
        MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Clamps/Gregory Suture Clamp/gregory_suture_clamp.obj");

    surfMesh->scale(5.0, Geometry::TransformType::ApplyToData);

    auto toolObj = std::make_shared<SceneObject>(name);
    toolObj->setVisualGeometry(surfMesh);
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(Color::LightGray);
    renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
    renderMaterial->setRoughness(0.5);
    renderMaterial->setMetalness(1.0);
    toolObj->getVisualModel(0)->setRenderMaterial(renderMaterial);

    return toolObj;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength)
{
    imstkNew<PbdObject> stringObj(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 50.0);
    pbdParams->enableBendConstraint(0.2, 1);
    pbdParams->m_fixedNodeIds     = { 0, 1 };
    pbdParams->m_uniformMassValue = 0.0001 / numVerts; // 0.002 / numVerts; // grams
    pbdParams->m_gravity = Vec3d(0.0, 0.01, 0.0);
    pbdParams->m_dt      = 0.001;                      // Overwritten for real time

    // Very important parameter for stability of solver, keep lower than 1.0:
    pbdParams->m_contactStiffness = 0.01;

    // Requires large amounts of iterations the longer, a different
    // solver would help
    pbdParams->m_iterations = 30;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(stringMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(18.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);

    return stringObj;
}

///
/// \brief This example demonstrates suturing of a hole in a tissue
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Construct the scene
    auto scene = std::make_shared<Scene>("DynamicSuture");

    scene->getActiveCamera()->setPosition(0.0, 0.04, 0.09);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.02, 0.05);
    scene->getActiveCamera()->setViewUp(0.001, 1.0, -0.4);

    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Load a tetrahedral mesh
    std::shared_ptr<TetrahedralMesh> tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "Tissues/tissue_hole.vtk");
    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    // Mesh with hole for suturing
    std::shared_ptr<PbdObject> tissueHole = createTissueHole(tetMesh);
    scene->addSceneObject(tissueHole);

    // Create arced needle
    auto needleObj = std::make_shared<NeedleObject>();
    needleObj->setForceThreshold(0.0001);
    scene->addSceneObject(needleObj);

    // Create the suture pbd-based string
    const double               stringLength      = 0.12;
    const int                  stringVertexCount = 70;
    std::shared_ptr<PbdObject> sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength);
    scene->addSceneObject(sutureThreadObj);

    // Add needle constraining behaviour between the tissue & arc needle/thread
    auto sutureInteraction = std::make_shared<NeedleInteraction>(tissueHole, needleObj, sutureThreadObj);
    scene->addInteraction(sutureInteraction);

    // Add thread CCD
    std::shared_ptr<PbdObjectCollision> interactionCCDThread = std::make_shared<PbdObjectCollision>(sutureThreadObj, sutureThreadObj, "LineMeshToLineMeshCCD");
    interactionCCDThread->setFriction(0.0);
    auto colSolver = std::dynamic_pointer_cast<PbdCollisionHandling>(interactionCCDThread->getCollisionHandlingAB())->getCollisionSolver();

    // Set the number of iterations for the CCD solver.
    colSolver->setCollisionIterations(100);
    scene->addInteraction(interactionCCDThread);

    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause();         // Start simulation paused

        // Setup a simulation manager to manage renders & scene updates
        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);         // 1ms, 1000hz

        auto hapticManager = std::make_shared<HapticDeviceManager>();
        hapticManager->setSleepDelay(0.01);         // Delay for 1ms (haptics thread is limited to max 1000hz)

        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        auto hapController = std::make_shared<RigidObjectController>(needleObj, deviceClient);
        hapController->setTranslationScaling(0.002);
        hapController->setLinearKs(10000.0);
        hapController->setAngularKs(100000000.0);
        hapController->setUseCritDamping(true);
        hapController->setForceScaling(0.00001);
        hapController->setSmoothingKernelSize(10);
        hapController->setUseForceSmoothening(true);
        scene->addController(hapController);

        // Update the needle opbject for real time
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                sutureThreadObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        // Constrain the first two vertices of the string to the needle
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getPhysicsGeometry());
                auto sutureLineMesh = std::dynamic_pointer_cast<LineMesh>(sutureThreadObj->getPhysicsGeometry());
                (*sutureLineMesh->getVertexPositions())[1] = (*needleLineMesh->getVertexPositions())[0];
                (*sutureLineMesh->getVertexPositions())[0] = (*needleLineMesh->getVertexPositions())[1];
            });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                // Perform stitch
                if (e->m_key == 's')
                {
                    sutureInteraction->stitch();
                }
            });

        driver->start();
    }
    return 0;
}