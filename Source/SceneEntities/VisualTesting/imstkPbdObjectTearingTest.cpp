/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkTearable.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneUtils.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTestingUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

// Setup the material
std::shared_ptr<RenderMaterial>
makeMaterial()
{
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);
    return material;
}

///
/// \brief Create PBD model to be used by all objects
///
static std::shared_ptr<PbdSystem>
makePbdSystem()
{
    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 5;
    pbdParams->m_linearDampingCoeff = 0.001;

    // Setup the Model
    auto pbdSystem = std::make_shared<PbdSystem>();

    return pbdSystem;
}

///
/// \brief Creates surface mesh tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<Entity>
makeSurfaceCubeObj(const std::string& name,
                   const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                   const Quatd& orientation,
                   const std::shared_ptr<PbdSystem> pbdSystem)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    auto surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, surfMesh, pbdSystem);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(makeMaterial());
    auto method = tissueObj->getComponent<PbdMethod>();
    method->getPbdBody()->uniformMassValue = 0.01;

    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
        method->getPbdBody()->bodyHandle);
    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 500.0,
        method->getPbdBody()->bodyHandle);

    return tissueObj;
}

///
/// \brief Creates tetrahedral mesh tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<Entity>
makeVolumeCubeObj(const std::string& name,
                  const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                  const Quatd& orientation,
                  const std::shared_ptr<PbdSystem> pbdSystem)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    auto surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Object
    auto tissueObj = SceneUtils::makePbdEntity(name, tetMesh, surfMesh, tetMesh, pbdSystem);
    tissueObj->getComponent<VisualModel>()->setRenderMaterial(makeMaterial());
    auto pbdBody = tissueObj->getComponent<PbdMethod>()->getPbdBody();
    pbdBody->uniformMassValue = 0.01;

    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
        pbdBody->bodyHandle);
    pbdSystem->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 500.0,
        pbdBody->bodyHandle);

    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = tetMesh->getVertexPositions();
    for (int i = 0; i < tetMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            pbdBody->fixedNodeIds.push_back(i);
        }
    }

    return tissueObj;
}

class PbdObjectTearingTest : public VisualTest
{
public:
    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0.278448, 0.0904159, 3.43076);
        m_scene->getActiveCamera()->setFocalPoint(0.0703459, -0.539532, 0.148011);
        m_scene->getActiveCamera()->setViewUp(-0.0400007, 0.980577, -0.19201);

        ASSERT_NE(m_pbdObj, nullptr) << "Missing a pbdObj for PbdObjectTest";

        m_scene->addSceneObject(m_pbdObj);

        if (m_pbdObjA != nullptr) { m_scene->addSceneObject(m_pbdObjA); }
        if (m_pbdObjB != nullptr) { m_scene->addSceneObject(m_pbdObjB); }

        m_scene->addSceneObject(m_pbdObj);

        auto pbdMethod = m_pbdObj->getComponent<PbdMethod>();
        auto pointSet  = std::dynamic_pointer_cast<PointSet>(pbdMethod->getGeometry());
        m_currVerticesPtr = pointSet->getVertexPositions();
        m_prevVertices    = *m_currVerticesPtr;

        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_pbdSystem->getConfig()->m_dt =
                    m_sceneManager->getDt();
            });

        // Assert the vertices stay within bounds
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const VecDataArray<double, 3>& vertices = *m_currVerticesPtr;
                // Assert to avoid hitting numerous times
                ASSERT_TRUE(assertBounds(vertices, m_assertionBoundsMin, m_assertionBoundsMax));
                m_prevVertices = vertices;
            });

        // Verify that map exists after the first iteration
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [ = ](Event*)
            {
                if (m_scene->getSceneTime() >= 0.5)
                {
                    ASSERT_EQ(false, pbdMethod->getPbdBody()->cellConstraintMap.empty());
                }
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

    // Pbd model used for simulation
    std::shared_ptr<PbdSystem> m_pbdSystem = nullptr;

    // Pbd object for testing component
    std::shared_ptr<Entity> m_pbdObj = nullptr;

    // Pbd objects to be connected for testing tearing
    std::shared_ptr<Entity> m_pbdObjA = nullptr;
    std::shared_ptr<Entity> m_pbdObjB = nullptr;

    // For assertions
    std::shared_ptr<VecDataArray<double, 3>> m_currVerticesPtr;
    VecDataArray<double, 3> m_prevVertices;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;

    Vec3d m_assertionBoundsMin = Vec3d(-3.0, -3.0, -3.0);
    Vec3d m_assertionBoundsMax = Vec3d(3.0, 3.0, 3.0);
};

///
/// \brief Test that the constraint map is correct
///
TEST_F(PbdObjectTearingTest, TestAddingTearingViaCall)
{
    m_pbdSystem = makePbdSystem();

    // Setup the tissue
    m_pbdObj = makeVolumeCubeObj("Tissue",
        Vec3d(0.4, 0.4, 0.4), Vec3i(2, 2, 2), Vec3d(0.0, 0.0, 0.0),
        Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdSystem);

    // Initialize Object
    m_pbdObj->getComponent<PbdMethod>()->initialize();

    // Do test
    auto tearing = std::make_shared<Tearable>();
    m_pbdObj->addComponent(tearing);

    auto testComponent = m_pbdObj->getComponent<Tearable>();

    EXPECT_EQ(testComponent, tearing);

    m_assertionBoundsMin = Vec3d(-3.0, -3.0, -3.0);
    m_assertionBoundsMax = Vec3d(3.0, 3.0, 3.0);

    createScene();
    runFor(1.0);
}

///
/// \brief Test that connective tissue is generated
///
TEST_F(PbdObjectTearingTest, PbdConnectiveDropTest)
{
    m_pbdSystem = makePbdSystem();

    // Setup the tissue
    m_pbdObjA = makeVolumeCubeObj("TissueA",
        Vec3d(0.4, 0.4, 0.4), Vec3i(2, 2, 2), Vec3d(0.0, 0.0, 0.0),
        Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdSystem);
    m_pbdObjA->getComponent<PbdMethod>()->initialize();

    m_pbdObjB = makeSurfaceCubeObj("TissueB",
        Vec3d(0.4, 0.4, 0.4), Vec3i(2, 2, 2), Vec3d(1.0, 0.0, 0.0),
        Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdSystem);
    m_pbdObjB->getComponent<PbdMethod>()->initialize();

    // Setup the connective tissue
    m_pbdObj = makeConnectiveTissue(m_pbdObjA, m_pbdObjB, m_pbdSystem);
    m_pbdObj->getComponent<PbdMethod>()->initialize();

    auto tearing = std::make_shared<Tearable>();
    m_pbdObj->addComponent(tearing);
    tearing->setMaxStrain(0.001);

    m_assertionBoundsMin = Vec3d(-3.0, -3.0, -3.0);
    m_assertionBoundsMax = Vec3d(3.0, 3.0, 3.0);

    createScene();
    runFor(2.0);
}