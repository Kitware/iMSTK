/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkDirectionalLight.h"
#include "imstkDummyClient.h"
#include "imstkGeometryUtilities.h"
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneUtils.h"
#include "imstkSceneManager.h"
#include "imstkSphere.h"
#include "imstkTestingUtils.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

static std::shared_ptr<RenderMaterial>
makeMaterial()
{
    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    return material;
}

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param size physical dimension of tissue
/// \param dim dimensions of tetrahedral grid used for tissue
/// \param center center of tissue block
/// \param useTetCollisionGeometry Whether to use a SurfaceMesh collision geometry+map
///
static std::shared_ptr<Entity>
makeTetTissueObj(const std::string& name,
                 std::shared_ptr<PbdSystem> pbdSystem,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                 const Quatd& orientation,
                 const bool useTetCollisionGeometry)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    for (int i = 0; i < tetMesh->getNumTetrahedra(); i++)
    {
        std::swap((*tetMesh->getCells())[i][2], (*tetMesh->getCells())[i][3]);
    }

    // Setup the Object
    std::shared_ptr<Entity> tissueObj;
    if (useTetCollisionGeometry)
    {
        tissueObj = SceneUtils::makePbdEntity(name, tetMesh, pbdSystem);
    }
    else
    {
        std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();
        tissueObj = SceneUtils::makePbdEntity(name, surfMesh, surfMesh, tetMesh, pbdSystem);
        auto method = tissueObj->getComponent<PbdMethod>();
        method->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    }
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(makeMaterial());
    auto method = tissueObj->getComponent<PbdMethod>();
    method->setUniformMass(0.01);

    pbdSystem->getConfig()->m_secParams->m_YoungModulus = 1000.0;
    pbdSystem->getConfig()->m_secParams->m_PoissonRatio = 0.45;     // 0.48 for tissue
    pbdSystem->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK,
                method->getBodyHandle());

    // Fix the borders
    std::vector<int> fixedNodes;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || z == 0 || x == dim[0] - 1 || z == dim[2] - 1)
                {
                    fixedNodes.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }
    method->setFixedNodes(fixedNodes);

    return tissueObj;
}

///
/// \brief Creates thin tissue object
/// \param name
/// \param size Physical dimension of tissue
/// \param dim Dimensions of triangle grid used for tissue
/// \param center Center of tissue quad
/// \param orientation Orientation of tissue plane
///
static std::shared_ptr<Entity>
makeTriTissueObj(const std::string& name,
                 std::shared_ptr<PbdSystem> pbdSystem,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center,
                 const Quatd& orientation)
{
    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh =
        GeometryUtils::toTriangleGrid(center, size, dim, orientation);

    // Setup the VisualModel
    auto material = makeMaterial();
    material->setBackFaceCulling(false);

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, triMesh, pbdSystem);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);
    auto method = tissueObj->getComponent<PbdMethod>();
    method->setUniformMass(0.00001);

    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
                method->getBodyHandle());
    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6,
                method->getBodyHandle());

    // Fix the borders
    std::vector<int> fixedNodes;
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++)
        {
            if (x == 0 || y == 0 || x == dim[0] - 1 || y == dim[1] - 1)
            {
                fixedNodes.push_back(x + dim[0] * y);
            }
        }
    }
    method->setFixedNodes(fixedNodes);

    return tissueObj;
}

///
/// \brief Creates a line thread object
/// \param name
/// \param length Length of the line
/// \param dim Divisions of the line
/// \param start Start position of the line
/// \param dir Direction the line goes
///
static std::shared_ptr<Entity>
makeLineThreadObj(const std::string& name,
                  std::shared_ptr<PbdSystem> pbdSystem,
                  const double length, const int dim, const Vec3d start,
                  const Vec3d& dir)
{
    // Setup the Geometry
    std::shared_ptr<LineMesh> lineMesh =
        GeometryUtils::toLineGrid(start, dir, length, dim);

    // Setup the VisualModel
    auto material = makeMaterial();
    material->setBackFaceCulling(false);
    material->setLineWidth(3.0);

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, lineMesh, pbdSystem);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(material);
    auto method = tissueObj->getComponent<PbdMethod>();
    method->setUniformMass(0.00001);

    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
                method->getBodyHandle());
    method->setFixedNodes({ 0, lineMesh->getNumVertices() - 1 });

    return tissueObj;
}

