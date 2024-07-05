/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
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
#include "imstkPointwiseMap.h"
#include "imstkCapsule.h"
#include "imstkPbdObjectCollision.h"

#include "imstkMeshIO.h"

#include <numeric>

using namespace imstk;

static std::shared_ptr<PbdObject>
build(const std::string&               name,
      std::shared_ptr<PbdModel>        model,
      std::shared_ptr<TetrahedralMesh> tetMesh)
{
    std::shared_ptr<SurfaceMesh> surfMesh  = tetMesh->extractSurfaceMesh();
    auto                         tissueObj = std::make_shared<PbdObject>(name);
    // Setup the Object
    tissueObj->setPhysicsGeometry(tetMesh);
    tissueObj->setVisualGeometry(tetMesh);
    tissueObj->setCollidingGeometry(surfMesh);

    auto map = std::make_shared<PointwiseMap>();
    map->setParentGeometry(tetMesh);
    map->setChildGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(map);

    {
        // Setup the material
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        material->setColor(Color(1.0, 0.0, 0.0));
        //material->setEdgeColor(Color(0, 0, 1.0));
        tissueObj->getVisualModel(0)->setRenderMaterial(material);
    }

    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.01;

    model->getConfig()->m_femParams->m_YoungModulus = 1000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.45;         // 0.48 for tissue
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK,
                tissueObj->getPbdBody()->bodyHandle);
    tissueObj->getPbdBody()->bodyGravity = false;

    // Visualize Collision Mesh
    auto collisionVisuals = std::make_shared<VisualModel>();
    collisionVisuals->setGeometry(surfMesh);

    {
        // Setup the material
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(true);
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setShadingModel(RenderMaterial::ShadingModel::PBR);
        collisionVisuals->setRenderMaterial(material);
    }

    tissueObj->addVisualModel(collisionVisuals);

    return tissueObj;
}

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param size physical dimension of tissue
/// \param dim dimensions of tetrahedral grid used for tissue
/// \param center center of tissue block
/// \param useTetCollisionGeometry Whether to use a SurfaceMesh collision geometry+map
///
static std::shared_ptr<PbdObject>
makeCubeTetTissueObj(const std::string& name,
                     std::shared_ptr<PbdModel> model,
                     const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                     const Quatd& orientation)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    for (int i = 0; i < tetMesh->getNumTetrahedra(); i++)
    {
        std::swap((*tetMesh->getCells())[i][2], (*tetMesh->getCells())[i][3]);
    }

    auto tissueObj =  build(name, model, tetMesh);
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

static std::shared_ptr<PbdObject>
loadTetTissueObject(const std::string& name, const std::string& filename,
                    std::shared_ptr<PbdModel> model,
                    double scale)
{
    auto geometry = MeshIO::read<TetrahedralMesh>(filename);
    EXPECT_TRUE(geometry != nullptr);

    geometry->scale(scale, Geometry::TransformType::ApplyToData);
    geometry->updatePostTransformData();

    auto tissueObj =  build(name, model, geometry);

    // Basically fix a random point
    tissueObj->getPbdBody()->fixedNodeIds.push_back(0);

    return tissueObj;
}

static std::shared_ptr<PbdObject>
makeCollisionObject(
    const std::string&        name,
    std::shared_ptr<PbdModel> model,
    Vec3d                     center)
{
    auto rigidPbdObj = std::make_shared<PbdObject>(name);

    // Setup line geometry
    //auto rigidGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.5);
    auto                         rigidGeom = std::make_shared<Capsule>(Vec3d::Zero(), 0.005, 0.005);
    std::shared_ptr<SurfaceMesh> surfMesh  = GeometryUtils::toSurfaceMesh(rigidGeom);
    rigidPbdObj->setVisualGeometry(rigidGeom);
    rigidPbdObj->setCollidingGeometry(rigidGeom);
    rigidPbdObj->setPhysicsGeometry(rigidGeom);

    // Setup material
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Flat);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    rigidPbdObj->setDynamicalModel(model);

    // Setup body
    const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 1.0).normalized());
    rigidPbdObj->getPbdBody()->setRigid(center,
                0.005, orientation, Mat3d::Identity() * 0.01);

    return rigidPbdObj;
}

class PbdObjectTetMeshCellRemovalTest : public VisualTest
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

        m_cellRemoval = std::make_shared<PbdObjectCellRemoval>(m_obj, PbdObjectCellRemoval::OtherMeshUpdateType::Collision);
        m_scene->addInteraction(m_cellRemoval);

        auto collider = makeCollisionObject("Collision", m_pbdModel, Vec3d(0, .05, 0));
        m_scene->addSceneObject(collider);

        auto collision = std::make_shared<PbdObjectCollision>(m_obj, collider);
        collision->setRigidBodyCompliance(0.000001);
        m_scene->addSceneObject(collision);

        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_obj->getPbdModel()->getConfig()->m_dt = m_sceneManager->getDt();
                        });

//         // Assert the vertices stay within bounds
//         connect<Event>(m_sceneManager, &SceneManager::postUpdate,
//             [&](Event*)
//             {
//                 const VecDataArray<double, 3>& vertices = *m_currVerticesPtr;
//                 // Assert to avoid hitting numerous times
//                 ASSERT_TRUE(assertBounds(vertices, m_assertionBoundsMin, m_assertionBoundsMax));
//                 m_prevVertices = vertices;
//             });

        m_time   = 0.0;
        m_cellId = 0;
        std::random_device rd;
        auto               x = rd();
        std::mt19937       g(x);
        std::cout << "Seed: " << x;

        std::vector<int> cells(m_mesh->getNumCells(), 0);
        std::iota(cells.begin(), cells.end(), 0);
        std::shuffle(cells.begin(), cells.end(), g);

        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&, cells](Event*)
            {
                m_time += m_sceneManager->getDt();

                if (m_time > 0.1 && m_cellId < m_mesh->getNumCells())
                {
                    std::cout << "Before: " << m_pbdModel->getConstraints()->getConstraints().size() << std::endl;
                    m_cellRemoval->removeCellOnApply(cells[m_cellId]);
                    m_cellRemoval->apply();
                    m_cellId += 1;
                    std::cout << "After: " << m_pbdModel->getConstraints()->getConstraints().size() << std::endl;
                    m_time = 0.0;
                }
            });

        // Light
        {
            auto light = std::make_shared<DirectionalLight>();
            light->setFocalPoint(Vec3d(5.0, -8.0, 5.0));
            light->setIntensity(2.0);
            m_scene->addLight("Light 1", light);
        }
        {
            auto light = std::make_shared<DirectionalLight>();
            light->setFocalPoint(Vec3d(-5.0, 8.0, -5.0));
            light->setIntensity(2.0);
            m_scene->addLight("Light 2", light);
        }
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
TEST_F(PbdObjectTetMeshCellRemovalTest, TetMeshTest)
{
    // Setup the tissue without mapping
    m_obj = makeCubeTetTissueObj("TetTissue", m_pbdModel,
            Vec3d(0.1, 0.1, 0.1), Vec3i(4, 4, 4), Vec3d::Zero(),
            Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))));

    //m_obj = loadTetTissueObject("TetTissue", "C:/Work/projects/rae/unity/models/ct_outer_layer.vtk", m_pbdModel, 0.5);

    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(m_obj->getPhysicsGeometry());
    createScene();

    // Run for 3s at 0.01 fixed timestep
    runFor(50, 0.01);
}
