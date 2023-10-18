/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkTearable.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTestingUtils.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

///
/// \brief Create PBD model to be used by all objects
///
static std::shared_ptr<PbdModel>
makePbdModel()
{
    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->m_doPartitioning = false;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 2;
    pbdParams->m_linearDampingCoeff = 0.03;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();

    return pbdModel;
}

///
/// \brief Creates tetrahedral mesh tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeVolumeCubeObj(const std::string& name,
                  const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                  const Quatd& orientation,
                  const std::shared_ptr<PbdModel> pbdModel)
{
    auto cubeObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    auto surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tetMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    cubeObj->addVisualModel(visualModel);
    cubeObj->setPhysicsGeometry(tetMesh);
    cubeObj->setCollidingGeometry(surfMesh);
    cubeObj->setDynamicalModel(pbdModel);

    cubeObj->getPbdBody()->uniformMassValue = .1 / tetMesh->getNumVertices();
    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = tetMesh->getVertexPositions();
    for (int i = 0; i < tetMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= center[1] - size[1] * 0.5)
        {
            cubeObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    int bodyId = cubeObj->getPbdBody()->bodyHandle;
    pbdModel->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean, 108000.0 / bodyId, 0.4, bodyId);
    pbdModel->getConfig()->setBodyDamping(bodyId, 0.01);

    return cubeObj;
}

// Verifies a bugfix where we weren't able to set up multiple bodies with different strain
// constraints
class PbdMultipleFemBodiesTest : public VisualTest
{
public:
    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0, .05, .5);
        m_scene->getActiveCamera()->setFocalPoint(0, 0, 0);
        m_scene->getActiveCamera()->setViewUp(0, 1, 0);

        if (m_pbdObj_0 != nullptr) { m_scene->addSceneObject(m_pbdObj_0); }
        if (m_pbdObj_1 != nullptr) { m_scene->addSceneObject(m_pbdObj_1); }
        if (m_pbdObj_2 != nullptr) { m_scene->addSceneObject(m_pbdObj_2); }

        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_pbdModel->getConfig()->m_dt =
                    m_sceneManager->getDt();
            });
    }

public:

    // Pbd model used for simulation
    std::shared_ptr<PbdModel> m_pbdModel = nullptr;

    // Pbd object for testing component
    std::shared_ptr<PbdObject> m_pbdObj_0 = nullptr;
    std::shared_ptr<PbdObject> m_pbdObj_1 = nullptr;
    std::shared_ptr<PbdObject> m_pbdObj_2 = nullptr;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;
};

///
/// \brief Test that the constraint map is correct
///
TEST_F(PbdMultipleFemBodiesTest, AnisotropicModel)
{
    m_pbdModel = makePbdModel();

    m_pbdObj_0 = makeVolumeCubeObj("Tissue",
        { .1, .1, .1 }, { 5, 5, 5 }, { -.2, 0, 0 },
        Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdModel);
    m_pbdObj_0->initialize();

        m_pbdObj_1 = makeVolumeCubeObj("Tissue",
        { .1, .1, .1 }, { 5, 5, 5 }, { 0, 0, 0 },
                Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdModel);
    m_pbdObj_1->initialize();

        m_pbdObj_2 = makeVolumeCubeObj("Tissue",
        { .1, .1, .1 }, { 5, 5, 5 }, { .2, 0, 0 },
                Quatd(Rotd(0.0, Vec3d(0.0, 0.0, 1.0))), m_pbdModel);
    m_pbdObj_2->initialize();

    createScene();
    runFor(5.0);
}