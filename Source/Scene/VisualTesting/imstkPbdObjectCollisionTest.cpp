/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkCollisionDataDebugModel.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkCylinder.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkLineMesh.h"
#include "imstkOrientedBox.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPlane.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTestingUtils.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTetTissueObj(const std::string& name,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                 const Quatd& orientation,
                 bool useTetCollisionGeometry)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_linearDampingCoeff = 0.025;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setPhysicsGeometry(tetMesh);
    if (useTetCollisionGeometry)
    {
        tissueObj->setVisualGeometry(tetMesh);
        tissueObj->addComponent<Collider>()->setGeometry(tetMesh);
    }
    else
    {
        std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();
        tissueObj->setVisualGeometry(surfMesh);
        tissueObj->addComponent<Collider>()->setGeometry(surfMesh);
        tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    }
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->uniformMassValue = 0.01;

    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.45; // 0.48 for tissue
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK,
        tissueObj->getPbdBody()->bodyHandle);

    return tissueObj;
}

///
/// \brief Creates thin tissue object
///
static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center,
                 const Quatd& orientation)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh =
        GeometryUtils::toTriangleGrid(center, size, dim, orientation);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
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

    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
        tissueObj->getPbdBody()->bodyHandle);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6,
        tissueObj->getPbdBody()->bodyHandle);

    return tissueObj;
}

///
/// \brief Creates a line thread object
///
static std::shared_ptr<PbdObject>
makeLineThreadObj(const std::string& name,
                  const double length, const int dim, const Vec3d start,
                  const Vec3d& dir)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> lineMesh =
        GeometryUtils::toLineGrid(start, dir, length, dim);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
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
    material->setLineWidth(3.0);
    material->setEdgeColor(Color(0.87, 0.63, 0.44));

    // Setup the Object
    tissueObj->setVisualGeometry(lineMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(lineMesh);
    tissueObj->addComponent<Collider>()->setGeometry(lineMesh);
    tissueObj->setDynamicalModel(pbdModel);
    tissueObj->getPbdBody()->uniformMassValue = 0.00001;

    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
        tissueObj->getPbdBody()->bodyHandle);

    return tissueObj;
}

class PbdObjectCollisionTest : public VisualTest
{
public:
    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        ASSERT_NE(m_pbdObj, nullptr) << "Missing a pbdObj for PbdObjectCollisionTest";
        m_pbdObj->getPbdModel()->getConfig()->m_doPartitioning = false;
        auto pointSet = std::dynamic_pointer_cast<PointSet>(m_pbdObj->getPhysicsGeometry());
        m_currVerticesPtr = pointSet->getVertexPositions();
        m_prevVertices    = *m_currVerticesPtr;
        m_scene->addSceneObject(m_pbdObj);

        ASSERT_NE(m_collidingGeometry, nullptr);
        m_cdObj = std::make_shared<SceneObject>("obj2");
        auto visualModel = m_cdObj->addComponent<VisualModel>();
        visualModel->setGeometry(m_collidingGeometry);
        m_cdObj->addComponent<Collider>()->setGeometry(m_collidingGeometry);
        visualModel->getRenderMaterial()->setBackFaceCulling(false);
        m_scene->addSceneObject(m_cdObj);

        m_pbdCollision = std::make_shared<PbdObjectCollision>(m_pbdObj, m_cdObj, m_collisionName);
        m_pbdCollision->setFriction(m_friction);
        m_pbdCollision->setRestitution(m_restitution);
        m_pbdCollision->setDeformableStiffnessA(m_collisionStiffness);
        std::dynamic_pointer_cast<PbdCollisionHandling>(m_pbdCollision->getCollisionHandlingA())->setEnableBoundaryCollisions(true);
        // Debug geometry to visualize collision data
        m_cdDebugModel = m_pbdCollision->addComponent<CollisionDataDebugModel>();
        m_cdDebugModel->setInputCD(m_pbdCollision->getCollisionDetection()->getCollisionData());
        m_cdDebugModel->setPrintContacts(m_printContacts);
        m_scene->addInteraction(m_pbdCollision);

        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                m_cdDebugModel->debugUpdate();
                if (m_pauseOnContact && !m_timerPaused
                    && m_cdDebugModel->getInputCD()->elementsA.size() > 0)
                {
                    m_timerPaused = true;
                    m_sceneManager->pause();
                }
            });
        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_pbdObj->getPbdModel()->getConfig()->m_dt =
                    m_sceneManager->getDt() * 0.5;
            });

        // Assert the vertices stay within bounds and below min displacement
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const VecDataArray<double, 3>& vertices = *m_currVerticesPtr;
                // Assert to avoid hitting numerous times
                ASSERT_TRUE(assertBounds(vertices, m_assertionBoundsMin, m_assertionBoundsMax));
                ASSERT_TRUE(assertMinDisplacement(m_prevVertices, vertices, 0.01));
                m_prevVertices = vertices;
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<PbdObject> m_pbdObj = nullptr;
    std::shared_ptr<Entity>    m_cdObj  = nullptr;
    std::shared_ptr<Geometry>  m_collidingGeometry = nullptr;

    std::shared_ptr<PbdObjectCollision> m_pbdCollision = nullptr;
    std::string m_collisionName      = "";
    double      m_friction           = 0.0;
    double      m_restitution        = 0.0;
    double      m_collisionStiffness = 0.5;
    std::shared_ptr<CollisionDataDebugModel> m_cdDebugModel = nullptr;

    // For assertions
    std::shared_ptr<VecDataArray<double, 3>> m_currVerticesPtr;
    VecDataArray<double, 3> m_prevVertices;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;

    Vec3d m_assertionBoundsMin = Vec3d(-1.0, -1.0, -1.0);
    Vec3d m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);
};

