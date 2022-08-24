/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollisionUtils.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCutting.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkTriangleToTetMap.h"
#include "imstkVertexLabelVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_HAPTICS
#include "imstkDeviceManager.h"
#include "imstkDeviceManagerFactory.h"
#else
#include "imstkDummyClient.h"
#endif

using namespace imstk;

///
/// \class PbdAttachmentConstraintFunctor
///
/// \brief This constraint generation functor takes an input TetrahedralMesh simulated
/// pbd object and a set of ligament points. It then finds the nearest points on the
/// TetrahedralMesh and generates constraints to attach the ligaments to the organ
///
class PbdAttachmentConstraintFunctor : public PbdBodyConstraintFunctor
{
public:
    PbdAttachmentConstraintFunctor() = default;
    ~PbdAttachmentConstraintFunctor() override = default;

    void setLigamentObj(std::shared_ptr<PbdObject> ligamentObj)
    {
        m_ligamentObj = ligamentObj;
    }

    void setGallbladerObj(std::shared_ptr<PbdObject> gallbladerObj)
    {
        m_gallbladerObj = gallbladerObj;
    }

    ///
    /// \brief Appends a set of constraint to the container given a geometry & body
    ///
    void operator()(PbdConstraintContainer& constraints) override
    {
        auto                         gallbladerTetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(m_gallbladerObj->getPhysicsGeometry());
        std::shared_ptr<SurfaceMesh> gallbladerSurfMesh = gallbladerTetMesh->extractSurfaceMesh();

        // Setup a map to figure out what tet the tri is from for attachment to the tet
        TriangleToTetMap triToTetMap;
        triToTetMap.setParentGeometry(gallbladerTetMesh);
        triToTetMap.setChildGeometry(gallbladerSurfMesh);
        triToTetMap.setTolerance(0.00001);
        triToTetMap.compute();

        auto                                     lineMesh = std::dynamic_pointer_cast<LineMesh>(m_ligamentObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> lineMeshVerticesPtr = lineMesh->getVertexPositions();

        for (int i = 0; i < lineMeshVerticesPtr->size() / 2; i++)
        {
            const Vec3d vertexOnTri = (*lineMeshVerticesPtr)[i * 2 + 1];

            // Find the corresponding triangle on the tet mesh
            double minSqrDist   = IMSTK_FLOAT_MAX;
            int    closestTriId = -1;
            for (int j = 0; j < gallbladerSurfMesh->getNumCells(); j++)
            {
                const Vec3i& cell = (*gallbladerSurfMesh->getCells())[j];
                const Vec3d& x1   = (*gallbladerSurfMesh->getVertexPositions())[cell[0]];
                const Vec3d& x2   = (*gallbladerSurfMesh->getVertexPositions())[cell[1]];
                const Vec3d& x3   = (*gallbladerSurfMesh->getVertexPositions())[cell[2]];

                int         ptOnTriangleCaseType;
                const Vec3d closestPtOnTri = CollisionUtils::closestPointOnTriangle(vertexOnTri,
                    x1, x2, x3, ptOnTriangleCaseType);
                const double sqrDist = (closestPtOnTri - vertexOnTri).squaredNorm();
                if (sqrDist < minSqrDist)
                {
                    minSqrDist   = sqrDist;
                    closestTriId = j;
                }
            }

            // Now we have the triangle, but we need the tetrahedron
            const int   tetId        = triToTetMap.getParentTetId(closestTriId);
            const Vec4d weights      = gallbladerTetMesh->computeBarycentricWeights(tetId, vertexOnTri);
            const int   gallbladerId = m_gallbladerObj->getPbdBody()->bodyHandle;

            // Constraint between point on tet to the line end
            auto                       vertToTriConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
            std::vector<PbdParticleId> ptsA = {
                { gallbladerId, (*gallbladerTetMesh->getCells())[tetId][0] },
                { gallbladerId, (*gallbladerTetMesh->getCells())[tetId][1] },
                { gallbladerId, (*gallbladerTetMesh->getCells())[tetId][2] },
                { gallbladerId, (*gallbladerTetMesh->getCells())[tetId][3] } };
            std::vector<double>        weightsA = { weights[0], weights[1], weights[2], weights[3] };
            // Ligament vertex end on the gallblader
            std::vector<PbdParticleId> ptsB     = { { m_ligamentObj->getPbdBody()->bodyHandle, i* 2 + 1 } };
            std::vector<double>        weightsB = { 1.0 };
            vertToTriConstraint->initConstraint(ptsA, weightsA, ptsB, weightsB, 0.1, 0.1);
            constraints.addConstraint(vertToTriConstraint);

            // Save this constraint for later topology changes
            m_constraintMap[i * 2 + 1] = vertToTriConstraint;
        }
    }

    ///
    /// \brief Adds constraints but only for the given vertices
    ///
    void addConstraints(
        PbdConstraintContainer&                     constraints,
        std::shared_ptr<std::unordered_set<size_t>> vertices) override
    {
        // Instead of regenerating we save the initial ones
        for (const size_t vertIdu : *vertices)
        {
            const int vertId = static_cast<int>(vertIdu);
            if (m_constraintMap.count(vertId) > 0)
            {
                constraints.addConstraint(m_constraintMap.at(vertId));
            }
        }
    }

protected:
    std::shared_ptr<PbdObject> m_ligamentObj   = nullptr;
    std::shared_ptr<PbdObject> m_gallbladerObj = nullptr;
    std::unordered_map<int, std::shared_ptr<PbdBaryPointToPointConstraint>> m_constraintMap;
};

///
/// \brief Creates pbd simulated gallblader object
///
static std::shared_ptr<PbdObject>
makeGallBlader(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    auto        tissueMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/Organs/Gallblader/gallblader.msh");
    const Vec3d center     = tissueMesh->getCenter();
    tissueMesh->translate(-center, Geometry::TransformType::ApplyToData);
    tissueMesh->rotate(Vec3d(0.0, 0.0, 1.0), 30.0 / 180.0 * 3.14, Geometry::TransformType::ApplyToData);
    tissueMesh->translate(center, Geometry::TransformType::ApplyToData);

    model->getConfig()->m_femParams->m_YoungModulus = 420000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.48;

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tissueMesh);
    visualModel->setRenderMaterial(material);

