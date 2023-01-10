/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCellRemoval.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkTestingUtils.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param size physical dimension of tissue
/// \param dim dimensions of tetrahedral grid used for tissue
/// \param center center of tissue block
/// \param useTetCollisionGeometry Whether to use a SurfaceMesh collision geometry+map
///
static std::shared_ptr<PbdObject>
makeTetTissueObj(const std::string& name,
                 std::shared_ptr<PbdModel> model,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                 const Quatd& orientation)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    for (int i = 0; i < tetMesh->getNumTetrahedra(); i++)
    {
        std::swap((*tetMesh->getCells())[i][2], (*tetMesh->getCells())[i][3]);
    }

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));

    // Setup the Object
    tissueObj->setPhysicsGeometry(tetMesh);
    tissueObj->setVisualGeometry(tetMesh);

    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.01;

    model->getConfig()->m_secParams->m_YoungModulus = 1000.0;
    model->getConfig()->m_secParams->m_PoissonRatio = 0.45;     // 0.48 for tissue
    model->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK,
                tissueObj->getPbdBody()->bodyHandle);

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || z == 0 || x == dim[0] - 1 || z == dim[2] - 1)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

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
static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 std::shared_ptr<PbdModel> model,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center,
                 const Quatd& orientation)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh =
        GeometryUtils::toTriangleGrid(center, size, dim, orientation);

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
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.00001;

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
                tissueObj->getPbdBody()->bodyHandle);
    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6,
                tissueObj->getPbdBody()->bodyHandle);

    // Fix the borders
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++)
        {
            if (x == 0 || y == 0 || x == dim[0] - 1 || y == dim[1] - 1)
            {
                tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * y);
            }
        }
    }

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
static std::shared_ptr<PbdObject>
makeLineThreadObj(const std::string& name,
                  std::shared_ptr<PbdModel> model,
                  const double length, const int dim, const Vec3d start,
                  const Vec3d& dir)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> lineMesh =
        GeometryUtils::toLineGrid(start, dir, length, dim);

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
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.00001;

    model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1,
                tissueObj->getPbdBody()->bodyHandle);
    tissueObj->getPbdBody()->fixedNodeIds = { 0, lineMesh->getNumVertices() - 1 };

    return tissueObj;
}

class PbdObjectCellRemovalTest : public VisualTest
{
public:
    void SetUp() override
    {
        VisualTest::SetUp();
        m_pbdModel = std::make_shared<PbdModel>();
        m_pbdModel->getConfig()->m_doPartitioning = false;
        m_pbdModel->getConfig()->m_dt = 0.001;
        m_pbdModel->getConfig()->m_iterations = 5;
        m_pbdModel->getConfig()->m_linearDampingCoeff = 0.025;
    }

    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        ASSERT_NE(m_obj, nullptr) << "Missing object to remove cells from";
        auto pointSet = std::dynamic_pointer_cast<PointSet>(m_obj->getPhysicsGeometry());
        m_currVerticesPtr = pointSet->getVertexPositions();
        m_prevVertices    = *m_currVerticesPtr;
        m_scene->addSceneObject(m_obj);

        m_cellRemoval = std::make_shared<PbdObjectCellRemoval>(m_obj);
        m_scene->addInteraction(m_cellRemoval);

        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_obj->getPbdModel()->getConfig()->m_dt = m_sceneManager->getDt();
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

        m_time   = 0.0;
        m_cellId = 0;

        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                m_time += m_sceneManager->getDt();

                if (m_time > 0.05 && m_cellId < m_mesh->getNumCells())
                {
                    m_cellRemoval->removeCellOnApply(m_cellId);
                    m_cellRemoval->apply();
                    m_cellId++;
                    m_time = 0.0;
                }
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<PbdModel>  m_pbdModel    = nullptr;
    std::shared_ptr<PbdObject> m_obj         = nullptr;
    std::shared_ptr<AbstractCellMesh> m_mesh = nullptr;

    std::shared_ptr<PbdObjectCellRemoval> m_cellRemoval = nullptr;

    // For assertions
    std::shared_ptr<VecDataArray<double, 3>> m_currVerticesPtr;
    VecDataArray<double, 3> m_prevVertices;

    Vec3d m_assertionBoundsMin = Vec3d(-10.0, -10.0, -10.0);
    Vec3d m_assertionBoundsMax = Vec3d(10.0, 10.0, 10.0);

    int    m_cellId = 0;
    double m_time   = 0.0;
};

///
/// \brief Test removal of cells for a tet mesh
///
TEST_F(PbdObjectCellRemovalTest, TetMeshTest)
{
    m_pbdModel->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the tissue without mapping
    m_obj = makeTetTissueObj("TetTissue", m_pbdModel,
            Vec3d(0.1, 0.1, 0.1), Vec3i(4, 4, 4), Vec3d::Zero(),
            Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))));

    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_obj->getPhysicsGeometry());
    createScene();

    // Run for 3s at 0.01 fixed timestep
    runFor(3.0, 0.01);
}

///
/// \brief Test removal of cells for a surface mesh
///
TEST_F(PbdObjectCellRemovalTest, SurfMeshTest)
{
    m_pbdModel->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the tissue without mapping
    m_obj = makeTriTissueObj("SurfTissue", m_pbdModel,
        Vec2d(0.1, 0.1), Vec2i(8, 8), Vec3d::Zero(),
        Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))));

    m_obj->initialize();

    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_obj->getPhysicsGeometry());

    createScene();

    // Run for 3s at 0.01 fixed timestep
    runFor(3.0, 0.01);
}

///
/// \brief Test removal of cells for a line mesh
///
TEST_F(PbdObjectCellRemovalTest, LineMeshTest)
{
    m_pbdModel->getConfig()->m_gravity = Vec3d::Zero();

    // Setup the tissue without mapping
    m_obj = makeLineThreadObj("String", m_pbdModel,
        0.3, 100, Vec3d(-0.15, 0.0, 0.0), Vec3d(1.0, 0.0, 0.0));

    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_obj->getPhysicsGeometry());
    createScene();

    // Run for 3s at 0.01 fixed timestep
    runFor(3.0, 0.01);
}