///
/// \brief Creates a capsule grasper object. If pbd system is provided
/// it will create a pbd object, otherwise creates a colliding/non-simulated
/// grasper
///
static std::shared_ptr<Entity>
makeGrasperObj(
    const std::string&         name,
    const Vec3d                position,
    std::shared_ptr<PbdSystem> pbdSystem = nullptr)
{
    if (pbdSystem != nullptr)
    {
        // Grasp with a capsule
        auto capsule    = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.01, 0.1);
        auto grasperObj = SceneUtils::makePbdEntity(name, capsule, pbdSystem);
        auto method     = grasperObj->getComponent<PbdMethod>();
        method->setRigid(
                        position, // Position
                        1.0);     // Mass

        auto controller = grasperObj->addComponent<PbdObjectController>();
        controller->setControlledObject(method, grasperObj->getComponent<VisualModel>());
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10.0);
        return grasperObj;
    }
    else
    {
        // Grasp with a capsule
        auto grasperObj = std::make_shared<SceneObject>(name);
        auto capsule    = std::make_shared<Capsule>(position, 0.01, 0.1);
        grasperObj->addComponent<Collider>()->setGeometry(capsule);
        grasperObj->addComponent<VisualModel>()->setGeometry(capsule);
        return grasperObj;
    }
}

class PbdObjectGraspingTest : public VisualTest
{
public:
    void SetUp() override
    {
        VisualTest::SetUp();
        m_pbdSystem = std::make_shared<PbdSystem>();
        m_pbdSystem->getConfig()->m_doPartitioning = false;
        m_pbdSystem->getConfig()->m_dt = 0.001;
        m_pbdSystem->getConfig()->m_iterations = 5;
        m_pbdSystem->getConfig()->m_linearDampingCoeff = 0.025;
    }

    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        ASSERT_NE(m_graspedObj, nullptr) << "Missing object to be grasped, m_graspedObj";
        auto graspedMethod = m_graspedObj->getComponent<PbdMethod>();
        graspedMethod->getPbdSystem()->getConfig()->m_doPartitioning = false;
        m_scene->addSceneObject(m_graspedObj);

        ASSERT_NE(m_grasperObj, nullptr) << "Missing object to do grasping, m_grasperObj";
        m_scene->addSceneObject(m_grasperObj);

        ASSERT_NE(m_beginGraspFunc, nullptr) << "Missing begin grasp function for test";
        ASSERT_NE(m_moveFunc, nullptr) << "Missing grasp move function for test";

        // Script the movement of the grasper
        auto movementScript = m_grasperObj->addComponent<LambdaBehaviour>();
        movementScript->setUpdate([&](const double& dt)
            {
                // If not started the grasp sequence
                if (m_graspState == 0)
                {
                    m_beginGraspFunc();
                    m_graspState = 1;                     // Start grasp
                }
                // If grasped move
                else if (m_graspState == 1)
                {
                    m_moveFunc(dt);

                    // If over 2s stop grasp
                    m_graspTime += dt;
                    if (m_graspTime > m_graspDuration)
                    {
                        m_graspState = 2; // End grasp
                        m_pbdGrasping->endGrasp();
                                          // Optional end grasp condition
                        if (m_endGraspFunc != nullptr)
                        {
                            m_endGraspFunc();
                        }
                    }
                }
                        });