    /* auto labelModel = std::make_shared<VertexLabelVisualModel>();
     labelModel->setGeometry(tissueMesh);*/

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->addVisualModel(visualModel);
    //tissueObj->addVisualModel(labelModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setDynamicalModel(model);
    // Gallblader is about 60g
    tissueObj->getPbdBody()->uniformMassValue = 60.0 / tissueMesh->getNumVertices();
    tissueObj->getPbdBody()->fixedNodeIds     = { 72, 57, 131, 132 };
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK,
        tissueObj->getPbdBody()->bodyHandle);
    LOG(INFO) << "Per particle mass: " << tissueObj->getPbdBody()->uniformMassValue;

    return tissueObj;
}

///
/// \brief Creates pbd rbd simulated tool object with cutting plane
///
static std::shared_ptr<PbdObject>
makeToolObj(const std::string& name, std::shared_ptr<PbdModel> model)
{
    // Create a cutting plane object in the scene
    std::shared_ptr<SurfaceMesh> cutGeom =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(0.05, 0.05), Vec2i(2, 2));
    cutGeom->setTranslation(Vec3d(-10, -20, 0));
    cutGeom->updatePostTransformData();

    auto cutObj = std::make_shared<PbdObject>(name);
    cutObj->setVisualGeometry(cutGeom);
    cutObj->setPhysicsGeometry(cutGeom);
    cutObj->setCollidingGeometry(cutGeom);
    cutObj->setDynamicalModel(model);
    cutObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    cutObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);

    cutObj->getPbdBody()->setRigid(Vec3d(0.0, 0.0, 0.0), 1.0);

    return cutObj;
}