///
/// \brief Test PbdObjectCollision with line on line CCD
/// This test is currently disabled as there is an edge case at the vertex that
/// causes the test to indeterminstically fail according to some floating point
/// error
///
TEST_F(PbdObjectCollisionTest, DISABLED_PbdTissue_LineMeshToLineMeshCCD)
{
    // Setup the tissue
    m_pbdObj = makeLineThreadObj("Thread",
        0.2, 4, Vec3d(0.0, 0.05, -0.1), Vec3d(0.0, 0.0, 1.0));

    // Setup the geometry
    auto                    lineMesh = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(-0.5, 0.0, 0.0), Vec3d(0.5, 0.0, 0.0) };
    VecDataArray<int, 2>    indices  = { Vec2i(0, 1) };
    lineMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));
    m_collidingGeometry = lineMesh;

    m_collisionName      = "LineMeshToLineMeshCCD";
    m_friction           = 0.0;
    m_restitution        = 0.0;
    m_collisionStiffness = 0.1;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();

    runFor(2.0, 0.01);
}

///
/// \brief Test PbdObjectCollision code path with and without mapping of collision geometry
///@{
TEST_F(PbdObjectCollisionTest, PbdTissue_TetNoMapping)
{
    // Setup the tissue without mapping
    m_pbdObj = makeTetTissueObj("Tissue",
        Vec3d(0.1, 0.05, 0.1), Vec3i(3, 2, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))), true);

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}
TEST_F(PbdObjectCollisionTest, PbdTissue_TetMapping)
{
    // Setup the tissue with mapping
    m_pbdObj = makeTetTissueObj("Tissue",
        Vec3d(0.1, 0.05, 0.1), Vec3i(3, 2, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))), false);

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}
///@}

///
/// \brief Test edge case with multiple conflicting contacts
///
TEST_F(PbdObjectCollisionTest, PbdTissue_Crevice)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto                    surfMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> vertices(4);
    vertices[0] = Vec3d(0.0, -0.1, -0.25);
    vertices[1] = Vec3d(0.0, -0.1, 0.25);
    vertices[2] = Vec3d(0.25, 0.2, 0.0);
    vertices[3] = Vec3d(-0.25, 0.2, 0.0);
    VecDataArray<int, 3> indices(2);
    indices[0] = Vec3i(0, 1, 2);
    indices[1] = Vec3i(0, 3, 1);
    surfMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 3>>(indices));
    m_collidingGeometry = surfMesh;

    m_collisionName = "ClosedSurfaceMeshToMeshCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(4.0);
}