        // Supports both grasping via a non simulated object and pbd object
        auto grasperMethod = m_grasperObj->getComponentUnsafe<PbdMethod>();
        m_pbdGrasping = std::make_shared<PbdObjectGrasping>(graspedMethod, grasperMethod);
        m_pbdGrasping->setStiffness(m_graspStiffness);
        m_pbdGrasping->setCompliance(m_graspCompliance);
        // Optional support for grasping via a mapped geometry
        if (m_geomToGrasp != nullptr)
        {
            m_pbdGrasping->setGeometryToPick(m_geomToGrasp, m_geomToGraspMap);
        }
        m_scene->addInteraction(m_pbdGrasping);

        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [ = ](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                graspedMethod->getPbdSystem()->getConfig()->m_dt = m_sceneManager->getDt();
            });

        // Assert the vertices stay within bounds and below min displacement
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [ = ](Event*)
            {
                // Initialize
                if (m_prevGraspedVertices.size() == 0)
                {
                    m_prevGraspedVertices = *graspedMethod->getPbdBody()->vertices;
                }

                std::shared_ptr<VecDataArray<double, 3>> currGraspedVerticesPtr = graspedMethod->getPbdBody()->vertices;
                const VecDataArray<double, 3>& graspedVertices = *currGraspedVerticesPtr;
                // Assert to avoid hitting numerous times
                ASSERT_TRUE(assertBounds(graspedVertices, m_assertionBoundsMin, m_assertionBoundsMax));
                ASSERT_TRUE(assertMinDisplacement(m_prevGraspedVertices, graspedVertices, 0.1));
                m_prevGraspedVertices = graspedVertices;

                if (grasperMethod)
                {
                    if (m_prevGrasperVertices.size() == 0)
                    {
                        m_prevGrasperVertices = *grasperMethod->getPbdBody()->vertices;
                    }

                    std::shared_ptr<VecDataArray<double, 3>> currGrasperVerticesPtr = grasperMethod->getPbdBody()->vertices;
                    const VecDataArray<double, 3>& grasperVertices = *currGrasperVerticesPtr;
                    // Assert to avoid hitting numerous times
                    ASSERT_TRUE(assertBounds(grasperVertices, m_assertionBoundsMin, m_assertionBoundsMax));
                    ASSERT_TRUE(assertMinDisplacement(m_prevGrasperVertices, grasperVertices, 0.1));
                    m_prevGrasperVertices = grasperVertices;
                }
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<PbdSystem> m_pbdSystem  = nullptr;
    std::shared_ptr<Entity>    m_graspedObj = nullptr;
    std::shared_ptr<Entity>    m_grasperObj = nullptr;

    std::shared_ptr<PbdObjectGrasping> m_pbdGrasping = nullptr;
    double m_graspStiffness  = 0.5;
    double m_graspCompliance = 0.001;
    std::shared_ptr<Geometry>     m_geomToGrasp    = nullptr;
    std::shared_ptr<PointwiseMap> m_geomToGraspMap = nullptr;

    // For assertions
    VecDataArray<double, 3> m_prevGraspedVertices;
    VecDataArray<double, 3> m_prevGrasperVertices;

    Vec3d m_assertionBoundsMin = Vec3d(-1.0, -1.0, -1.0);
    Vec3d m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    std::function<void(const double)> m_moveFunc = nullptr;
    std::function<void()> m_beginGraspFunc       = nullptr;
    std::function<void()> m_endGraspFunc = nullptr;

    int    m_graspState    = 0;
    double m_graspTime     = 0.0;
    double m_graspDuration = 1.0;
};