static std::shared_ptr<PbdObject>
makeLigamentObj(const std::string& name, std::shared_ptr<PbdObject> gallbladerObj,
                std::shared_ptr<PbdModel> model)
{
    auto ligamentOriginMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Organs/Gallblader/ligamentOrigins.stl");

    auto                         gallbladerTetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(gallbladerObj->getPhysicsGeometry());
    std::shared_ptr<SurfaceMesh> gallbladerSurfMesh = gallbladerTetMesh->extractSurfaceMesh();

    auto             lineMeshVerticesPtr = std::make_shared<VecDataArray<double, 3>>();
    auto             lineMeshIndicesPtr  = std::make_shared<VecDataArray<int, 2>>();
    std::vector<int> fixedPts;

    // For every vertex find the nearest point on the other mesh
    // Add a ligament line between the two points
    {
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = ligamentOriginMesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;

        SurfaceMeshDistanceTransform distTransform;
        distTransform.setInputMesh(gallbladerSurfMesh);
        distTransform.setupDistFunc();

        for (int i = 0; i < vertices.size(); i++)
        {
            const Vec3d& pos       = vertices[i];
            const Vec3d  nearestPt = distTransform.getNearestPoint(pos);
            lineMeshVerticesPtr->push_back(pos);
            lineMeshVerticesPtr->push_back(nearestPt);
            lineMeshIndicesPtr->push_back(Vec2i(i * 2, i * 2 + 1));
            fixedPts.push_back(i * 2);
        }
    }

    auto lineMesh = std::make_shared<LineMesh>();
    lineMesh->initialize(lineMeshVerticesPtr, lineMeshIndicesPtr);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setLineWidth(2.0);
    material->setColor(Color::Blood);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(lineMesh);
    visualModel->setRenderMaterial(material);

    /*auto labelModel = std::make_shared<VertexLabelVisualModel>();
    labelModel->setGeometry(lineMesh);*/

    // Setup the Object
    auto ligamentObj = std::make_shared<PbdObject>(name);
    ligamentObj->addVisualModel(visualModel);
    //ligamentObj->addVisualModel(labelModel);
    ligamentObj->setPhysicsGeometry(lineMesh);
    ligamentObj->setDynamicalModel(model);
    // Gallblader is about 60g
    ligamentObj->getPbdBody()->uniformMassValue = 0.23;
    ligamentObj->getPbdBody()->fixedNodeIds     = fixedPts;

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10000.0,
        ligamentObj->getPbdBody()->bodyHandle);

    // Setup constraints between the gallblader and ligaments
    auto attachmentConstraintFunctor = std::make_shared<PbdAttachmentConstraintFunctor>();
    attachmentConstraintFunctor->setLigamentObj(ligamentObj);
    attachmentConstraintFunctor->setGallbladerObj(gallbladerObj);
    attachmentConstraintFunctor->setBodyIndex(ligamentObj->getPbdBody()->bodyHandle);
    model->getConfig()->addPbdConstraintFunctor(attachmentConstraintFunctor);

    return ligamentObj;
}

///
/// \brief This example demonstrates collision interaction with a 2d pbd
/// simulated tissue/membrane/cloth
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdLigamentCutExample");
    scene->getActiveCamera()->setPosition(-0.0512055, 1.22458, 0.15584);
    scene->getActiveCamera()->setFocalPoint(-0.0313295, 1.15476, 0.0488925);
    scene->getActiveCamera()->setViewUp(-0.00236634, 0.837158, -0.546956);

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.001; // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 5;
    //pbdModel->getConfig()->m_gravity = Vec3d::Zero();
    pbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
    pbdModel->getConfig()->m_linearDampingCoeff  = 0.08; // Removed from velocity
    pbdModel->getConfig()->m_angularDampingCoeff = 0.08;

    // Setup gallblader object
    std::shared_ptr<PbdObject> gallbladerObj = makeGallBlader("Gallblader", pbdModel);
    scene->addSceneObject(gallbladerObj);

    // Setup the tool with cutting plane
    std::shared_ptr<PbdObject> toolObj = makeToolObj("Tool", pbdModel);
    scene->addSceneObject(toolObj);

    // Setup ligaments
    std::shared_ptr<PbdObject> ligamentObj = makeLigamentObj("LigamentObj", gallbladerObj, pbdModel);
    scene->addSceneObject(ligamentObj);

    // Setup cutting between ligaments and plane
    auto pbdCutting = std::make_shared<PbdObjectCutting>(ligamentObj, toolObj);
    scene->addInteraction(pbdCutting);

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
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation pause

        auto driver = std::make_shared<SimulationManager>();
        driver->setDesiredDt(0.001);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
#ifdef iMSTK_USE_HAPTICS
        // Setup default haptics manager
        std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
        std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        // Queue haptic button press to be called after scene thread
        queueConnect<ButtonEvent>(deviceClient, &DeviceClient::buttonStateChanged, sceneManager,
            [&](ButtonEvent* e)
            {
                // When button 0 is pressed replace the PBD cloth with a cut one
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    pbdCutting->apply();
                }
            });
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.1) * 0.5;

                deviceClient->setPosition(worldPos);
                deviceClient->setOrientation(Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0),
                    Vec3d(1.0, 0.0, 0.0)));
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    pbdCutting->apply();
                }
            });
#endif

        auto controller = std::make_shared<PbdObjectController>();
        controller->setDevice(deviceClient);
        controller->setControlledObject(toolObj);
        controller->setTranslationOffset(Vec3d(0.0, 1.1, 0.0));
        controller->setTranslationScaling(1.0);
        controller->setForceScaling(0.0);
        controller->setLinearKs(2000.0);
        controller->setAngularKs(500.0);
        // Damping doesn't work well here. The force is applied at the start of pbd
        // Because velocities are ulimately computed after the fact from positions
        controller->setUseCritDamping(true);
        scene->addControl(controller);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControlEntity(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}