///
/// \brief Test ClosedSurfaceMeshToMeshCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_ClosedSurfaceMeshToMeshCD_LineMeshVsSurfMesh)
{
    // Setup the tissue
    m_pbdObj = makeLineThreadObj("Thread",
        0.1, 3, Vec3d::Zero(), Vec3d(1.0, 1.0, 1.0));

    // Setup the geometry
    auto                    surfMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> vertices(3);
    vertices[0] = Vec3d(-0.25, -0.1, 0.25);
    vertices[1] = Vec3d(0.25, -0.1, 0.25);
    vertices[2] = Vec3d(0.0, -0.1, -0.25);
    VecDataArray<int, 3> indices(1);
    indices[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 3>>(indices));
    m_collidingGeometry = surfMesh;

    m_collisionName = "ClosedSurfaceMeshToMeshCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test ClosedSurfaceMeshToMeshCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_ClosedSurfaceMeshToMeshCD_SurfMeshVsSurfMesh)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto                    surfMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> vertices(3);
    vertices[0] = Vec3d(-0.25, -0.1, 0.25);
    vertices[1] = Vec3d(0.25, -0.1, 0.25);
    vertices[2] = Vec3d(0.0, -0.1, -0.25);
    VecDataArray<int, 3> indices(1);
    indices[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 3>>(indices));
    m_collidingGeometry = surfMesh;

    m_collisionName = "ClosedSurfaceMeshToMeshCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test SurfaceMeshToSphereCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_SurfaceMeshToSphereCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.3, 0.3), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Sphere>();
    implicitGeom->setPosition(0.0, -0.3, 0.0);
    implicitGeom->setRadius(0.2);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "SurfaceMeshToSphereCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test SurfaceMeshToCapsuleCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_SurfaceMeshToCapsuleCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.3, 0.3), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>();
    implicitGeom->setPosition(0.0, -0.2, 0.0);
    implicitGeom->setRadius(0.1);
    implicitGeom->setLength(0.1);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "SurfaceMeshToCapsuleCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToSphereCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_PointSetToSphereCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Sphere>();
    implicitGeom->setPosition(0.0, -0.3, 0.0);
    implicitGeom->setRadius(0.2);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToSphereCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToOrientedBoxCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_PointSetToOrientedBoxCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<OrientedBox>();
    implicitGeom->setOrientation(Quatd(Rotd(-0.2, Vec3d(0.0, 0.0, -1.0))));
    implicitGeom->setPosition(-0.1, -0.2, 0.0);
    implicitGeom->setExtents(Vec3d(0.125, 0.1, 0.1));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToOrientedBoxCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(3.0);
}

///
/// \brief Test ImplicitGeometryToPointSetCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_ImplicitGeometryToPointSetCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "ImplicitGeometryToPointSetCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToPlaneCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_PointSetToPlaneCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test ImplicitGeometryToPointSetCCD with PbdObjectCollision
/// \todo Doesn't work yet
///
TEST_F(PbdObjectCollisionTest, DISABLED_PbdTissue_ImplicitGeometryToPointSetCCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "ImplicitGeometryToPointSetCCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToCapsuleCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_PointSetToCapsuleCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>();
    implicitGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(0.0, 0.0, -1.0))));
    implicitGeom->setPosition(0.0, -0.15, 0.0);
    implicitGeom->setRadius(0.1);
    implicitGeom->setLength(0.1);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToCapsuleCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.2, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToCylinderCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_PointSetToCylinderCD)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(4, 4), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Cylinder>();
    implicitGeom->setOrientation(Quatd(Rotd(PI_2 * 0.5, Vec3d(0.0, 0.0, -1.0))));
    implicitGeom->setPosition(0.0, -0.15, 0.0);
    implicitGeom->setRadius(0.1);
    implicitGeom->setLength(0.1);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToCylinderCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.2, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test friction capabilities of PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdTissue_Friction)
{
    // Setup the tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(-1.0, 2.0, 0.0);
    implicitGeom->setPosition(0.0, -0.05, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.2;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.2, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test LineMeshToCapsuleCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdThread_LineMeshToCapsuleCD)
{
    // Setup the thread
    m_pbdObj = makeLineThreadObj("Thread",
        0.4, 20, Vec3d(-0.2, 0.0, 0.0),
        Vec3d(1.0, 0.0, 0.0));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>();
    implicitGeom->setPosition(0.0, -0.2, 0.0);
    implicitGeom->setRadius(0.1);
    implicitGeom->setLength(1.0);
    implicitGeom->setOrientation(Quatd(0.0, 0.0, 0.707, 0.707));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "LineMeshToCapsuleCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test LineMeshToSphereCD with PbdObjectCollision
///
TEST_F(PbdObjectCollisionTest, PbdThread_LineMeshToSphereCD)
{
    // Setup the thread
    m_pbdObj = makeLineThreadObj("Thread",
        0.4, 20, Vec3d(-0.2, 0.0, 0.0),
        Vec3d(1.0, 0.0, 0.0));

    // Setup the geometry
    auto implicitGeom = std::make_shared<Sphere>();
    implicitGeom->setPosition(0.0, -0.2, 0.0);
    implicitGeom->setRadius(0.1);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "LineMeshToSphereCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}