///
/// \brief Test grasping with two graspers simulatenously grasping the same object
///
TEST_F(PbdObjectGraspingTest, PbdRigid_MultiGrasp)
{
    m_pbdSystem->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the sphere to grab
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.025);
    m_graspedObj = SceneUtils::makePbdEntity("grasped", sphere, m_pbdSystem);
    auto graspedMethod = m_graspedObj->getComponent<PbdMethod>();
    graspedMethod->setRigid(
        Vec3d(0.0, 0.0, 0.0), // Position
        1.0);                 // Mass

    // grasper0 positioned over the sphere
    m_grasperObj = makeGrasperObj("grasper0", Vec3d(0.0, 0.08, 0.0), m_pbdSystem);
    auto grasperMethod = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*grasperMethod->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d velocity = Vec3d(0.0, 0.1, 0.0);
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                 };

    createScene();

    // Setup a second grasper below the sphere now that the scene is setup
    auto grasperObj2    = makeGrasperObj("grasper1", Vec3d(0.0, -0.08, 0.0), m_pbdSystem);
    auto grasperMethod2 = grasperObj2->getComponent<PbdMethod>();
    auto capsule1       = std::dynamic_pointer_cast<Capsule>(grasperObj2->getComponent<Collider>()->getGeometry());
    auto controller1    = grasperObj2->getComponent<PbdObjectController>();
    auto client1 = std::make_shared<DummyClient>();
    client1->setPosition((*grasperMethod2->getPbdBody()->vertices)[0]);
    controller1->setDevice(client1);
    m_scene->addSceneObject(grasperObj2);

    auto capsuleGrasping1 = std::make_shared<PbdObjectGrasping>(graspedMethod, grasperMethod2);
    capsuleGrasping1->setCompliance(m_graspCompliance);
    capsuleGrasping1->setStiffness(m_graspStiffness);
    m_scene->addInteraction(capsuleGrasping1);

    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                           capsuleGrasping1->beginCellGrasp(capsule1);
                       };
    m_endGraspFunc = [&]()
                     {
                         capsuleGrasping1->endGrasp();
                     };

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test two-way grasping with SurfaceMesh
///
TEST_F(PbdObjectGraspingTest, PbdRigid_PointSet_CellGrasp)
{
        m_pbdSystem->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the rigid mesh to grab
    auto                         sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.025);
    std::shared_ptr<SurfaceMesh> surfMeshSphere = GeometryUtils::toUVSphereSurfaceMesh(sphere, 10, 10);
    m_graspedObj = SceneUtils::makePbdEntity("grasped", surfMeshSphere, m_pbdSystem);
    auto graspedMethod = m_graspedObj->getComponent<PbdMethod>();
    graspedMethod->setRigid(
        Vec3d(0.0, 0.0, 0.0), // Position
        1.0);                 // Mass

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.08, 0.0), m_pbdSystem);
    auto grasperMethod = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*grasperMethod->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d  velocity = Vec3d(0.0, 0.05, 0.0);
    const double angularVelocity = 5.0;
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                     const Quatd& orientation = client->getOrientation();
                     const Quatd  dq = Quatd(Eigen::AngleAxisd(angularVelocity * dt, Vec3d(0.0, 0.0, 1.0)));
                     client->setOrientation((orientation * dq).normalized());
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test two-way grasping with SurfaceMesh
///
TEST_F(PbdObjectGraspingTest, PbdRigid_Sphere_CellGrasp)
{
    m_pbdSystem->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the sphere to grab
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.025);
    m_graspedObj = SceneUtils::makePbdEntity("grasped", sphere, m_pbdSystem);
    m_graspedObj->getComponent<PbdMethod>()->setRigid(
                    Vec3d(0.0, 0.0, 0.0), // Position
                    1.0);                 // Mass

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.08, 0.0), m_pbdSystem);
    auto pbdGrasperObj = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*pbdGrasperObj->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d  velocity = Vec3d(0.0, 0.05, 0.0);
    const double angularVelocity = 5.0;
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                     const Quatd& orientation = client->getOrientation();
                     const Quatd  dq = Quatd(Eigen::AngleAxisd(angularVelocity * dt, Vec3d(0.0, 0.0, 1.0)));
                     client->setOrientation((orientation * dq).normalized());
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test two-way grasping with SurfaceMesh
///
TEST_F(PbdObjectGraspingTest, PbdThinTissue_PbdTwoWay_CellGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTriTissueObj("grasped",
            m_pbdSystem,
            Vec2d(0.1, 0.1), Vec2i(4, 4), Vec3d::Zero(),
            Quatd::Identity());

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0), m_pbdSystem);
    auto pbdGrasperObj = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*pbdGrasperObj->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test two-way grasping with TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_PbdTwoWay_CellGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTetTissueObj("grasped",
            m_pbdSystem,
            Vec3d(0.1, 0.05, 0.1), Vec3i(5, 2, 5), Vec3d::Zero(),
            Quatd::Identity(), true);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0), m_pbdSystem);
    auto pbdGrasperObj = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*pbdGrasperObj->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test two-way grasping with LineMesh
///
TEST_F(PbdObjectGraspingTest, PbdThread_PbdTwoWay_CellGrasp)
{
    // Setup the thread
    m_graspedObj = makeLineThreadObj("grasped",
            m_pbdSystem,
            0.2, 4, Vec3d(-0.1, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0), m_pbdSystem);
    auto pbdGrasperObj = m_grasperObj->getComponent<PbdMethod>();
    auto capsule       = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };

    auto client = std::make_shared<DummyClient>();
    client->setPosition((*pbdGrasperObj->getPbdBody()->vertices)[0]);
    auto controller = m_grasperObj->getComponent<PbdObjectController>();
    controller->setDevice(client);

    const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
    m_moveFunc = [&](const double dt)
                 {
                     client->setPosition(client->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test vertex grasping of LineMesh cells of a thread
///
TEST_F(PbdObjectGraspingTest, PbdThread_CollidingObject_CellGrasp)
{
    // Setup the thread
        m_graspedObj = makeLineThreadObj("grasped",
                m_pbdSystem,
                0.2, 4, Vec3d(-0.1, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));

    // Grasp with a capsule
        m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
        auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

        m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
        m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

        m_graspDuration  = 1.0;
        m_beginGraspFunc = [&]()
                           {
                               m_pbdGrasping->beginCellGrasp(capsule);
                           };
        m_moveFunc = [&](const double dt)
                     {
                         const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                         capsule->setPosition(capsule->getPosition() + velocity * dt);
                     };

        createScene();

    // Run for 2s at 0.01 fixed timestep
        runFor(2.0, 0.01);
}

///
/// \brief Test vertex grasping of LineMesh vertices of a thread
///
TEST_F(PbdObjectGraspingTest, PbdThread_CollidingObject_VertexGrasp)
{
    // Setup the thread
        m_graspedObj = makeLineThreadObj("grasped",
                m_pbdSystem,
                0.2, 5, Vec3d(-0.1, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));

    // Grasp with a capsule
        m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
        auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

        m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
        m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

        m_graspDuration  = 1.0;
        m_beginGraspFunc = [&]()
                           {
                               m_pbdGrasping->beginVertexGrasp(capsule);
                           };
        m_moveFunc = [&](const double dt)
                     {
                         const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                         capsule->setPosition(capsule->getPosition() + velocity * dt);
                     };

        createScene();

    // Run for 2s at 0.01 fixed timestep
        runFor(2.0, 0.01);
}

///
/// \brief Test ray grasping of SurfaceMesh vertices of a TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_Mapped_CollidingObject_RayGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTetTissueObj("grasped",
            m_pbdSystem,
            Vec3d(0.1, 0.05, 0.1), Vec3i(4, 2, 4), Vec3d::Zero(),
            Quatd::Identity(), false);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_geomToGrasp    = m_graspedObj->getComponent<Collider>()->getGeometry();
    m_geomToGraspMap = std::dynamic_pointer_cast<PointwiseMap>(m_graspedObj->getComponent<PbdMethod>()->getPhysicsToCollidingMap());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginRayPointGrasp(capsule, capsule->getCenter(), Vec3d(0.0, -1.0, 0.0));
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test ray grasping of SurfaceMesh vertices of a thin tissue
///
TEST_F(PbdObjectGraspingTest, PbdThinTissue_CollidingObject_RayGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTriTissueObj("grasped",
            m_pbdSystem,
            Vec2d(0.1, 0.1), Vec2i(4, 4), Vec3d::Zero(),
            Quatd::Identity());

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginRayPointGrasp(capsule, capsule->getCenter(), Vec3d(0.0, -1.0, 0.0));
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test cell grasping of SurfaceMesh vertices of a thin tissue
///
TEST_F(PbdObjectGraspingTest, PbdThinTissue_CollidingObject_CellGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTriTissueObj("grasped",
            m_pbdSystem,
            Vec2d(0.1, 0.1), Vec2i(5, 5), Vec3d::Zero(),
            Quatd::Identity());

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test vertex grasping of SurfaceMesh vertices of a thin tissue
///
TEST_F(PbdObjectGraspingTest, PbdThinTissue_CollidingObject_VertexGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTriTissueObj("grasped",
            m_pbdSystem,
            Vec2d(0.1, 0.1), Vec2i(5, 5), Vec3d::Zero(),
            Quatd::Identity());

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginVertexGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test vertex grasping by TetrahedralMesh vertices of a TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_CollidingObject_VertexGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTetTissueObj("grasped",
            m_pbdSystem,
            Vec3d(0.1, 0.05, 0.1), Vec3i(5, 2, 5), Vec3d::Zero(),
            Quatd::Identity(), true);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginVertexGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test cell grasping by TetrahedralMesh vertices of a TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_CollidingObject_CellGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTetTissueObj("grasped",
            m_pbdSystem,
            Vec3d(0.1, 0.05, 0.1), Vec3i(5, 2, 5), Vec3d::Zero(),
            Quatd::Identity(), true);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test vertex grasping of SurfaceMesh vertices of a TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_Mapped_CollidingObject_VertexGrasp)
{
    // Setup the tissue
    m_graspedObj = makeTetTissueObj("grasped",
            m_pbdSystem,
            Vec3d(0.1, 0.05, 0.1), Vec3i(5, 2, 5), Vec3d::Zero(),
            Quatd::Identity(), false);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_geomToGrasp    = m_graspedObj->getComponent<Collider>()->getGeometry();
    m_geomToGraspMap = std::dynamic_pointer_cast<PointwiseMap>(m_graspedObj->getComponent<PbdMethod>()->getPhysicsToCollidingMap());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginVertexGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}

///
/// \brief Test cell grasping of SurfaceMesh vertices of a TetrahedralMesh
///
TEST_F(PbdObjectGraspingTest, PbdTissue_Mapped_CollidingObject_CellGrasp)
{
    // Setup the tissue
    m_graspedObj =
        makeTetTissueObj("grasped",
        m_pbdSystem,
        Vec3d(0.1, 0.05, 0.1), Vec3i(5, 2, 5), Vec3d::Zero(),
        Quatd::Identity(), false);

    // Grasp with a capsule
    m_grasperObj = makeGrasperObj("grasper", Vec3d(0.0, 0.05, 0.0));
    auto capsule = std::dynamic_pointer_cast<Capsule>(m_grasperObj->getComponent<Collider>()->getGeometry());

    m_geomToGrasp    = m_graspedObj->getComponent<Collider>()->getGeometry();
    m_geomToGraspMap = std::dynamic_pointer_cast<PointwiseMap>(m_graspedObj->getComponent<PbdMethod>()->getPhysicsToCollidingMap());

    m_assertionBoundsMin = Vec3d(-1.0, -0.5, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    m_graspDuration  = 1.0;
    m_beginGraspFunc = [&]()
                       {
                           m_pbdGrasping->beginCellGrasp(capsule);
                       };
    m_moveFunc = [&](const double dt)
                 {
                     const Vec3d velocity = Vec3d(0.0, 0.05, 0.0);
                     capsule->setPosition(capsule->getPosition() + velocity * dt);
                 };

    createScene();

    // Run for 2s at 0.01 fixed timestep
    runFor(2.0, 0.01);
}